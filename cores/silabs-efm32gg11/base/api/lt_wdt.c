/* Copyright (c) WGM160P-LibreTiny port 2026-05-26.
 *
 * Phase 1 watchdog API for silabs-efm32gg11.
 */

#include "em_cmu.h"
#include "em_wdog.h"
#include "lt_family.h"
#include <libretiny.h>

bool lt_wdt_enable(uint32_t timeout_ms) {
	// Map ms -> WDOG period select. The counter is clocked from ULFRCO
	// (~1 kHz), so timeout_ms ~= period count. PERSEL is a power-of-2 enum.
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

	// WDOG is a low-energy peripheral on GG11; its registers live behind the LE
	// bus clock. Gate it on, else WDOGn_Init writes are dropped and the watchdog
	// never arms. (GG11/Series 1 has no cmuClock_WDOG0 enum — the LE branch
	// clock is cmuClock_CORELE.)
	CMU_ClockEnable(cmuClock_CORELE, true);

	WDOG_Init_TypeDef init = WDOG_INIT_DEFAULT;
	init.perSel			   = per;
	// WDOG_INIT_DEFAULT selects LFRCO (32 kHz), which lt_init never enables — so
	// the counter would not tick and the watchdog would never fire. Use ULFRCO
	// (always-on ~1 kHz) instead: no oscillator setup needed and it matches the
	// ms->period mapping above. debugRun stays false so a debugger halt freezes
	// the counter (lets SWD reads happen without spurious resets).
	init.clkSel = wdogClkSelULFRCO;
	init.enable = true;
	WDOGn_Init(DEFAULT_WDOG, &init);
	return true;
}

void lt_wdt_disable(void) {
	WDOGn_Enable(DEFAULT_WDOG, false);
}

void lt_wdt_feed(void) {
	WDOGn_Feed(DEFAULT_WDOG);
}
