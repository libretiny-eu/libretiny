/* Copyright (c) Bl00d-B0b 2026-10-28. */

#include <libretiny.h>
#include <sdk_private.h>

// KM4 watchdog: 32.768 kHz clock, 16-bit divider and 12-bit count, so up to ~8190 s.
// WDG_Init() selects reset mode; lt_reboot() uses the same timer for the full-chip reset.
bool lt_wdt_enable(uint32_t timeout) {
	WDG_InitTypeDef wdg;
	u32 count, div_fac;
	WDG_Scalar(timeout, &count, &div_fac);
	wdg.CountProcess  = count;
	wdg.DivFacProcess = div_fac;
	wdg.RstAllPERI	  = 0;
	WDG_Init(&wdg);
	WDG_Cmd(ENABLE);
	return true;
}

void lt_wdt_disable() {
	WDG_Cmd(DISABLE);
}

void lt_wdt_feed() {
	WDG_Refresh();
}
