/* Copyright (c) WGM160P-LibreTiny port 2026-05-26.
 *
 * Phase 1 device API for silabs-efm32gg11. cores/common/base/api/lt_device.c
 * provides weak defaults for lt_reboot, lt_reboot_wdt, lt_reboot_download_mode,
 * lt_get_reboot_reason, lt_set_debug_mode, lt_gpio_recover. Family only needs
 * to provide lt_get_device_mac and (optionally) override the rest.
 *
 * NOTE: The plan called for lt_device_get_chip_id() / lt_device_get_mac()
 * (bool return), but the actual common API surface (lt_device.h) uses
 * lt_get_device_mac(uint8_t *mac) with void return. Adjusted accordingly.
 */

#include "lt_family.h"
#include <libretiny.h>
#include <string.h>

void lt_get_device_mac(uint8_t *mac) {
	// Phase 1: no Wi-Fi MAC available. Derive a stable 6-byte ID from the
	// chip's unique 64-bit DEVINFO number so user code calling lt_get_device_mac()
	// gets *something*. Phase 2 will replace this with the WF200's assigned MAC.
	uint32_t low  = DEVINFO->UNIQUEL;
	uint32_t high = DEVINFO->UNIQUEH;
	mac[0]		  = 0x02; // locally-administered bit set
	mac[1]		  = (uint8_t)(high >> 0);
	mac[2]		  = (uint8_t)(low >> 24);
	mac[3]		  = (uint8_t)(low >> 16);
	mac[4]		  = (uint8_t)(low >> 8);
	mac[5]		  = (uint8_t)(low >> 0);
}

void lt_reboot(void) {
	NVIC_SystemReset();
	while (1) {}
}

lt_reboot_reason_t lt_get_reboot_reason(void) {
	// RMU->RSTCAUSE bits indicate the cause of the most recent reset.
	uint32_t cause = RMU->RSTCAUSE;
	if (cause & RMU_RSTCAUSE_PORST)
		return REBOOT_REASON_POWER;
	// EFM32GG11B has a single AVDD brown-out reset cause (RMU_RSTCAUSE_AVDDBOD).
	// Series-2 parts split it into BODAVDD0/BODAVDD1 — those macros don't exist here.
	if (cause & RMU_RSTCAUSE_AVDDBOD)
		return REBOOT_REASON_BROWNOUT;
	if (cause & RMU_RSTCAUSE_EXTRST)
		return REBOOT_REASON_HARDWARE;
	if (cause & RMU_RSTCAUSE_WDOGRST)
		return REBOOT_REASON_WATCHDOG;
	if (cause & RMU_RSTCAUSE_SYSREQRST)
		return REBOOT_REASON_SOFTWARE;
	if (cause & RMU_RSTCAUSE_LOCKUPRST)
		return REBOOT_REASON_CRASH;
	return REBOOT_REASON_UNKNOWN;
}
