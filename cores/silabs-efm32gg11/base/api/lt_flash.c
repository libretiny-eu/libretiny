/* Copyright (c) WGM160P-LibreTiny port 2026-05-26.
 *
 * Phase 1 flash API for silabs-efm32gg11.
 *
 * cores/common/base/api/lt_flash.c provides STRONG implementations of
 * lt_flash_erase, lt_flash_erase_block, lt_flash_read, lt_flash_write
 * that route to FAL (fal_partition_* against fal_root_part). If FAL is
 * not initialized (Phase 1 — no partition table wired), those calls
 * return -1/0 to the caller, which is exactly the desired "not
 * implemented" Phase 1 behaviour. We don't redefine them here (that
 * would cause a duplicate-symbol link error).
 *
 * lt_flash_get_size has a weak default in common; our override returns
 * the real EFM32GG11B820 internal flash size.
 *
 * lt_flash_get_id is non-weak in the common API and must be provided
 * by each family — that's what this file is for.
 *
 * Phase 2's OTA path will wire emlib MSC_* into FAL via a fal_flash_dev
 * port in this directory, so common's lt_flash_* immediately start
 * working without further family changes.
 */

#include "lt_family.h"
#include <libretiny.h>

lt_flash_id_t lt_flash_get_id(void) {
	// EFM32GG11B has internal flash only (no external SPI flash).
	// Return a synthetic identifier so callers querying chip ID get
	// something stable. Manufacturer 0x00 (no JEDEC ID for internal flash).
	lt_flash_id_t id = {
		.manufacturer_id = 0x00,
		.chip_id		 = 0x00,
		.chip_size_id	 = 0x15, // 2^21 = 2 MiB (matches EFM32GG11B820 flash size)
	};
	return id;
}

uint32_t lt_flash_get_size(void) {
	return 2 * 1024 * 1024; // 2 MiB internal flash on EFM32GG11B820
}
