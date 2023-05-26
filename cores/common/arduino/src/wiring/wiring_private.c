/* Copyright (c) Kuba Szczodrzyński 2023-05-24. */

#include "wiring_private.h"

#if __has_include(<wiring_data.h>)
/**
 * @brief Allocate and return a PinData structure (family-specific).
 */
PinData *pinData(PinInfo *pin) {
	if (pin->data == NULL) {
		pin->data = calloc(1, sizeof(PinData));
	}
	return (PinData *)pin->data;
}

/**
 * @brief Deallocate the PinData structure.
 */
void pinRemoveData(PinInfo *pin) {
	if (pin->data != NULL) {
		free(pin->data);
	}
	pin->data = NULL;
}
#endif
