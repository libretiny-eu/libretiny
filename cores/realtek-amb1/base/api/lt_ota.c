/* The fridge image2/HMAC A/B loader is not the AmebaZ XIP OTA protocol.
 * Fail closed until its slot selection and authenticated update are ported. */
#include <libretiny.h>

lt_ota_type_t lt_ota_get_type(void) { return OTA_TYPE_DUAL; }
bool lt_ota_is_valid(uint8_t index) { (void)index; return false; }
uint8_t lt_ota_dual_get_current(void) { return 0; }
uint8_t lt_ota_dual_get_stored(void) { return 0; }
bool lt_ota_switch(bool revert) { (void)revert; return false; }
