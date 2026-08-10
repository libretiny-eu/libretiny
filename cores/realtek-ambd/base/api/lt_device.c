/* Copyright (c) Bl00d-B0b 2026-08-06. */

#include <libretiny.h>
#include <sdk_private.h>

void lt_reboot(void) {
	// Full-chip reset like the vendor ota_platform_reset(): flag the KM4 reset in
	// the backup register and let the watchdog reset the whole SoC. sys_reset()
	// resets KM4 only and leaves KM0 with stale WiFi/PHY state.
	WDG_InitTypeDef wdg;
	u32 count, div_fac;
	// Deauthenticate first: after a silent watchdog reset the AP rejects the quick
	// re-join of a seemingly active client (assoc status 30).
	extern int wifi_disconnect(void);
	wifi_disconnect();
	DelayMs(200);
	// Mark the software reset in the user area of retention RAM: it survives the
	// watchdog reset, is random after a cold power-on (hence the magic) and is
	// plain SRAM, unlike the BKUP registers.
	*(volatile uint32_t *)&RRAM_BASE->RRAM_USER_RSVD[116] = 0x4C545253; // "LTRS"
	BKUP_Set(BKUP_REG0, BIT_KM4SYS_RESET_HAPPEN);
	WDG_Scalar(50, &count, &div_fac);
	wdg.CountProcess  = count;
	wdg.DivFacProcess = div_fac;
	WDG_Init(&wdg);
	WDG_Cmd(ENABLE);
	while (1) {}
}

lt_reboot_reason_t lt_get_reboot_reason(void) {
	// BOOT_Reason() (ROM) returns the bootloader's latched copy of the BKUP_REG0
	// reason bits, cleared from the live register before app start. lt_reboot()
	// sets KM4SYS and then fires the KM4 watchdog, so the software bit must win.
	u32 boot = BOOT_Reason();
	if (boot & BIT_BOOT_DSLP_RESET_HAPPEN)
		return REBOOT_REASON_SLEEP;
	if (boot & (BIT_BOOT_KM4SYS_RESET_HAPPEN | BIT_BOOT_SYS_RESET_HAPPEN))
		return REBOOT_REASON_SOFTWARE;
	if (boot & (BIT_BOOT_KM4WDG_RESET_HAPPEN | BIT_BOOT_WDG_RESET_HAPPEN))
		return REBOOT_REASON_WATCHDOG;
	if (boot & BIT_BOOT_BOD_RESET_HAPPEN)
		return REBOOT_REASON_BROWNOUT;
	return REBOOT_REASON_POWER;
}

// Factory MAC addresses live in the logical eFuse map (OTP), not flash:
// WiFi at 0x11A (wlan driver "read_mac"), BT at 0x190 (hci_board.c).
#define AMBD_EFUSE_WIFI_MAC 0x11A

void lt_get_device_mac(uint8_t *mac) {
	extern int efuse_logical_read(u16 addr, u16 size, u8 * pbuf);
	uint8_t buf[6];
	if (efuse_logical_read(AMBD_EFUSE_WIFI_MAC, 6, buf) >= 0) {
		// all-FF means the map slot was never programmed
		for (int i = 0; i < 6; i++) {
			if (buf[i] != 0xFF) {
				memcpy(mac, buf, 6);
				return;
			}
		}
	}
	// fall back to the WiFi driver's own view once it is up
	memset(mac, 0, 6);
}
