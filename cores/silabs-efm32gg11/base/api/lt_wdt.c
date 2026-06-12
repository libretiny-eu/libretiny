/* Copyright (c) WGM160P-LibreTiny port 2026-05-26.
 *
 * Phase 1 watchdog API for silabs-efm32gg11.
 */

#include "em_wdog.h"
#include "lt_family.h"
#include <libretiny.h>

bool lt_wdt_enable(uint32_t timeout_ms) {
	// Map ms -> WDOG period select. WDOG ticks at LFRCO (~1 kHz),
	// so timeout_ms ~= period count. PERSEL is a power-of-2 enum.
	WDOG_PeriodSel_TypeDef per;
	if (timeout_ms <= 9)
		per = wdogPeriod_9;
	else if (timeout_ms <= 17)
		per = wdogPeriod_17;
	else if (timeout_ms <= 33)
		per = wdogPeriod_33;
	else if (timeout_ms <= 65)
		per = wdogPeriod_65;
	else if (timeout_ms <= 129)
		per = wdogPeriod_129;
	else if (timeout_ms <= 257)
		per = wdogPeriod_257;
	else if (timeout_ms <= 513)
		per = wdogPeriod_513;
	else if (timeout_ms <= 1025)
		per = wdogPeriod_1k;
	else if (timeout_ms <= 2049)
		per = wdogPeriod_2k;
	else if (timeout_ms <= 4097)
		per = wdogPeriod_4k;
	else if (timeout_ms <= 8193)
		per = wdogPeriod_8k;
	else
		per = wdogPeriod_16k;

	WDOG_Init_TypeDef init = WDOG_INIT_DEFAULT;
	init.perSel			   = per;
	init.enable			   = true;
	WDOGn_Init(DEFAULT_WDOG, &init);
	return true;
}

void lt_wdt_disable(void) {
	WDOGn_Enable(DEFAULT_WDOG, false);
}

void lt_wdt_feed(void) {
	WDOGn_Feed(DEFAULT_WDOG);
}
