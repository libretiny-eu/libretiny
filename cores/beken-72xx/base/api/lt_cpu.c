/* Copyright (c) Kuba Szczodrzyński 2023-02-27. */

#include <libretiny.h>
#include <sdk_private.h>

lt_cpu_model_t lt_cpu_get_model() {
	uint8_t chipId = *(uint8_t *)(SCTRL_CHIP_ID);
	return CPU_MODEL_ENUM(FAMILY, chipId);
}

const char *lt_cpu_get_core_type() {
	return "ARM968E-S (ARMv5TE)";
}
