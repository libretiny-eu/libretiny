#include "image2_sdk_adapter.h"
#include <string.h>

#if LT_AMB1_EXPERIMENTAL_IMAGE2_SDK_ADAPTER
#include <FreeRTOS.h>
#include <ameba_soc.h>
#include <device_lock.h>
#include <flash_api.h>
#include <task.h>

/* Actual wrapped SDK signatures. Only the private capability path uses these. */
extern void __real_flash_erase_sector(flash_t *, uint32_t);
extern int __real_flash_burst_write(flash_t *, uint32_t, uint32_t, uint8_t *);

/* The family hook erases the whole round sector extent inside a single
 * begin(), which on this part takes about as long as the platform's 10 s
 * hardware watchdog. ESPHome feeds that watchdog in its own OTA receive loop
 * but cannot run during this call, so refresh once per erased sector. */
extern void lt_wdt_feed(void);

enum { SECTOR = 4096, PAGE = 256 };

static struct {
	flash_t flash;
	TaskHandle_t task;
	const void *owner;
	uint32_t read_start, read_size, scratch[IMAGE2_SDK_SCRATCH_MAX];
	size_t count;
	image2_sdk_range staging;
	image2_sdk_range deny[IMAGE2_SDK_DENY_MAX];
	size_t deny_count;
	image2_sdk_range source;
	uint32_t demote_address;
	uint32_t target_signature_address;
	int bound, used, held, busy, failed;
	int staged_write, demoted;
	uint8_t staged[PAGE], check[PAGE];
} adapter;

static int task_ok(void) {
	return __get_IPSR() == 0 && adapter.task && xTaskGetCurrentTaskHandle() == adapter.task;
}

static int contains(uint32_t start, uint32_t size, uint32_t address, size_t n) {
	return n && address >= start && address - start < size && n <= size - (address - start);
}

static int scratch_ok(uint32_t address, size_t n) {
	for (size_t i = 0; i < adapter.count; ++i)
		if (contains(adapter.scratch[i], SECTOR, address, n))
			return 1;
	return 0;
}

static int overlaps(uint32_t address, size_t n, image2_sdk_range r) {
	/* Both bounds are validated at bind, and n is bounded by the containing
	 * window check below, but use 64-bit arithmetic so a bad request can never
	 * wrap the half-open end and look disjoint. */
	uint64_t end = (uint64_t)address + (uint64_t)n;
	return (uint64_t)address < (uint64_t)r.end && (uint64_t)r.start < end;
}

static int staging_ok(uint32_t address, size_t n) {
	if (adapter.staging.start >= adapter.staging.end)
		return 0;
	if (!contains(adapter.staging.start, adapter.staging.end - adapter.staging.start, address, n))
		return 0;
	for (size_t i = 0; i < adapter.deny_count; ++i)
		if (overlaps(address, n, adapter.deny[i]))
			return 0;
	return 1;
}

/* The observed running slot is a hard denial for every generic mutation, even
 * when a scratch sector or the staging window overlaps it. */
static int source_ok(uint32_t address, size_t n) {
	if (adapter.source.start >= adapter.source.end)
		return 0;
	return overlaps(address, n, adapter.source);
}

/* Either write capability, never an implicit whole-flash or slot-wide grant.
 * The running-slot source is checked first, so it can never be widened by an
 * overlapping scratch sector. */
static int authorized(uint32_t address, size_t n) {
	if (source_ok(address, n))
		return 0;
	return scratch_ok(address, n) || staging_ok(address, n);
}

static int take(void *ctx, const void *owner) {
	if (ctx != &adapter || !owner || !task_ok() || adapter.used || adapter.held)
		return 0;
	if (device_mutex_try_lock(RT_DEV_LOCK_FLASH) != 1)
		return 0;
	adapter.used = adapter.held = 1;
	adapter.owner				= owner;
	return 1;
}

