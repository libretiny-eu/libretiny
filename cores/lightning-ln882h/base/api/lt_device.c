/* Copyright (c) Etienne Le Cousin 2024-03-03. */

#include <libretiny.h>
#include <sdk_private.h>

void lt_get_device_mac(uint8_t *mac) {
	ln_fotp_get_mac_val(mac);
}

void lt_reboot() {
	ln_chip_reboot();
	while (1) {}
}

lt_reboot_reason_t lt_get_reboot_reason() {
	chip_reboot_cause_t reason = ln_chip_get_reboot_cause();
	switch (reason) {
		case CHIP_REBOOT_POWER_ON:
			return REBOOT_REASON_POWER;
		case CHIP_REBOOT_SOFTWARE:
			return REBOOT_REASON_SOFTWARE;
		case CHIP_REBOOT_WATCHDOG:
			return REBOOT_REASON_WATCHDOG;
		default:
			return REBOOT_REASON_UNKNOWN;
	}
}
