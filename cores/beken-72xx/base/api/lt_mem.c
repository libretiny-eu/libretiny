/* Copyright (c) Kuba Szczodrzyński 2023-02-27. */

#include <libretiny.h>
#include <sdk_private.h>

uint32_t lt_ram_get_size() {
	return 256 * 1024;
}

uint32_t lt_heap_get_size() {
#if configDYNAMIC_HEAP_SIZE
	extern unsigned char _empty_ram;
#if CFG_SOC_NAME == SOC_BK7231N
	return (0x00400000 + 192 * 1024) - (uint32_t)(&_empty_ram);
#else
	return (0x00400000 + 256 * 1024) - (uint32_t)(&_empty_ram);
#endif
#else
	return configTOTAL_HEAP_SIZE;
#endif
}
