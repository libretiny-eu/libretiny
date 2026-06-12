/* FAL flash port for silabs-efm32gg11 (emlib MSC-backed).
 *
 * cores/common/base/config/fal_cfg.h declares `extern const struct fal_flash_dev
 * flash0` and lt_main() calls fal_init() unconditionally. These ops back the
 * running app's read/write/erase of internal flash (memory-mapped @ 0x0, 4 KiB
 * pages, 2 MiB on EFM32GG11B820) via Silicon Labs emlib MSC, which OTA bank
 * staging and metadata writes build on.
 *
 * em_msc.h resolves via EMLIB_DIR/inc, already on the family CPPPATH in
 * builder/family/silabs-efm32gg11.py; em_msc.c is already compiled into the
 * silabs-emlib-core library, so MSC_* symbols link without a build change.
 */

#include "em_device.h"
#include "em_msc.h"
#include "lt_family.h"

#include <fal.h>
#include <libretiny.h>
#include <string.h>

#define FLASH_ERASE_MIN_SIZE (4 * 1024)
#ifndef FLASH_LENGTH
#define FLASH_LENGTH (2 * 1024 * 1024) // EFM32GG11B820 internal flash
#endif

static int init(void) {
	MSC_Init();
	return 0;
}

static int read(long offset, uint8_t *buf, size_t size) {
	// Internal flash is memory-mapped from 0x0.
	memcpy(buf, (const void *)(uintptr_t)offset, size);
	return (int)size;
}

static int write(long offset, const uint8_t *buf, size_t size) {
	// MSC requires a 4-byte aligned address + length. Callers may pass a
	// sub-word length, but OTA writes are word-multiples; round the length up
	// to 4 and pad the tail with 0xFF.
	if (offset & 0x3)
		return -1;
	uint32_t aligned = (size + 3u) & ~3u;
	uint8_t tmp[256];
	long pos		   = offset;
	const uint8_t *src = buf;
	uint32_t rem	   = aligned;
	while (rem) {
		uint32_t n = rem < sizeof(tmp) ? rem : sizeof(tmp);
		memset(tmp, 0xFF, n);
		uint32_t copy = (uint32_t)(src - buf) < size
							? ((size - (uint32_t)(src - buf)) < n ? (size - (uint32_t)(src - buf)) : n)
							: 0;
		if (copy)
			memcpy(tmp, src, copy);
		if (MSC_WriteWord((uint32_t *)(uintptr_t)pos, tmp, n) != mscReturnOk)
			return -1;
		pos += n;
		src += copy;
		rem -= n;
	}
	return (int)size;
}

static int erase(long offset, size_t size) {
	long start = offset & ~((long)FLASH_PAGE_SIZE - 1);
	long end   = offset + (long)size;
	for (long a = start; a < end; a += FLASH_PAGE_SIZE) {
		if (MSC_ErasePage((uint32_t *)(uintptr_t)a) != mscReturnOk)
			return -1;
	}
	return (int)(end - start);
}

const struct fal_flash_dev flash0 = {
	.name		= FAL_FLASH_DEV_NAME,
	.addr		= 0x0,
	.len		= FLASH_LENGTH,
	.blk_size	= FLASH_ERASE_MIN_SIZE,
	.ops		= {init, read, write, erase},
	.write_gran = 1,
};
