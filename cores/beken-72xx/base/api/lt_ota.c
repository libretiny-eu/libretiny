/* Copyright (c) Kuba Szczodrzyński 2023-02-27. */

#include <libretiny.h>
#include <sdk_private.h>

lt_ota_type_t lt_ota_get_type() {
	return OTA_TYPE_SINGLE;
}

bool lt_ota_is_valid(uint8_t index) {
	if (index != 0)
		return false;
	// check download RBL
	// TODO: maybe check header CRC or even binary hashes
	uint32_t magic;
	lt_flash_read(FLASH_DOWNLOAD_OFFSET, (uint8_t *)&magic, 4);
	return magic == 0x004C4252; // "RBL\0", little-endian
}

uint8_t lt_ota_dual_get_current() {
	return 0;
}

uint8_t lt_ota_dual_get_stored() {
	return 0;
}

bool lt_ota_switch(bool revert) {
	if (!lt_ota_is_valid(0))
		// no valid "download" image
		// - return false when trying to activate
		// - return true when trying to revert
		return revert;
	if (revert) {
		// there's a valid "download" image, which has to be removed
		return lt_flash_erase_block(FLASH_DOWNLOAD_OFFSET);
	}
	return true;
}
