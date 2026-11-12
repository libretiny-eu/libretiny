/* Copyright (c) Kuba Szczodrzyński 2023-02-27. */
/* Ported to Realtek Ameba1 (RTL8195A / RTL8711AM) — Cortex-M3. */

#include <libretiny.h>
#include <sdk_private.h>

lt_cpu_model_t lt_cpu_get_model() {
	// RTL8195A/RTL8711AM is a single-variant family; there is no per-chip id
	// efuse read like AmebaZ's EFUSE_OneByteReadROM(0xF8).
	return CPU_MODEL_ENUM(FAMILY, 0);
}

uint32_t lt_cpu_get_mac_id() {
	uint8_t mac[6] = {0};
	lt_get_device_mac(mac);
	return ((uint32_t)mac[3] << 16) | ((uint32_t)mac[4] << 8) | mac[5];
}

const char *lt_cpu_get_core_type() {
	return "ARM Cortex-M3 (ARMv7-M)";
}

uint32_t lt_cpu_get_freq() {
	return HalGetCpuClk();
}
