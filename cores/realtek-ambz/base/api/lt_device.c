/* Copyright (c) Kuba Szczodrzyński 2023-02-27. */

#include <libretiny.h>
#include <sdk_private.h>

void lt_get_device_mac(uint8_t *mac) {
	uint8_t *efuse = (uint8_t *)malloc(512);
	EFUSE_LogicalMap_Read(efuse);
	memcpy(mac, efuse + 0x11A, 6);
	free(efuse);
}

bool lt_reboot_download_mode() {
	// mww 0x40000138 0x8
	HAL_WRITE32(SYSTEM_CTRL_BASE, REG_SYS_NORESET_FF, 0x08);
	// reboot it the ugly way
	sys_reset();
	while (1) {}
	return true;
}

bool lt_set_debug_mode(lt_debug_mode_t mode) {
	uint32_t *swd;
	switch (mode) {
		case DEBUG_MODE_OFF:
			sys_jtag_off();
			Pinmux_Config(PA_14, PINMUX_FUNCTION_GPIO);
			Pinmux_Config(PA_15, PINMUX_FUNCTION_GPIO);
			return true;
		case DEBUG_MODE_SWD:
			Pinmux_Config(PA_14, PINMUX_FUNCTION_SWD);
			Pinmux_Config(PA_15, PINMUX_FUNCTION_SWD);
			uint32_t *swd = (uint32_t *)0x400000A4;
			*swd |= 0x1000;
			return true;
		default:
			return false;
	}
}
