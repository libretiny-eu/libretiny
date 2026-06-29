/* Copyright (c) Etienne Le Cousin 2025-01-19. */

#include <libretiny.h>
#include <sdk_private.h>

bool lt_wdt_enable(uint32_t timeout) {

	wdt_top_value_t wdt_top_value = 0;
	for (uint8_t i = 0; i < 15; i++) {
		if ((0x100UL << i) < (timeout * 32768 / 1000))
			wdt_top_value = i;
	}

	wdt_init_t_def wdt_init;
	memset(&wdt_init, 0, sizeof(wdt_init));
	wdt_init.wdt_rmod = WDT_RMOD_1;
	wdt_init.wdt_rpl  = WDT_RPL_32_PCLK;
	wdt_init.top	  = wdt_top_value;

	hal_wdt_init(WDT_BASE, &wdt_init);

	/* Configure Interrupt */
	NVIC_SetPriority(WDT_IRQn, 4);
	NVIC_EnableIRQ(WDT_IRQn);

	/* Watchdog enable */
	/* Note : Watchdog cannot be disabled */
	hal_wdt_en(WDT_BASE, HAL_ENABLE);

	/* Start feeding */
	hal_wdt_cnt_restart(WDT_BASE);

	return true;
}

void lt_wdt_disable() {
	hal_wdt_deinit();
	hal_wdt_en(WDT_BASE, HAL_DISABLE);
}

void lt_wdt_feed() {
	hal_wdt_cnt_restart(WDT_BASE);
}
