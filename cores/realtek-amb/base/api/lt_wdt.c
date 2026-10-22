/* Copyright (c) Kuba Szczodrzyński 2023-05-23. */

#include <libretiny.h>
#include <sdk_private.h>

bool lt_wdt_enable(uint32_t timeout) {
	watchdog_init(timeout);
	watchdog_start();
	return true;
}

void lt_wdt_disable() {
	watchdog_stop();
}

void lt_wdt_feed() {
	watchdog_refresh();
}
