/* Copyright (c) Kuba Szczodrzyński 2023-02-27. */

#include "lt_family_api.h"

#include <start_type_pub.h>

ResetReason lt_get_reset_reason() {
	switch (bk_misc_get_start_type()) {
		case RESET_SOURCE_POWERON:
			return RESET_REASON_POWER;
		case RESET_SOURCE_REBOOT:
			return RESET_REASON_SOFTWARE;
		case RESET_SOURCE_WATCHDOG:
			return RESET_REASON_WATCHDOG;
		case RESET_SOURCE_CRASH_XAT0:
		case RESET_SOURCE_CRASH_UNDEFINED:
		case RESET_SOURCE_CRASH_PREFETCH_ABORT:
		case RESET_SOURCE_CRASH_DATA_ABORT:
		case RESET_SOURCE_CRASH_UNUSED:
		case RESET_SOURCE_CRASH_PER_XAT0:
			return RESET_REASON_CRASH;
		case RESET_SOURCE_DEEPPS_GPIO:
		case RESET_SOURCE_DEEPPS_RTC:
		case RESET_SOURCE_DEEPPS_USB:
			return RESET_REASON_SLEEP;
		default:
			return RESET_REASON_UNKNOWN;
	}
}