static void release(void *ctx, const void *owner) {
	if (ctx != &adapter || !task_ok() || !adapter.held || adapter.busy || owner != adapter.owner)
		return;
	adapter.held  = 0;
	adapter.owner = NULL;
	device_mutex_unlock(RT_DEV_LOCK_FLASH);
}

static int enter(void *ctx) {
	if (ctx != &adapter || !task_ok() || !adapter.held || adapter.busy || adapter.failed)
		return 0;
	adapter.busy = 1;
	return 1;
}

static int finish(int ok) {
	if (!ok)
		adapter.failed = 1;
	adapter.busy = 0;
	return ok ? 1 : 0;
}

static int raw_read(uint32_t address, void *data, size_t n) {
	return flash_stream_read(&adapter.flash, address, (uint32_t)n, data) == 1;
}

static int read_bytes(void *ctx, uint32_t address, void *data, size_t n) {
	if (!enter(ctx))
		return 0;
	if (!data || !contains(adapter.read_start, adapter.read_size, address, n))
		return finish(0);
	/* Bound each SDK call and do not depend on unaligned caller buffers. */
	while (n) {
		size_t part = n < PAGE ? n : PAGE;
		if (!raw_read(address, adapter.check, part))
			return finish(0);
		memcpy(data, adapter.check, part);
		data = (uint8_t *)data + part;
		address += (uint32_t)part;
		n -= part;
	}
	return finish(1);
}

/* Stack-only capability; neither real symbol is exposed through public ops.
 * Authorization is rechecked immediately before crossing the guard boundary. */
typedef struct {
	uint32_t address;
	size_t length;
	int erase;
} mutation_cap;

static int mutate(const mutation_cap *cap) {
	if (!task_ok() || !adapter.held || !adapter.owner || !adapter.busy || adapter.failed ||
		!authorized(cap->address, cap->length))
		return 0;
	if (cap->erase) {
		if (cap->address % SECTOR || cap->length != SECTOR)
			return 0;
		__real_flash_erase_sector(&adapter.flash, cap->address);
		return 1; /* Invocation only; caller MUST verify the entire sector. */
	}
	if (cap->length > PAGE || cap->length > PAGE - cap->address % PAGE)
		return 0;
	return __real_flash_burst_write(&adapter.flash, cap->address, (uint32_t)cap->length, adapter.staged) == 1;
}

static int erase_sector(void *ctx, uint32_t address) {
	mutation_cap cap = {address, SECTOR, 1};
	if (!enter(ctx))
		return 0;
	/* Bounds the unfed gap to one sector erase plus its readback. */
	lt_wdt_feed();
	if (!mutate(&cap))
		return finish(0);
	for (uint32_t offset = 0; offset < SECTOR; offset += PAGE) {
		if (!raw_read(address + offset, adapter.check, PAGE))
			return finish(0);
		for (size_t i = 0; i < PAGE; ++i)
			if (adapter.check[i] != 0xff)
				return finish(0);
	}
	return finish(1);
}

/* A single serial-NOR program burst occasionally fails to latch every bit on
 * this part. Observed on hardware, each confirmed by an off-device programmer
 * read of the chip: one run left the trailing word of a 128-byte burst blank,
 * another left two bits set in one byte of a programme chunk. Re-issuing the
 * same program is safe because the preflight below proves every bit we need
 * set is still set, so a retry can only clear bits that must be clear. The
 * whole fragment is re-read after every attempt and the result must be exact,
 * so a genuinely bad cell still fails closed. */
#define PROGRAM_ATTEMPTS 3

