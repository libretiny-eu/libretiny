/* Copyright (c) Kuba Szczodrzyński 2023-02-27. */

#include <libretiny.h>
#include <sdk_private.h>

// private utilities from realtek-amb core
extern bool lt_ota_dual_get_offset(uint8_t index, uint32_t *offset);
extern uint8_t lt_ota_dual_get_stored_by_flag();
extern bool lt_ota_dual_switch_flag();

lt_ota_type_t lt_ota_get_type() {
	return OTA_TYPE_DUAL;
}

bool lt_ota_is_valid(uint8_t index) {
	uint32_t offset;
	if (!lt_ota_dual_get_offset(index, &offset))
		return false;
	uint8_t *address = (uint8_t *)(SPI_FLASH_BASE + offset);
	return memcmp(address, "81958711", 8) == 0;
}

uint8_t lt_ota_dual_get_current() {
	// RTL8710B is XIP, so check the code offset in flash
	uint32_t addr = (uint32_t)lt_log;
	uint32_t offs = addr - SPI_FLASH_BASE;
	return offs > FLASH_OTA2_OFFSET ? 2 : 1;
}

uint8_t lt_ota_dual_get_stored() {
	uint32_t *ota_address = (uint32_t *)0x8009000;
	if (*ota_address == 0xFFFFFFFF)
		return 1;
	return lt_ota_dual_get_stored_by_flag();
}

bool lt_ota_switch(bool revert) {
	uint8_t current = lt_ota_dual_get_current();
	uint8_t stored	= lt_ota_dual_get_stored();
	if ((current == stored) == revert)
		return true;

	if (!lt_ota_is_valid(stored ^ 0b11))
		return false;

	return lt_ota_dual_switch_flag();
}
