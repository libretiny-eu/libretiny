// BLE MAC derivation for LN882H.
//
// The BLE SDK has no factory BLE address of its own — ln_kv_ble_pub_addr_get()
// returns a fixed placeholder (BLE_DEFAULT_PUBLIC_ADDR, 00:50:C2:00:00:00) on a
// blank device, identical on every chip. Derive a unique BLE address from the
// WiFi STA MAC (recovered/persisted by lt_init): low 24-bit NIC + 1 with
// wraparound, OUI unchanged, MSB-first. Reproduces the BLE = WiFi + 1 pairing
// seen across Tuya LN882H flash dumps without reading the Tuya KV, so it works
// on any board.
#include <libretiny.h>
#include <sdk_private.h>

#include "lt_ble.h"

bool lt_ble_mac_get(uint8_t out[6]) {
	if (sysparam_sta_mac_get(out) != SYSPARAM_ERR_NONE)
		return false;
	uint32_t nic = ((((uint32_t)out[3] << 16) | ((uint32_t)out[4] << 8) | out[5]) + 1) & 0xFFFFFF;
	out[3]		 = (uint8_t)(nic >> 16);
	out[4]		 = (uint8_t)(nic >> 8);
	out[5]		 = (uint8_t)(nic >> 0);
	return true;
}
