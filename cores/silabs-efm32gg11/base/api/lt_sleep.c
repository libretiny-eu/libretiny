/* Copyright (c) WGM160P-LibreTiny port 2026-05-26.
 *
 * Phase 1 sleep API for silabs-efm32gg11.
 *
 * NOTE: The plan referred to a single lt_sleep(uint32_t ms) primitive, but the
 * actual common API surface (cores/common/base/api/lt_sleep.h) exposes the
 * lt_deep_sleep_* family from Peter Sarkozi's 2023-06-17 rework. Phase 1
 * stubs these — full deep-sleep wakeup paths arrive with RTCC/GPIO IRQ
 * support in a later phase.
 */

#include "em_emu.h"
#include "lt_family.h"
#include <libretiny.h>

void lt_deep_sleep_config_gpio(uint32_t gpio_index_map, bool on_high) {
	(void)gpio_index_map;
	(void)on_high;
	// Phase 1 stub. GPIO EM4 wake-up unsupported.
}

void lt_deep_sleep_unset_gpio(uint32_t gpio_index_map) {
	(void)gpio_index_map;
}

void lt_deep_sleep_keep_floating_gpio(uint32_t gpio_index_map, bool enabled) {
	(void)gpio_index_map;
	(void)enabled;
}

void lt_deep_sleep_config_timer(uint32_t sleep_duration) {
	(void)sleep_duration;
	// Phase 1 stub. Timed deep-sleep requires RTCC wakeup wiring.
}

void lt_deep_sleep_enter(void) {
	// Phase 1: enter EM1 (sleep) — wakes on any IRQ.
	// EM2/EM3/EM4 require LF clock setup we don't do here yet.
	EMU_EnterEM1();
}
