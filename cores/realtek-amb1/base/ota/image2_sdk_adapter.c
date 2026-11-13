#include "image2_sdk_adapter.h"
#include <string.h>

#if LT_AMB1_EXPERIMENTAL_IMAGE2_SDK_ADAPTER
#include <ameba_soc.h>
#include <flash_api.h>
#include <device_lock.h>
#include <FreeRTOS.h>
#include <task.h>

/* Actual wrapped SDK signatures. Only the private capability path uses these. */
extern void __real_flash_erase_sector(flash_t *, uint32_t);
extern int __real_flash_burst_write(flash_t *, uint32_t, uint32_t, uint8_t *);

enum { SECTOR = 4096, PAGE = 256 };
static struct {
	flash_t flash;
	TaskHandle_t task;
	const void *owner;
	uint32_t read_start, read_size, scratch[IMAGE2_SDK_SCRATCH_MAX];
	size_t count;
	int bound, used, held, busy, failed;
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

static int take(void *ctx, const void *owner) {
	if (ctx != &adapter || !owner || !task_ok() || adapter.used || adapter.held)
		return 0;
	if (device_mutex_try_lock(RT_DEV_LOCK_FLASH) != 1)
		return 0;
	adapter.used = adapter.held = 1;
	adapter.owner = owner;
	return 1;
}

static void release(void *ctx, const void *owner) {
	if (ctx != &adapter || !task_ok() || !adapter.held || adapter.busy || owner != adapter.owner)
		return;
	adapter.held = 0;
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
typedef struct { uint32_t address; size_t length; int erase; } mutation_cap;
static int mutate(const mutation_cap *cap) {
	if (!task_ok() || !adapter.held || !adapter.owner || !adapter.busy || adapter.failed ||
		!scratch_ok(cap->address, cap->length))
		return 0;
	if (cap->erase) {
		if (cap->address % SECTOR || cap->length != SECTOR)
			return 0;
		__real_flash_erase_sector(&adapter.flash, cap->address);
		return 1; /* Invocation only; caller MUST verify the entire sector. */
	}
	if (cap->length > PAGE || cap->length > PAGE - cap->address % PAGE)
		return 0;
	return __real_flash_burst_write(&adapter.flash, cap->address,
		(uint32_t)cap->length, adapter.staged) == 1;
}

static int erase_sector(void *ctx, uint32_t address) {
	mutation_cap cap = { address, SECTOR, 1 };
	if (!enter(ctx))
		return 0;
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

static int program(void *ctx, uint32_t address, const void *data, size_t n) {
	mutation_cap cap = { address, n, 0 };
	if (!enter(ctx))
		return 0;
	if (!data || !scratch_ok(address, n) || n > PAGE || n > PAGE - address % PAGE)
		return finish(0);
	memcpy(adapter.staged, data, n);
	if (!raw_read(address, adapter.check, n))
		return finish(0);
	for (size_t i = 0; i < n; ++i)
		if ((adapter.check[i] & adapter.staged[i]) != adapter.staged[i])
			return finish(0);
	if (!mutate(&cap) || !raw_read(address, adapter.check, n))
		return finish(0);
	return finish(memcmp(adapter.staged, adapter.check, n) == 0);
}

int image2_sdk_adapter_bind(const image2_sdk_qualification_policy *policy, image2_flash_sdkops *ops) {
	if (!ops)
		return 0;
	memset(ops, 0, sizeof(*ops));
	if (adapter.bound || !policy || __get_IPSR() != 0 || !xTaskGetCurrentTaskHandle() ||
		!policy->read_size || policy->read_start > UINT32_MAX - policy->read_size ||
		policy->scratch_count > IMAGE2_SDK_SCRATCH_MAX ||
		(policy->scratch_count && !policy->scratch_sectors))
		return 0;
	for (size_t i = 0; i < policy->scratch_count; ++i)
		if (policy->scratch_sectors[i] % SECTOR ||
			!contains(policy->read_start, policy->read_size, policy->scratch_sectors[i], SECTOR))
			return 0;
	adapter.read_start = policy->read_start;
	adapter.read_size = policy->read_size;
	adapter.count = policy->scratch_count;
	for (size_t i = 0; i < adapter.count; ++i)
		adapter.scratch[i] = policy->scratch_sectors[i];
	adapter.task = xTaskGetCurrentTaskHandle();
	adapter.bound = 1;
	*ops = (image2_flash_sdkops){ &adapter, take, release, read_bytes, program, erase_sector };
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
