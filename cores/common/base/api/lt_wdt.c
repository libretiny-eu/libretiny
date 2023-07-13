/* Copyright (c) Kuba Szczodrzyński 2022-04-29. */

#include "lt_wdt.h"

__attribute__((weak)) bool lt_wdt_enable(uint32_t timeout) {
	return false;
}

__attribute__((weak)) void lt_wdt_disable() {}

__attribute__((weak)) void lt_wdt_feed() {}
