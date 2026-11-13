#include "preferences_sdk.h"

#if LT_AMB1_EXPERIMENTAL_KVS
#include <ameba_soc.h>
#include <flash_api.h>
#include <device_lock.h>
#include <FreeRTOS.h>
#include <task.h>
#include <string.h>

extern int __real_flash_burst_write(flash_t *, uint32_t, uint32_t, uint8_t *);
extern void __real_flash_erase_sector(flash_t *, uint32_t);

enum { PAGE = 256 };
static struct {
	flash_t object;
	amb1_preferences_flash_t bridge;
	TaskHandle_t owner;
	int held;
	uint32_t staged[PAGE / sizeof(uint32_t)];
} sdk;

static int task_ready(void) {
	return __get_IPSR() == 0 && xTaskGetSchedulerState() == taskSCHEDULER_RUNNING &&
	       xTaskGetCurrentTaskHandle() != NULL;
}

static int held_range(uint32_t address, size_t size, int erase) {
	return task_ready() && sdk.held && sdk.owner == xTaskGetCurrentTaskHandle() &&
	       size <= AMB1_PREF_KVS_END - AMB1_PREF_KVS_START &&
	       amb1_preferences_range_allowed(&sdk.bridge.policy.review, address, (uint32_t)size, erase);
}

static int take(void *context) {
	if (context != &sdk || !task_ready() || sdk.held ||
	    device_mutex_try_lock(RT_DEV_LOCK_FLASH) != 1)
		return -1;
	sdk.owner = xTaskGetCurrentTaskHandle();
	sdk.held = 1;
	return 0;
}

static void release(void *context) {
	if (context != &sdk || !task_ready() || !sdk.held || sdk.owner != xTaskGetCurrentTaskHandle())
		return;
	sdk.held = 0;
	sdk.owner = NULL;
	device_mutex_unlock(RT_DEV_LOCK_FLASH);
}

static int read_locked(void *context, uint32_t address, void *data, size_t size) {
	if (context != &sdk || !data || !held_range(address, size, 0))
		return -1;
	/* SDK read is non-locking. Do not route through FAL here. */
	return flash_stream_read(&sdk.object, address, (uint32_t)size, data) == 1 ? 0 : -1;
}

/* Private capability: only this translation unit constructs one, under the
 * operation's shared-controller lock. Recheck exact extent at guard bypass. */
typedef struct { uint32_t address; size_t size; int erase; } mutation_cap;
static int mutate(const mutation_cap *cap) {
	if (!held_range(cap->address, cap->size, cap->erase))
		return -1;
	if (cap->erase) {
		if (cap->size != AMB1_PREF_SECTOR_SIZE)
			return -1;
		__real_flash_erase_sector(&sdk.object, cap->address);
		return 0; /* Void SDK: preferences_flash verifies EVERY erased byte. */
	}
	if (cap->size > PAGE - cap->address % PAGE)
		return -1;
	return __real_flash_burst_write(&sdk.object, cap->address, (uint32_t)cap->size,
		(uint8_t *)sdk.staged) == 1 ? 0 : -1;
}

static int write_locked(void *context, uint32_t address, const void *data, size_t size) {
	const uint8_t *bytes = data;
	if (context != &sdk || !data || !held_range(address, size, 0))
		return -1;
	while (size) {
		size_t part = PAGE - address % PAGE;
		if (part > size)
			part = size;
		mutation_cap cap = { address, part, 0 };
		memcpy(sdk.staged, bytes, part);
		if (mutate(&cap))
			return -1;
		address += (uint32_t)part;
		bytes += part;
		size -= part;
	}
	return 0;
}

static int erase_locked(void *context, uint32_t address, size_t size) {
	mutation_cap cap = { address, size, 1 };
	return context == &sdk ? mutate(&cap) : -1;
}

static int geometry_ok(const amb1_preferences_sdk_geometry_t *g) {
	return g->flash_size == AMB1_PREF_FLASH_SIZE && g->sector_size == AMB1_PREF_SECTOR_SIZE &&
	       g->page_size == PAGE && g->ota1_start == AMB1_PREF_SLOT_A &&
	       g->ota1_end == AMB1_PREF_SLOT_A_END && g->ota2_start == AMB1_PREF_SLOT_B &&
	       g->ota2_end == AMB1_PREF_SLOT_B_END && g->kvs_start == AMB1_PREF_KVS_START &&
	       g->kvs_end == AMB1_PREF_KVS_END && g->system_start == UINT32_C(0x2e3000) &&
	       g->system_end == AMB1_PREF_FLASH_SIZE && g->image_max == AMB1_PREF_IMAGE_MAX;
}

int amb1_preferences_sdk_bind(const amb1_preferences_sdk_policy_t *policy) {
	amb1_preferences_flash_t candidate = {0};
	const amb1_preferences_flash_ops_t ops = {
		&sdk, read_locked, write_locked, erase_locked, take, release
	};
	if (sdk.bridge.bound || !policy || !task_ready() || policy->runtime_qualification_passed != 1 ||
	    !geometry_ok(&policy->current_geometry) ||
	    amb1_preferences_flash_bind(&candidate, &ops, &policy->flash))
		return -1;
	/* Fail closed if external SDK mutex preparation has not completed. */
	if (take(&sdk))
		return -1;
	sdk.bridge = candidate;
	release(&sdk);
	return 0;
}

static long relative(long offset, size_t size) {
	if (offset < 0 || (unsigned long)offset < AMB1_PREF_KVS_START ||
	    (unsigned long)offset >= AMB1_PREF_KVS_END || !size ||
	    size > AMB1_PREF_KVS_END - (unsigned long)offset)
		return -1;
	return offset - AMB1_PREF_KVS_START;
}

int amb1_preferences_sdk_read(long offset, uint8_t *data, size_t size) {
	return amb1_preferences_flash_read(&sdk.bridge, relative(offset, size), data, size);
}
int amb1_preferences_sdk_write(long offset, const uint8_t *data, size_t size) {
	return amb1_preferences_flash_write(&sdk.bridge, relative(offset, size), data, size);
}
int amb1_preferences_sdk_erase(long offset, size_t size) {
	return amb1_preferences_flash_erase(&sdk.bridge, relative(offset, size), size);
}
int amb1_preferences_sdk_format_allowed(void) {
	return amb1_preferences_flash_format_allowed(&sdk.bridge);
}
#else
int amb1_preferences_sdk_bind(const amb1_preferences_sdk_policy_t *policy) {
	(void)policy;
	return -1;
}
int amb1_preferences_sdk_read(long offset, uint8_t *data, size_t size) {
	(void)offset; (void)data; (void)size;
	return -1;
}
int amb1_preferences_sdk_write(long offset, const uint8_t *data, size_t size) {
	(void)offset; (void)data; (void)size;
	return -1;
}
int amb1_preferences_sdk_erase(long offset, size_t size) {
	(void)offset; (void)size;
	return -1;
}
int amb1_preferences_sdk_format_allowed(void) { return 0; }
#endif
