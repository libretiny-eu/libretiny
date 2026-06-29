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
#include "lt_ota_meta.h"

#include <fal.h>
#include <libretiny.h>
#include <string.h>

#define FLASH_ERASE_MIN_SIZE (4 * 1024)
#ifndef FLASH_LENGTH
#define FLASH_LENGTH (2 * 1024 * 1024) // EFM32GG11B820 internal flash
#endif

// --- OTA write-extent tracking -------------------------------------------
// uf2ota and direct OTA staging both funnel image bytes through write() below
// (FAL partitions delegate to this device's ops). Track the highest written
// offset within each app bank so lt_ota_switch can learn a freshly-staged
// image's length without the image self-describing. erase() of a bank base
// clears it. ota_hi[b] == 0 means "nothing staged since the last erase".
static uint32_t ota_hi[LT_OTA_BANK_COUNT];

static int ota_bank_of(long off) {
	if (off >= (long)LT_OTA_BANK_A_OFF && off < (long)(LT_OTA_BANK_A_OFF + LT_OTA_BANK_LEN))
		return 0;
	if (off >= (long)LT_OTA_BANK_B_OFF && off < (long)(LT_OTA_BANK_B_OFF + LT_OTA_BANK_LEN))
		return 1;
	return -1;
}

// Bytes staged into a bank since its last erase (0 if none).
uint32_t lt_fal_ota_written(uint8_t bank) {
	if (bank >= LT_OTA_BANK_COUNT || ota_hi[bank] == 0)
		return 0;
	uint32_t base = bank ? LT_OTA_BANK_B_OFF : LT_OTA_BANK_A_OFF;
	return ota_hi[bank] - base;
}

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
	int wb = ota_bank_of(offset);
	if (wb >= 0) {
		uint32_t end = (uint32_t)(offset + (long)size);
		if (end > ota_hi[wb])
			ota_hi[wb] = end;
	}
	return (int)size;
}

static int erase(long offset, size_t size) {
	long start	   = offset & ~((long)FLASH_PAGE_SIZE - 1);
	long end	   = offset + (long)size;
	// Round the erased extent UP to a full page: MSC_ErasePage clears whole 4 KB
	// pages, so the region [start, end_pages) is actually blank afterwards.
	long end_pages = (end + (long)FLASH_PAGE_SIZE - 1) & ~((long)FLASH_PAGE_SIZE - 1);
	for (long a = start; a < end_pages; a += FLASH_PAGE_SIZE) {
		if (MSC_ErasePage((uint32_t *)(uintptr_t)a) != mscReturnOk)
			return -1;
	}
	for (int eb = 0; eb < LT_OTA_BANK_COUNT; eb++) {
		long base = eb ? (long)LT_OTA_BANK_B_OFF : (long)LT_OTA_BANK_A_OFF;
		if (start <= base && base < end_pages)
			ota_hi[eb] = 0;
	}
	// Return bytes erased AT/AFTER the caller's offset, not from the page-aligned
	// start. uf2ota records {erased_offset = offset, erased_length = <ret>} and
	// then skips re-erasing any sub-block whose [offset, offset+len) falls inside
	// that window. Returning (end - start) understates the window for a
	// page-aligned offset (reports 256 B when the whole 4 KB page is blank),
	// making every later block in the page trigger a re-erase that wipes the
	// blocks already written. Match the other FAL ports: report the full erased
	// span measured from offset. (OTA images are page-aligned, so this is
	// ceil(size/page)*page.)
	return (int)(end_pages - offset);
}

const struct fal_flash_dev flash0 = {
	.name		= FAL_FLASH_DEV_NAME,
	.addr		= 0x0,
	.len		= FLASH_LENGTH,
	.blk_size	= FLASH_ERASE_MIN_SIZE,
	.ops		= {init, read, write, erase},
	.write_gran = 1,
};
