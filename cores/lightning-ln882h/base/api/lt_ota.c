/* Copyright (c) Etienne Le Cousin 2024-12-21. */

#include <libretiny.h>
#include <ota_image.h>
#include <sdk_private.h>

lt_ota_type_t lt_ota_get_type() {
	return OTA_TYPE_SINGLE;
}

bool lt_ota_is_valid(uint8_t index) {
	image_hdr_t ota_header;

	if (OTA_ERR_NONE != image_header_fast_read(FLASH_OTA_OFFSET, &ota_header)) {
		return false;
	}

	if (OTA_ERR_NONE != image_header_verify(&ota_header)) {
		return false;
	}

	if (OTA_ERR_NONE != image_body_verify(FLASH_OTA_OFFSET, &ota_header)) {
		return false;
	}
	return true;
}

uint8_t lt_ota_dual_get_current() {
	return 0;
}

uint8_t lt_ota_dual_get_stored() {
	return 0;
}

void lt_ota_set_write_protect(uf2_ota_t *uf2) {
	LT_DM(OTA, "lt_ota_set_write_protect");
	ln_nvds_set_ota_upg_state(UPG_STATE_DOWNLOAD_ING);
}

bool lt_ota_switch(bool revert) {
	LT_DM(OTA, "lt_ota_switch(%d)", revert);
	ln_nvds_set_ota_upg_state(UPG_STATE_DOWNLOAD_OK);
	return true;
}
