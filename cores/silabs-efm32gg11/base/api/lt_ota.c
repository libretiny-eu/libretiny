/* Copyright (c) WGM160P-LibreTiny port 2026-05-26.
 *
 * Phase 1 OTA API stub for silabs-efm32gg11.
 * Phase 2 or 3 picks up OTA via the Gecko Bootloader.
 *
 * NOTE: cores/common/base/api/lt_ota.c provides defaults for
 * lt_ota_begin / lt_ota_end / lt_ota_write / lt_ota_write_block /
 * lt_ota_can_rollback / lt_ota_get_uf2_scheme, plus a weak
 * lt_ota_set_write_protect. The family must provide:
 *   - lt_ota_get_type()
 *   - lt_ota_is_valid(uint8_t index)
 *   - lt_ota_dual_get_current()
 *   - lt_ota_dual_get_stored()
 *   - lt_ota_switch(bool revert)
 *
 * Signatures match cores/common/base/api/lt_ota.h exactly.
 */

#include "lt_family.h"
#include <libretiny.h>

lt_ota_type_t lt_ota_get_type(void) {
	return OTA_TYPE_SINGLE;
}

bool lt_ota_is_valid(uint8_t index) {
	(void)index;
	return false; // no OTA image staged in Phase 1
}

uint8_t lt_ota_dual_get_current(void) {
	return 0; // single-image device
}

uint8_t lt_ota_dual_get_stored(void) {
	return 0; // single-image device
}

bool lt_ota_switch(bool revert) {
	(void)revert;
	return false; // no upgrade image to activate in Phase 1
}
