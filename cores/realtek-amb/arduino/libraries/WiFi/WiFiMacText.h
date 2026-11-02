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

/**
 * Read a MAC address given as twelve hex digits, e.g. "ec0bae9e21c1".
 * Colons, dashes and quote characters in between are skipped, so
 * "ec:0b:ae:9e:21:c1" works as well. Anything else, or a digit count other than
 * twelve, fails and leaves `mac` untouched.
 */
static inline bool wifiMacFromText(const char *text, uint8_t *mac) {
	if (text == nullptr)
		return false;

	uint8_t parsed[6] = {};
	unsigned digits	  = 0;
	for (; *text; text++) {
		char c = *text;
		if (c == ':' || c == '-' || c == '"' || c == '\\')
			continue;

		unsigned value;
		if (c >= '0' && c <= '9')
			value = c - '0';
		else if (c >= 'a' && c <= 'f')
			value = c - 'a' + 10;
		else if (c >= 'A' && c <= 'F')
			value = c - 'A' + 10;
		else
			return false;

		if (digits >= 12)
			return false;
		parsed[digits / 2] = (digits % 2) ? (parsed[digits / 2] | value) : (value << 4);
		digits++;
	}
	if (digits != 12)
		return false;

	for (unsigned i = 0; i < 6; i++)
		mac[i] = parsed[i];
	return true;
}
