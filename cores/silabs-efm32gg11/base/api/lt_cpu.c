/* Copyright (c) WGM160P-LibreTiny port 2026-05-26.
 *
 * Phase 1 CPU API for silabs-efm32gg11.
 *
 * cores/common/base/api/lt_cpu.c provides weak defaults for lt_cpu_get_family,
 * lt_cpu_get_model (via MCU macro), lt_cpu_get_unique_id, lt_cpu_get_mac_id,
 * lt_cpu_get_freq (returning configCPU_CLOCK_HZ when FreeRTOS is on),
 * lt_cpu_get_cycle_count, etc. The family only needs to override what the
 * MCU implements better (e.g. true HF clock freq instead of the FreeRTOS
 * configured rate, and a meaningful unique_id from DEVINFO).
 *
 * NOTE: Plan called for lt_cpu_reset() — that's not in the common API.
 * NVIC_SystemReset() lives in lt_device.c's lt_reboot() override instead.
 */

#include "em_cmu.h"
#include "lt_family.h"
#include <libretiny.h>

const char *lt_cpu_get_core_type(void) {
	return "ARM Cortex-M4F (ARMv7E-M)";
}

uint32_t lt_cpu_get_freq(void) {
	return CMU_ClockFreqGet(cmuClock_HF);
}

uint32_t lt_cpu_get_unique_id(void) {
	// Lower 32 bits of the unique 64-bit chip ID from DEVINFO,
	// masked to 24 bits per the common API contract.
	return DEVINFO->UNIQUEL & 0x00FFFFFFu;
}