static int program(void *ctx, uint32_t address, const void *data, size_t n) {
	mutation_cap cap = {address, n, 0};
	int mutated = 0, ok = 0;
	if (!enter(ctx))
		return 0;
	if (!data || !authorized(address, n) || n > PAGE || n > PAGE - address % PAGE)
		return finish(0);
	memcpy(adapter.staged, data, n);
	for (unsigned attempt = 0; attempt < PROGRAM_ATTEMPTS && !ok; ++attempt) {
		/* NOR preflight: programming may only clear bits. Re-checked before
		 * every attempt because a partial program lowers the set bits. */
		if (!raw_read(address, adapter.check, n))
			return finish(0);
		for (size_t i = 0; i < n; ++i)
			if ((adapter.check[i] & adapter.staged[i]) != adapter.staged[i])
				return finish(0);
		if (memcmp(adapter.staged, adapter.check, n) == 0) {
			ok = 1; /* already exact, nothing left to programme */
			break;
		}
		if (!mutate(&cap))
			return finish(0);
		mutated = 1;
		if (!raw_read(address, adapter.check, n))
			return finish(0);
		ok = memcmp(adapter.staged, adapter.check, n) == 0;
	}
	if (!ok)
		return finish(0);
	if (mutated)
		adapter.staged_write = 1;
	return finish(1);
}

/* Dedicated activation demote. This is the ONLY mutation allowed inside the
 * running slot: exactly one ASCII '8' -> '0' at the bound signature byte, and
 * only after this session staged data, the FULL running source signature reads
 * new, AND the inactive-slot target has published that same signature. The
 * target read duplicates the backend's publication check on purpose: a direct
 * adapter demote must also fail closed until the target image is fully published.
 * It never consults authorized(), which hard-denies source. */
static int demote(void *ctx) {
	static const uint8_t new_signature[8] = {'8', '1', '9', '5', '8', '7', '1', '1'};
	static const uint8_t old_signature[8] = {'0', '1', '9', '5', '8', '7', '1', '1'};
	if (ctx != &adapter)
		return 0;
	if (!enter(ctx))
		return finish(0);
	if (!adapter.demote_address || !adapter.target_signature_address || !adapter.staged_write || adapter.demoted ||
		adapter.demote_address != adapter.source.start + 8 || adapter.source.start >= adapter.source.end ||
		!contains(adapter.source.start, adapter.source.end - adapter.source.start, adapter.demote_address, 1) ||
		adapter.target_signature_address != adapter.staging.start + 8 || adapter.staging.start >= adapter.staging.end ||
		!contains(
			adapter.staging.start,
			adapter.staging.end - adapter.staging.start,
			adapter.target_signature_address,
			8
		))
		return finish(0);
	if (!raw_read(adapter.demote_address, adapter.check, 8) || memcmp(adapter.check, new_signature, 8))
		return finish(0);
	/* Target evidence: an erased, old or partial inactive-slot signature means
	 * the target image is not published yet, so the demote must not fire. */
	if (!raw_read(adapter.target_signature_address, adapter.check, 8) || memcmp(adapter.check, new_signature, 8))
		return finish(0);
	memcpy(adapter.staged, old_signature, 1);
	if (__real_flash_burst_write(&adapter.flash, adapter.demote_address, 1, adapter.staged) != 1)
		return finish(0);
	adapter.demoted = 1;
	/* The full eight-byte signature must now read the demoted form. */
	if (!raw_read(adapter.demote_address, adapter.check, 8) || memcmp(adapter.check, old_signature, 8))
		return finish(0);
	return finish(1);
}

