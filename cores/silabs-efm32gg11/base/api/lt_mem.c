/* Copyright (c) WGM160P-LibreTiny port 2026-05-26.
 *
 * Phase 1 memory API for silabs-efm32gg11.
 *
 * cores/common/base/api/lt_mem.c provides weak defaults (when LT_HAS_FREERTOS)
 * for lt_heap_get_size, lt_heap_get_free, lt_heap_get_min_free using FreeRTOS
 * heap stats. We only need to provide lt_ram_get_size, which common does not
 * default. EFM32GG11B820 has 512 KiB of SRAM.
 */

#include "lt_family.h"
#include <libretiny.h>

uint32_t lt_ram_get_size(void) {
	return 512 * 1024;
}
