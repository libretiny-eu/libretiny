/* Copyright (c) Kuba Szczodrzyński 2023-02-27. */

#include <libretiny.h>
#include <sdk_private.h>

bool lt_wdt_enable(uint32_t timeout) {
	wdt_ctrl(WCMD_SET_PERIOD, &timeout);
	wdt_ctrl(WCMD_POWER_UP, NULL);
	return true;
}

void lt_wdt_disable() {
	wdt_ctrl(WCMD_POWER_DOWN, NULL);
}

void lt_wdt_feed() {
	wdt_ctrl(WCMD_RELOAD_PERIOD, NULL);
}
