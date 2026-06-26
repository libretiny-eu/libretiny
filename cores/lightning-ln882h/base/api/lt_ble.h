// BLE MAC derivation for LN882H — see lt_ble.c.
#pragma once

#include <stdbool.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

// Derive the BLE MAC from the WiFi STA MAC: low 24-bit NIC + 1 (wraparound),
// OUI unchanged, written MSB-first (out[0] = OUI). Returns false and leaves out
// unmodified if the WiFi STA MAC is unavailable.
bool lt_ble_mac_get(uint8_t out[6]);

#ifdef __cplusplus
}
#endif
