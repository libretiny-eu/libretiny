/* Copyright (c) Kuba Szczodrzyński 2023-02-27. */

#include <libretiny.h>
#include <sdk_private.h>

lt_ota_type_t lt_ota_get_type() {
	return OTA_TYPE_DUAL;
}

bool lt_ota_is_valid(uint8_t index) {
	uint32_t offset;
	switch (index) {
		case 1:
			offset = FLASH_OTA1_OFFSET;
			break;
		case 2:
			offset = FLASH_OTA2_OFFSET;
			break;
		default:
			return false;
	}
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
	uint32_t ota_counter = *((uint32_t *)0x8009004);
	// even count of zero-bits means OTA1, odd count means OTA2
	// this allows to switch OTA images by simply clearing next bits,
	// without needing to erase the flash
	uint8_t count = 0;
	for (uint8_t i = 0; i < 32; i++) {
		if ((ota_counter & (1 << i)) == 0)
			count++;
	}
	return 1 + (count % 2);
}

bool lt_ota_switch(bool revert) {
	uint8_t current = lt_ota_dual_get_current();
	uint8_t stored	= lt_ota_dual_get_stored();
	if ((current == stored) == revert)
		return true;

	if (!lt_ota_is_valid(stored ^ 0b11))
		return false;

	// - read current OTA switch value from 0x9004
	// - reset OTA switch to 0xFFFFFFFE if it's 0x0
	// - else check first non-zero bit of OTA switch
	// - write OTA switch with first non-zero bit cleared

	uint32_t value = HAL_READ32(SPI_FLASH_BASE, FLASH_SYSTEM_OFFSET + 4);
	if (value == 0) {
		uint8_t *system = (uint8_t *)malloc(64);
		lt_flash_read(FLASH_SYSTEM_OFFSET, system, 64);
		// reset OTA switch
		((uint32_t *)system)[1] = -2;
		lt_flash_erase_block(FLASH_SYSTEM_OFFSET);
		return lt_flash_write(FLASH_SYSTEM_OFFSET, system, 64);
	}

	// clear first non-zero bit
	value <<= 1;
	// write OTA switch to flash
	flash_write_word(NULL, FLASH_SYSTEM_OFFSET + 4, value);
	return true;
}