int image2_sdk_adapter_bind(const image2_sdk_qualification_policy *policy, image2_flash_sdkops *ops) {
	if (!ops)
		return 0;
	memset(ops, 0, sizeof(*ops));
	if (adapter.bound || !policy || __get_IPSR() != 0 || !xTaskGetCurrentTaskHandle() || !policy->read_size ||
		policy->read_start > UINT32_MAX - policy->read_size || policy->scratch_count > IMAGE2_SDK_SCRATCH_MAX ||
		(policy->scratch_count && !policy->scratch_sectors) || policy->deny_count > IMAGE2_SDK_DENY_MAX ||
		(policy->deny_count && !policy->deny_ranges) || policy->staging.start > policy->staging.end ||
		policy->source.start > policy->source.end)
		return 0;
	for (size_t i = 0; i < policy->scratch_count; ++i)
		if (policy->scratch_sectors[i] % SECTOR ||
			!contains(policy->read_start, policy->read_size, policy->scratch_sectors[i], SECTOR))
			return 0;
	if (policy->staging.start == policy->staging.end) {
		/* Only the canonical {0, 0} read-only sentinel; deny data without a
		 * window would silently do nothing and is therefore rejected. */
		if (policy->staging.start || policy->deny_count)
			return 0;
	} else {
		uint32_t size = policy->staging.end - policy->staging.start;
		if (policy->staging.start % SECTOR || policy->staging.end % SECTOR ||
			!contains(policy->read_start, policy->read_size, policy->staging.start, size))
			return 0;
		for (size_t i = 0; i < policy->deny_count; ++i) {
			image2_sdk_range d = policy->deny_ranges[i];
			if (d.start >= d.end || !contains(policy->read_start, policy->read_size, d.start, d.end - d.start))
				return 0;
			/* The window and every denial must be disjoint before any
			 * request can be approved; no runtime repair or clamping. */
			if (d.start < policy->staging.end && policy->staging.start < d.end)
				return 0;
		}
	}
	/* The observed running slot is a separate capability. It must be disjoint
	 * from the staging window and, when a demote byte is declared, exactly one
	 * byte past the start of the full running signature. The demote and the
	 * target-publication evidence are bound together so a demote can never be
	 * authorized without an explicitly named, in-window inactive-slot
	 * signature address. */
	if (policy->source.start == policy->source.end) {
		if (policy->source.start || policy->demote_address || policy->target_signature_address)
			return 0;
	} else {
		uint32_t source_size = policy->source.end - policy->source.start;
		if (!contains(policy->read_start, policy->read_size, policy->source.start, source_size))
			return 0;
		if (policy->staging.start < policy->staging.end && policy->staging.start < policy->source.end &&
			policy->source.start < policy->staging.end)
			return 0;
		if (policy->demote_address) {
			if (source_size < 9 || policy->demote_address != policy->source.start + 8 ||
				!contains(policy->read_start, policy->read_size, policy->demote_address, 1))
				return 0;
			/* The target signature region mirrors demote_address: the first
			 * byte past the inactive-slot start, with all eight bytes inside a
			 * valid staging window and the declared read extent. An erased or
			 * empty staging window cannot provide publication evidence. */
			if (policy->staging.start >= policy->staging.end || policy->staging.end - policy->staging.start < 16 ||
				policy->target_signature_address != policy->staging.start + 8 ||
				!contains(policy->read_start, policy->read_size, policy->target_signature_address, 8))
				return 0;
		} else if (policy->target_signature_address) {
			/* Evidence without a demote capability is a malformed policy, not
			 * an implicit grant. */
			return 0;
		}
	}
	adapter.read_start = policy->read_start;
	adapter.read_size  = policy->read_size;
	adapter.count	   = policy->scratch_count;
	for (size_t i = 0; i < adapter.count; ++i)
		adapter.scratch[i] = policy->scratch_sectors[i];
	adapter.staging	   = policy->staging;
	adapter.deny_count = policy->deny_count;
	for (size_t i = 0; i < adapter.deny_count; ++i)
		adapter.deny[i] = policy->deny_ranges[i];
	adapter.source					 = policy->source;
	adapter.demote_address			 = policy->demote_address;
	adapter.target_signature_address = policy->target_signature_address;
	adapter.task					 = xTaskGetCurrentTaskHandle();
	adapter.bound					 = 1;
	*ops = (image2_flash_sdkops){&adapter, take, release, read_bytes, program, erase_sector, demote};
	return 1;
}
#else
int image2_sdk_adapter_bind(const image2_sdk_qualification_policy *policy, image2_flash_sdkops *ops) {
	(void)policy;
	if (ops)
		memset(ops, 0, sizeof(*ops));
	return 0;
}
#endif
