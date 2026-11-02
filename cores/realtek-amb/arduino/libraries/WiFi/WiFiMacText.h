/* Copyright (c) LibreTiny contributors. */

#pragma once

#include <stdbool.h>
#include <stdint.h>

// Text buffer size for "aa:bb:cc:dd:ee:ff" including the terminator.
#define WIFI_MAC_TEXT_SIZE 18

/**
 * Check whether a MAC address may be assigned to the station interface.
 *
 * The Realtek driver silently replaces an all-zero or all-ones address with
 * its built-in default, so those are rejected here instead. Multicast
 * addresses (lowest bit of the first byte set) are not valid station addresses.
 */
static inline bool wifiMacIsAssignable(const uint8_t *mac) {
	if (mac == nullptr || (mac[0] & 0x01))
		return false;
	uint8_t anyBit = 0;
	for (unsigned i = 0; i < 6; i++)
		anyBit |= mac[i];
	return anyBit != 0;
}

/**
 * Format a MAC address the way the Realtek driver parses `rtw_initmac`:
 * six lowercase hex pairs separated by colons. `text` must hold
 * WIFI_MAC_TEXT_SIZE bytes.
 */
static inline void wifiMacToText(const uint8_t *mac, char *text) {
	static const char digits[] = "0123456789abcdef";
	for (unsigned i = 0; i < 6; i++) {
		text[3 * i]		= digits[mac[i] >> 4];
		text[3 * i + 1] = digits[mac[i] & 0x0F];
		text[3 * i + 2] = (i < 5) ? ':' : '\0';
	}
}
