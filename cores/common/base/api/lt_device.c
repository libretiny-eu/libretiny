/* Copyright (c) Kuba Szczodrzyński 2022-04-29. */

#include "lt_device.h"

static char *device_name = NULL;

const char *lt_get_version() {
	return LT_VERSION_STR;
}

const char *lt_get_board_code() {
	return LT_BOARD_STR;
}

const char *lt_get_device_name() {
	if (device_name)
		return device_name;
	uint32_t chip_id = lt_cpu_get_mac_id();
	uint8_t *id		 = (uint8_t *)&chip_id;

	const char *model = lt_cpu_get_model_code();
	uint8_t model_len = strlen(model);
	device_name		  = (char *)malloc(3 + model_len + 1 + 6 + 1);

	sprintf(device_name, "LT-%s-%02x%02x%02x", model, id[0], id[1], id[2]);
	return device_name;
}

__attribute__((weak)) void lt_reboot() {
	// The Watchdog Way
	lt_wdt_enable(1L);
	while (1) {}
}

__attribute__((weak)) bool lt_reboot_wdt() {
	if (!lt_wdt_enable(1L))
		return false;
	while (1) {}
}

__attribute__((weak)) bool lt_reboot_download_mode() {
	return false;
}

__attribute__((weak)) lt_reboot_reason_t lt_get_reboot_reason() {
	return REBOOT_REASON_UNKNOWN;
}

const char *lt_get_reboot_reason_name(lt_reboot_reason_t reason) {
	if (!reason)
		reason = lt_get_reboot_reason();
	switch (reason) {
		case REBOOT_REASON_POWER:
			return "Power-On";
		case REBOOT_REASON_BROWNOUT:
			return "Brownout";
		case REBOOT_REASON_HARDWARE:
			return "HW Reboot";
		case REBOOT_REASON_SOFTWARE:
			return "SW Reboot";
		case REBOOT_REASON_WATCHDOG:
			return "WDT Reset";
		case REBOOT_REASON_CRASH:
			return "Crash";
		case REBOOT_REASON_SLEEP_GPIO:
			return "Sleep Wakeup (GPIO)";
		case REBOOT_REASON_SLEEP_RTC:
			return "Sleep Wakeup (RTC)";
		case REBOOT_REASON_SLEEP_USB:
			return "Sleep Wakeup (USB)";
		case REBOOT_REASON_DEBUGGER:
			return "Debugger";
		default:
			return "Unknown";
	}
}

__attribute__((weak)) bool lt_set_debug_mode(lt_debug_mode_t mode) {
	return false;
}

__attribute__((weak)) void lt_gpio_recover() {
	lt_set_debug_mode(DEBUG_MODE_OFF);
}
