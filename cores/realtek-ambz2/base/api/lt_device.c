/* Copyright (c) Kuba Szczodrzyński 2023-05-22. */

#include <libretiny.h>
#include <sdk_private.h>

void lt_get_device_mac(uint8_t *mac) {
	efuse_logical_read(0x11A, 6, mac);
}

void lt_reboot() {
	sys_cpu_reset();
	while (1) {}
}

bool lt_reboot_download_mode() {
	sys_uart_download_mode();
	while (1) {}
	return true;
}

lt_reboot_reason_t lt_get_reboot_reason() {
	hal_reset_reason_t reason = -1;
	rtl8710c_reset_reason_get(&reason);
	switch (reason) {
		case HAL_RESET_REASON_POWER_ON:
			return REBOOT_REASON_POWER;
		case HAL_RESET_REASON_SOFTWARE:
			return REBOOT_REASON_SOFTWARE;
		case HAL_RESET_REASON_WATCHDOG:
			return REBOOT_REASON_WATCHDOG;
		case HAL_RESET_REASON_JTAG:
			return REBOOT_REASON_DEBUGGER;
		default:
			return REBOOT_REASON_UNKNOWN;
	}
}

bool lt_set_debug_mode(lt_debug_mode_t mode) {
	switch (mode) {
		case DEBUG_MODE_OFF:
			if (hal_misc_jtag_pin_ctrl(0) != HAL_OK)
				return false;
			if (hal_misc_swd_pin_ctrl(0) != HAL_OK)
				return false;
			return true;
		case DEBUG_MODE_JTAG:
			if (hal_misc_swd_pin_ctrl(0) != HAL_OK)
				return false;
			if (hal_misc_jtag_pin_ctrl(1) != HAL_OK)
				return false;
			return true;
		case DEBUG_MODE_SWD:
			if (hal_misc_jtag_pin_ctrl(0) != HAL_OK)
				return false;
			if (hal_misc_swd_pin_ctrl(1) != HAL_OK)
				return false;
			return true;
		default:
			return false;
	}
}
