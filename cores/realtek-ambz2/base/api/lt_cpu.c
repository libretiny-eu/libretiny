/* Copyright (c) Kuba Szczodrzyński 2023-05-22. */

#include <libretiny.h>
#include <sdk_private.h>

lt_cpu_model_t lt_cpu_get_model() {
	uint32_t *addr	   = (uint32_t *)0x40000038;
	uint8_t flash_mode = (addr[0] >> 5) & 0b11;
	uint32_t chip_id   = 0;
	hal_get_chip_id(&chip_id);
	chip_id <<= 2;
	return CPU_MODEL_ENUM(FAMILY, (chip_id & 0xFF) | flash_mode);
}

const char *lt_cpu_get_core_type() {
	return "ARM Cortex-M33 (ARMv8-M)";
}

uint32_t lt_cpu_get_freq() {
	return hal_syson_query_sys_clk();
}
