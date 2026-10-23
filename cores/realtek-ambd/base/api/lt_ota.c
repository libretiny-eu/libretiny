/* Copyright (c) Bl00d-B0b 2026-08-06. */

#include <libretiny.h>
#include <sdk_private.h>

// AmebaD OTA slot selection: each slot starts with the 8-byte signature
// "81958711"; the bootloader boots OTA2 when its signature is valid, OTA1
// otherwise. Switching = write the target signature, blank the other.
#define IMG2_SIGN		"81958711"
#define AMBD_OTA_SIGN_0 0x35393138 // "8195"
#define AMBD_OTA_SIGN_1 0x31313738 // "8711"

lt_ota_type_t lt_ota_get_type() {
	return OTA_TYPE_DUAL;
}

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
 * Check which OTA image the bootloader would pick, by the OTA2 slot signature.
 */
uint8_t lt_ota_dual_get_stored_by_flag() {
	uint32_t sig[2];
	if (lt_flash_read(FLASH_OTA2_OFFSET, (void *)sig, 8) != 8)
		return 1;
	return (sig[0] == AMBD_OTA_SIGN_0 && sig[1] == AMBD_OTA_SIGN_1) ? 2 : 1;
}

// The signature is part of the image, so an update written into OTA2 changes
// what lt_ota_dual_get_stored_by_flag() reports; latch the running slot before
// the first flash write (lt_ota_begin() precedes all writes).
static uint8_t s_boot_slot = 0;

static uint8_t lt_ota_boot_slot() {
	if (s_boot_slot == 0)
		s_boot_slot = lt_ota_dual_get_stored_by_flag();
	return s_boot_slot;
}

// The vendor tree defines this in rtl8721d_ota.c, which stays out of the
// build (SD-card/HTTP OTA app code); app_start references it. lt_reboot()
// reproduces its backup-register + watchdog full-chip reset.
void ota_platform_reset(void) {
	lt_reboot();
}

// FLASH_WriteStream/FLASH_EreaseDwordsXIP/FLASH_TxData12BXIP come from
// rtl8721d_flash.h via ameba_soc.h (sdk_private.h).

/**
 * Activate the slot the update was written to: make sure its signature is valid
 * first, then blank the old slot's, so a crash in between leaves the old image
 * bootable.
 */
bool lt_ota_dual_switch_flag() {
	u32 sig[2]		 = {AMBD_OTA_SIGN_0, AMBD_OTA_SIGN_1};
	u8 blank[8]		 = {0};
	uint8_t target	 = lt_ota_boot_slot() ^ 0b11;
	uint32_t tgt_off = target == 2 ? FLASH_OTA2_OFFSET : FLASH_OTA1_OFFSET;
	uint32_t old_off = target == 2 ? FLASH_OTA1_OFFSET : FLASH_OTA2_OFFSET;
	if (!lt_ota_is_valid(target)) {
		// The update did not carry the signature: write it.
		FLASH_EreaseDwordsXIP(tgt_off, 2);
		FLASH_TxData12BXIP(tgt_off, 8, (u8 *)sig);
		if (!lt_ota_is_valid(target))
			return false;
	}
	if (target == 1) {
		// Activating OTA1 invalidates OTA2's signature, the slot this code runs from:
		// program zeros over it with the XIP-safe writer like the SDK's OTA_Change
		// (erasing the active sector hard-faults), then drop the cache line.
		FLASH_TxData12BXIP(old_off, 8, blank);
		DCache_Invalidate(SPI_FLASH_BASE + old_off, 8);
	}
	return lt_ota_dual_get_stored_by_flag() == target;
}

bool lt_ota_is_valid(uint8_t index) {
	uint32_t offset;
	if (!lt_ota_dual_get_offset(index, &offset))
		return false;
	uint8_t head[8];
	if (lt_flash_read(offset, head, sizeof(head)) != sizeof(head))
		return false;
	return memcmp(head, IMG2_SIGN, 8) == 0;
}

uint8_t lt_ota_dual_get_current() {
	// KM4 image2 runs from the 0x0E000000 XIP window mapped onto the active slot,
	// so the code address carries no flash offset; the booted slot is the
	// signature state latched before the first OTA write.
	return lt_ota_boot_slot();
}

uint8_t lt_ota_dual_get_stored() {
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
