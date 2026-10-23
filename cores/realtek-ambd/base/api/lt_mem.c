/* Copyright (c) Bl00d-B0b 2026-08-07. */

#include <libretiny.h>
#include <sdk_private.h>

uint32_t lt_ram_get_size() {
	// KM4 view: 256K SRAM + 256K "extension" SRAM (BW16 has no PSRAM).
	return 512 * 1024;
}

uint32_t lt_heap_get_size() {
	// heap_5 regions are sized at runtime by os_heap_init(); the high-water
	// mark plus what is free now is the closest thing to a total.
	extern size_t xPortGetFreeHeapSize(void);
	extern size_t xPortGetMinimumEverFreeHeapSize(void);
	size_t free_now = xPortGetFreeHeapSize();
	size_t free_min = xPortGetMinimumEverFreeHeapSize();
	return free_now > free_min ? free_now : free_min;
}
