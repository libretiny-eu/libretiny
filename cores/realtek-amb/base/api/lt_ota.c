/* Copyright (c) Kuba Szczodrzyński 2026-02-01. */

#include <libretiny.h>
#include <sdk_private.h>

#define SYSTEM_DATA_LENGTH 128

/**
 * Convert OTA index (1, 2) into partition offset.
 */
bool lt_ota_dual_get_offset(uint8_t index, uint32_t *offset) {
	switch (index) {
		case 1:
			*offset = FLASH_OTA1_OFFSET;
			break;
		case 2:
			*offset = FLASH_OTA2_OFFSET;
			break;
		default:
			return false;
	}
	return true;
}

/**
 * Check which OTA image is active using the bit flag method.
 */
uint8_t lt_ota_dual_get_stored_by_flag() {
	// find first non-zero bit of the flag in system data:
	// - even count of zero-bits means OTA1, odd count means OTA2
	//   this allows to switch OTA images by simply clearing next bits,
	//   without needing to erase the flash
	uint32_t ota_counter = LT_MEM32(SPI_FLASH_BASE + FLASH_SYSTEM_OFFSET + 4);
	uint8_t bitidx;
	for (bitidx = 0; bitidx < 32; bitidx++) {
		if (ota_counter & (1 << bitidx))
			break;
	}
	return 1 + (bitidx & 1);
}

/**
 * Flip the OTA selection bit flag switch.
 */
bool lt_ota_dual_switch_flag() {
	uint32_t bit_flag = LT_MEM32(SPI_FLASH_BASE + FLASH_SYSTEM_OFFSET + 4);

	if (bit_flag == 0) {
		// allocate memory and read old system data contents
		uint32_t *system = malloc(SYSTEM_DATA_LENGTH);
		if (!system)
			return false;
		if (lt_flash_read(FLASH_SYSTEM_OFFSET, (void *)system, SYSTEM_DATA_LENGTH) != SYSTEM_DATA_LENGTH)
			goto err;

		// set bit flag to OTA 2
		system[1] = 0xFFFFFFFE;

		// erase and write
		if (!lt_flash_erase_block(FLASH_SYSTEM_OFFSET))
			goto err;
		if (lt_flash_write(FLASH_SYSTEM_OFFSET, (void *)system, SYSTEM_DATA_LENGTH) != SYSTEM_DATA_LENGTH)
			goto err;

		free(system);
		return true;
	err:
		free(system);
		return false;
	}

	// shift the bit flag and write without erasing
	bit_flag <<= 1;
	return lt_flash_write(FLASH_SYSTEM_OFFSET + 4, (void *)&bit_flag, 4) == 4;
}
