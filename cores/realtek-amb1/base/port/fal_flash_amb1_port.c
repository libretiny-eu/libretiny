/* Ameba1 defaults to read-only; experimental KVS uses a separately bound bridge. */
#include <libretiny.h>
#include <sdk_private.h>
#include <device_lock.h>

#include <fal.h>
#include <limits.h>
#include "preferences_sdk.h"

#if defined(LT_AMB1_EXPERIMENTAL_IMAGE2_SDK_ADAPTER) && LT_AMB1_EXPERIMENTAL_IMAGE2_SDK_ADAPTER
#include "lt_ota_family_amb1_sdk.h"
#endif

flash_t lt_flash_obj;

/* Boot-crash instrumentation, see fixups/boot_trace.c (temporary). */
extern void lt_amb1_trace(const char *tag);
extern void lt_amb1_heartbeat_init(void);

/* A reviewed profile may bind the SDK bridge from a running task before FlashDB init.
 * The default neither binds storage nor approves qualification or formatting. */
__attribute__((weak)) void lt_amb1_preferences_configure(void) {
}

#if LT_AMB1_EXPERIMENTAL_KVS
static int wholly_kvs(long offset, size_t size) {
	return offset >= 0 && (unsigned long)offset >= AMB1_PREF_KVS_START &&
		(unsigned long)offset < AMB1_PREF_KVS_END && size &&
		size <= AMB1_PREF_KVS_END - (unsigned long)offset;
}
#endif

static int init(void) {
	/* Do not initialize storage by changing flash status/protection bits. */
#if LT_AMB1_EXPERIMENTAL_KVS
	LT_W("AMB1 flash: experimental KVS requires explicit SDK binding; OTA disabled");
#else
	LT_W("AMB1 flash: read-only bench build; persistence and OTA disabled");
#endif
	lt_amb1_trace("lt_main -> fal_init -> flash0 init");
	/* Safe runtime hook: fal_init() runs inside lt_main AFTER the _AppStart BSS
	 * clears and AFTER __libc_init_array (the 002 capture shows this hook
	 * reached on hardware), so the FreeRTOS heap and its bookkeeping are valid
	 * here. candidate005 created the heartbeat timer from _AppStart, before the
	 * BSS clears, and hard-faulted in pvPortMalloc (heap_5.c:276). The timer
	 * itself only starts firing once the scheduler is up; the LED is
	 * initialised from its callback. */
	lt_amb1_heartbeat_init();
	/* Prepare the shared flash mutex before any concurrent SDK flash user can
	 * exist. This runs after BSS clears and __libc_init_array, from the single
	 * lt_main task, so allocation is valid. Failure is logged only: the OTA SDK
	 * seam stays fail-closed and never retries with a private mutex. */
#if defined(LT_AMB1_EXPERIMENTAL_IMAGE2_SDK_ADAPTER) && LT_AMB1_EXPERIMENTAL_IMAGE2_SDK_ADAPTER
	if (!lt_ota_family_amb1_sdk_prepare())
		LT_W("AMB1 flash: OTA SDK mutex unavailable; OTA staging stays disabled");
#endif
	return 0;
}

static int read(long offset, uint8_t *buf, size_t size) {
	if (offset < 0 || (unsigned long)offset > FLASH_LENGTH ||
		size > FLASH_LENGTH - (unsigned long)offset || size > INT_MAX || (!buf && size))
		return -1;
	if (!size)
		return 0;
#if LT_AMB1_EXPERIMENTAL_KVS
	/* The bridge takes its own lock and reads directly through the SDK.
	 * Never fall back on failure (including an unbound bridge). */
	if (wholly_kvs(offset, size))
		return amb1_preferences_sdk_read(offset, buf, size);
#endif
	/* The SDK read does not lock; serialize with SDK users and the OTA adapter.
	 * Already-locked OTA callbacks must call the SDK directly, not this path. */
	device_mutex_lock(RT_DEV_LOCK_FLASH);
	int result = flash_stream_read(&lt_flash_obj, offset, size, buf);
	device_mutex_unlock(RT_DEV_LOCK_FLASH);
	/* RTL8195A returns 1 on success, not the number of bytes transferred. */
	return result == 1 ? (int)size : -1;
}

static int write(long offset, const uint8_t *buf, size_t size) {
#if LT_AMB1_EXPERIMENTAL_KVS
	if (buf && wholly_kvs(offset, size))
		return amb1_preferences_sdk_write(offset, buf, size);
#endif
	(void)buf;
	LT_E("AMB1 read-only flash: denied write at 0x%lx (%lu bytes)", offset, (unsigned long)size);
	return -1;
}

static int erase(long offset, size_t size) {
#if LT_AMB1_EXPERIMENTAL_KVS
	if (wholly_kvs(offset, size))
		return amb1_preferences_sdk_erase(offset, size);
#endif
	LT_E("AMB1 read-only flash: denied erase at 0x%lx (%lu bytes)", offset, (unsigned long)size);
	return -1;
}

const struct fal_flash_dev flash0 = {
	.name       = FAL_FLASH_DEV_NAME,
	.addr       = 0,
	.len        = FLASH_LENGTH,
	.blk_size   = 4096,
	.ops        = {init, read, write, erase},
	.write_gran = 1,
};
