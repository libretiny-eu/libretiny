/* Copyright (c) Etienne Le Cousin 2024-12-21. */

#include <libretiny.h>
#include <sdk_private.h>

typedef enum {
	UPG_STATE_DOWNLOAD_ING  = 0u,        /*< ota image downloading */
	UPG_STATE_DOWNLOAD_OK   = 1u,        /*< ota image download finish and check success */
	UPG_STATE_RESTORE_ING   = 2u,        /*< ota image restoring */
	UPG_STATE_RESTORE_OK    = 3u,        /*< ota image restore finish and check success */
	UPG_STATE_RESTORE_FILED = 4u,        /*< ota image restore filed */
	UPG_STATE_REPORT_OK     = 5u,        /*< ota remote server was notified of the result successfully */
	UPG_STATE_PLACE = 0x12345678,        /*< make different IDEs compatible */
} upg_state_t;


lt_ota_type_t lt_ota_get_type() {
	return OTA_TYPE_SINGLE;
}

bool lt_ota_is_valid(uint8_t index) {
	return true;
}

uint8_t lt_ota_dual_get_current() {
	return 0;
}

uint8_t lt_ota_dual_get_stored() {
	return 0;
}

void lt_ota_set_write_protect(uf2_ota_t *uf2) 
{
	LT_DM(OTA, "lt_ota_set_write_protect");
	ln_nvds_set_ota_upg_state(UPG_STATE_DOWNLOAD_ING);
}

bool lt_ota_switch(bool revert) {
	LT_DM(OTA, "lt_ota_switch(%d)", revert);
	ln_nvds_set_ota_upg_state(UPG_STATE_DOWNLOAD_OK);
	return true;
}
