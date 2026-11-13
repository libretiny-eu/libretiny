/* Ameba1 first-bench policy: flash is readable, never writable. */
#include <libretiny.h>
#include <sdk_private.h>
#include <device_lock.h>

#include <fal.h>
#include <limits.h>

flash_t lt_flash_obj;

/* Boot-crash instrumentation, see fixups/boot_trace.c (temporary). */
extern void lt_amb1_trace(const char *tag);
extern void lt_amb1_heartbeat_init(void);

static int init(void) {
	/* Do not initialize storage by changing flash status/protection bits. */
	LT_W("AMB1 flash: read-only bench build; persistence and OTA disabled");
	lt_amb1_trace("lt_main -> fal_init -> flash0 init");
	/* Safe runtime hook: fal_init() runs inside lt_main AFTER the _AppStart BSS
	 * clears and AFTER __libc_init_array (the 002 capture shows this hook
	 * reached on hardware), so the FreeRTOS heap and its bookkeeping are valid
	 * here. candidate005 created the heartbeat timer from _AppStart, before the
	 * BSS clears, and hard-faulted in pvPortMalloc (heap_5.c:276). The timer
	 * itself only starts firing once the scheduler is up; the LED is
	 * initialised from its callback. */
	lt_amb1_heartbeat_init();
	return 0;
}

static int read(long offset, uint8_t *buf, size_t size) {
	if (offset < 0 || (unsigned long)offset > FLASH_LENGTH ||
		size > FLASH_LENGTH - (unsigned long)offset || size > INT_MAX || (!buf && size))
		return -1;
	if (!size)
		return 0;
	/* The SDK read does not lock; serialize with SDK users and the OTA adapter.
	 * Already-locked OTA callbacks must call the SDK directly, not this path. */
	device_mutex_lock(RT_DEV_LOCK_FLASH);
	int result = flash_stream_read(&lt_flash_obj, offset, size, buf);
	device_mutex_unlock(RT_DEV_LOCK_FLASH);
	/* RTL8195A returns 1 on success, not the number of bytes transferred. */
	return result == 1 ? (int)size : -1;
}

static int write(long offset, const uint8_t *buf, size_t size) {
	(void)buf;
	LT_E("AMB1 read-only flash: denied write at 0x%lx (%lu bytes)", offset, (unsigned long)size);
	return -1;
}

static int erase(long offset, size_t size) {
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
