/* Copyright (c) Martin Prokopič 2024-12-03. */

#include <libretiny.h>
#include <sdk_private.h>

// private utilities from realtek-amb core
extern bool lt_ota_dual_get_offset(uint8_t index, uint32_t *offset);
extern uint8_t lt_ota_dual_get_stored_by_flag();
extern bool lt_ota_dual_switch_flag();

#define FLASH_SECTOR_SIZE		0x1000
#define IMAGE_PUBLIC_KEY_OFFSET 0x20
#define IMAGE_TYPE_OFFSET		0xE8
#define IMAGE_TYPE_VALUE		0x02 // FWHS_S
#define IMAGE_SERIAL_OFFSET		0xF4

typedef enum {
	LT_OTA_SWITCH_SERIAL_NUMBER = 1,
	LT_OTA_SWITCH_BIT_FLAG		= 2,
} lt_ota_switch_type_t;

// IMAGE_PUBLIC_KEY is defined by build script without braces to avoid shell escaping issues
static const uint8_t lt_image_public_key[] = {IMAGE_PUBLIC_KEY};

static lt_ota_switch_type_t ota_switch_type = 0;

/**
 * Scan the bootloader code to check the OTA switch method (bit flag or serial number).
 */
lt_ota_switch_type_t lt_ota_check_switch_type() {
	if (ota_switch_type)
		return ota_switch_type;

	for (uint32_t pos = 0; pos < FLASH_BOOT_LENGTH; pos++) {
		// if the bootloader contains the string, it uses a bit flag for switching OTA images
		uint8_t *data = (void *)(SPI_FLASH_BASE + FLASH_BOOT_OFFSET + pos);
		if (memcmp(data, "fw1 USE", 7) == 0)
			return (ota_switch_type = LT_OTA_SWITCH_BIT_FLAG);
	}
	return (ota_switch_type = LT_OTA_SWITCH_SERIAL_NUMBER);
}

/**
 * Check which OTA image is active using the serial number method.
 */
uint8_t lt_ota_dual_get_stored_by_sn() {
	// choose the one with a higher serial number
	// SN is always at offset 0xF4 into the OTA image
	uint32_t ota1_sn = LT_MEM32(SPI_FLASH_BASE + FLASH_OTA1_OFFSET + IMAGE_SERIAL_OFFSET);
	uint32_t ota2_sn = LT_MEM32(SPI_FLASH_BASE + FLASH_OTA2_OFFSET + IMAGE_SERIAL_OFFSET);
	if (ota2_sn > ota1_sn)
		return 2;
	return 1;
}

/**
 * Make the image invalid, if it isn't already.
 */
bool lt_ota_image_invalidate(uint32_t index) {
	if (!lt_ota_is_valid(index))
		return true;
	uint32_t offset;
	if (!lt_ota_dual_get_offset(index, &offset))
		return false;

	// read 32 bits at 0xE8 - points to image type (in image header)
	// - that byte is known to be 0x02, so it can be restored in lt_ota_image_revalidate()
	// - the image header is used during OTA hash calculation, unlike the public key
	uint32_t header_flags = LT_MEM32(SPI_FLASH_BASE + offset + IMAGE_TYPE_OFFSET);
	// zero-out the 1st byte (little-endian)
	header_flags &= ~0xFF;
	// write back to flash (erase not needed)
	return lt_flash_write(offset + IMAGE_TYPE_OFFSET, (void *)&header_flags, 4) == 4;
}

/**
 * Make the image valid again, if it isn't already.
 * Only works if the image was invalidated using `lt_ota_image_invalidate()`.
 */
bool lt_ota_image_revalidate(uint32_t index) {
	if (lt_ota_is_valid(index))
		return true;
	uint32_t offset;
	if (!lt_ota_dual_get_offset(index, &offset))
		return false;

	// read the entire sector (required for erase/write cycle)
	uint8_t *header = malloc(FLASH_SECTOR_SIZE);
	if (!header)
		return false;
	lt_flash_read(offset, header, FLASH_SECTOR_SIZE);

	// make sure the image type is currently 0x00
	if (header[IMAGE_TYPE_OFFSET] != 0x00)
		goto err;

	// set the image type back to 0x02
	header[IMAGE_TYPE_OFFSET] = IMAGE_TYPE_VALUE;

	// erase the sector and write it back (with a restored image type)
	if (!lt_flash_erase_block(offset))
		goto err;
	if (lt_flash_write(offset, header, FLASH_SECTOR_SIZE) != FLASH_SECTOR_SIZE)
		goto err;

	free(header);
	return true;
err:
	free(header);
	return false;
}

// public interface implementation

lt_ota_type_t lt_ota_get_type() {
	return OTA_TYPE_DUAL;
}

bool lt_ota_is_valid(uint8_t index) {
	uint32_t offset;
	if (!lt_ota_dual_get_offset(index, &offset))
		return false;

	// check the public key
	uint8_t *public_key = (void *)(SPI_FLASH_BASE + offset + IMAGE_PUBLIC_KEY_OFFSET);
	if (memcmp(public_key, lt_image_public_key, sizeof(lt_image_public_key)) != 0)
		return false;

	// check the image type
	if ((LT_MEM32(SPI_FLASH_BASE + offset + IMAGE_TYPE_OFFSET) & 0xFF) != IMAGE_TYPE_VALUE)
		return false;
	return true;
}

uint8_t lt_ota_dual_get_current() {
	// ambz2 uses virtual memory, so we can't use function address to determine active image
	// use the bootloader-exported boot info instead
	fw_img_export_info_type_t *info = hal_flash_boot_stubs.fw_img_info_tbl_query();
	return info->loaded_fw_idx;
}

uint8_t lt_ota_dual_get_stored() {
	bool ota1_valid = lt_ota_is_valid(1);
	bool ota2_valid = lt_ota_is_valid(2);

	// if both images are invalid, assume the current one is somehow still bootable
	if (!ota1_valid && !ota2_valid)
		return lt_ota_dual_get_current();

	// if one image is invalid, return the valid one
	if (!ota1_valid)
		return 2;
	if (!ota2_valid)
		return 1;

	// if both images seem valid - guess the bootloader's choice
	switch (lt_ota_check_switch_type()) {
		case LT_OTA_SWITCH_SERIAL_NUMBER:
			return lt_ota_dual_get_stored_by_sn();
		case LT_OTA_SWITCH_BIT_FLAG:
			return lt_ota_dual_get_stored_by_flag();
		default:
			return 1;
	}
}

bool lt_ota_switch(bool revert) {
	uint8_t current = lt_ota_dual_get_current();
	uint8_t target	= current ^ 0b11;

	uint8_t to_enable  = revert ? current : target;
	uint8_t to_disable = revert ? target : current;

	// enable first, so there is always at least one enabled image
	if (!lt_ota_image_revalidate(to_enable))
		return false;
	// make sure it's valid after (possibly) re-validating
	if (!lt_ota_is_valid(to_enable))
		return false;
	// finally, invalidate the other one
	lt_ota_image_invalidate(to_disable);

	// additionally, adjust the bit flag
	uint8_t stored_flag = lt_ota_dual_get_stored_by_flag();
	if (stored_flag != to_enable)
		lt_ota_dual_switch_flag();

	return true;
}
