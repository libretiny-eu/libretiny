/* Copyright (c) LibreTiny contributors. */

#pragma once

#include <stdbool.h>
#include <stdint.h>
#include <string.h>

// Sector kept by the Broadlink firmware on modules without a MAC in the eFuse.
// Layout (only the part needed here): "CALD" magic, 4 unused bytes, MAC.
#define LT_CALD_FLASH_OFFSET 0x4000
#define LT_CALD_MAC_OFFSET	 8
#define LT_CALD_HEADER_SIZE	 (LT_CALD_MAC_OFFSET + 6)

/**
 * Extract the MAC from the first LT_CALD_HEADER_SIZE bytes of the sector.
 * Fails if the magic is missing (e.g. the sector holds a bootloader instead) or
 * if the address cannot be a station address: all zero, all ones (erased flash)
 * or multicast.
 */
static inline bool lt_cald_parse_mac(const uint8_t *header, uint8_t *mac) {
	if (memcmp(header, "CALD", 4) != 0)
		return false;

	const uint8_t *candidate = header + LT_CALD_MAC_OFFSET;
	uint8_t anyBit			 = 0;
	uint8_t allBits			 = 0xFF;
	for (int i = 0; i < 6; i++) {
		anyBit |= candidate[i];
		allBits &= candidate[i];
	}
	if (anyBit == 0 || allBits == 0xFF || (candidate[0] & 0x01))
		return false;

	memcpy(mac, candidate, 6);
	return true;
}
