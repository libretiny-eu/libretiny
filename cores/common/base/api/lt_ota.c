/* Copyright (c) Kuba Szczodrzyński 2022-04-29. */

#include "lt_ota.h"

bool lt_ota_can_rollback() {
	if (lt_ota_get_type() != OTA_TYPE_DUAL)
		return false;
	uint8_t current = lt_ota_dual_get_current();
	return lt_ota_is_valid(current ^ 0b11);
}

uf2_ota_scheme_t lt_ota_get_uf2_scheme() {
	if (lt_ota_get_type() == OTA_TYPE_SINGLE)
		return UF2_SCHEME_DEVICE_SINGLE;
	uint8_t current = lt_ota_dual_get_current();
	// UF2_SCHEME_DEVICE_DUAL_1 or UF2_SCHEME_DEVICE_DUAL_2
	return (uf2_ota_scheme_t)(current ^ 0b11);
}
