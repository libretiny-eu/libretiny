/* Copyright (c) Bl00d-B0b 2026-08-06. */

#include <libretiny.h>
#include <sdk_private.h>

// KM4 fixed clock; the SDK exposes no runtime scaling for the app core.
#define AMBD_KM4_FREQ 200000000

uint32_t lt_cpu_get_mac_id(void) {
	// EFUSE logical map carries the WiFi MAC at 0x11A (same layout family as
	// AmebaZ2). TODO(bench): verify offset against a BW16 efuse dump.
	uint8_t mac[6];
	extern int efuse_logical_read(uint16_t addr, uint16_t cnts, uint8_t *data);
	efuse_logical_read(0x11A, 6, mac);
	return mac[3] << 16 | mac[4] << 8 | mac[5];
}

uint32_t lt_cpu_get_freq(void) {
	return AMBD_KM4_FREQ;
}
