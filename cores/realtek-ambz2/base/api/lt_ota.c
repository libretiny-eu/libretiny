/* Copyright (c) Kuba Szczodrzyński 2023-05-22. */

#include <libretiny.h>
#include <sdk_private.h>
#include <osdep_service.h>
#include <device_lock.h>

// from SDK
extern uint32_t sys_update_ota_get_curr_fw_idx(void);

#define FLASH_SECTOR_SIZE		0x1000
// IMAGE_PUBLIC_KEY is defined by the build script
#define IMAGE_PUBLIC_KEY_OFFSET 32
#define IMAGE_PUBLIC_KEY_LENGTH 32

typedef enum {
	INVALID	 = 0,
	DISABLED = 1,
	ENABLED	 = 2,
} lt_ota_image_state_t;

static bool lt_ota_get_image_offset(uint8_t index, uint32_t *offset) {
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

static uint8_t lt_ota_get_other_index(uint8_t index) {
	return index ^ 0b11; // 1 -> 2, 2 -> 1
}

static lt_ota_image_state_t lt_ota_get_image_state(uint8_t index) {
	uint32_t offset;
	if (!lt_ota_get_image_offset(index, &offset))
		return INVALID;

	uint8_t public_key[IMAGE_PUBLIC_KEY_LENGTH];
	uint32_t num_read = lt_flash_read(offset + IMAGE_PUBLIC_KEY_OFFSET, public_key, sizeof(public_key));
	if (num_read != sizeof(public_key))
		return INVALID;

	if (memcmp(public_key, IMAGE_PUBLIC_KEY, sizeof(public_key)) == 0)
		return ENABLED;

	public_key[0] = ~(public_key[0]);
	if (memcmp(public_key, IMAGE_PUBLIC_KEY, sizeof(public_key)) == 0)
		return DISABLED;

	return INVALID;
}

static bool lt_ota_set_image_enabled(uint8_t index, bool new_enabled) {
	uint32_t offset;
	if (!lt_ota_get_image_offset(index, &offset))
		return false;

	_irqL irqL;
	uint8_t *header = (uint8_t *)malloc(FLASH_SECTOR_SIZE);

	rtw_enter_critical(NULL, &irqL);
	device_mutex_lock(RT_DEV_LOCK_FLASH);
	flash_stream_read(&lt_flash_obj, offset, FLASH_SECTOR_SIZE, header);

	bool enabled = header[IMAGE_PUBLIC_KEY_OFFSET] == IMAGE_PUBLIC_KEY[0];
	if (enabled != new_enabled) {
		// negate first byte of OTA signature
		header[0] = ~(header[0]);
		// negate first byte of public key
		header[IMAGE_PUBLIC_KEY_OFFSET] = ~(header[IMAGE_PUBLIC_KEY_OFFSET]);

		// write to flash
		hal_flash_sector_erase(lt_flash_obj.phal_spic_adaptor, offset);
		hal_flash_burst_write(lt_flash_obj.phal_spic_adaptor, FLASH_SECTOR_SIZE, offset, header);
	}

	device_mutex_unlock(RT_DEV_LOCK_FLASH);
	rtw_exit_critical(NULL, &irqL);
	free(header);

	return true;
}

// public interface implementation

lt_ota_type_t lt_ota_get_type() {
	return OTA_TYPE_DUAL;
}

bool lt_ota_is_valid(uint8_t index) {
	return lt_ota_get_image_state(index) != INVALID;
}

uint8_t lt_ota_dual_get_current() {
	// ambz2 uses virtual memory, so we can't use function address to determine active image
	// use the SDK instead
	return sys_update_ota_get_curr_fw_idx();
}

uint8_t lt_ota_dual_get_stored() {
	// bootloader prioritizes FW1 if both are valid
	return lt_ota_get_image_state(1) == ENABLED ? 1 : 2;
}

bool lt_ota_switch(bool revert) {
	uint8_t current = lt_ota_dual_get_current();
	uint8_t stored	= lt_ota_dual_get_stored();
	if ((current == stored) == revert)
		return true;

	uint8_t to_enable  = lt_ota_get_other_index(stored);
	uint8_t to_disable = stored;

	if (!lt_ota_is_valid(to_enable))
		return false;

	// enable first, so there is always at least one enabled image
	if (!lt_ota_set_image_enabled(to_enable, true))
		return false;
	if (!lt_ota_set_image_enabled(to_disable, false))
		return false;

	return true;
}
