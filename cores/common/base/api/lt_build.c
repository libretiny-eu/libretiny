/* Copyright (c) Kuba Szczodrzyński 2026-10-20. */

#include "lt_build.h"

const char *lt_get_version() {
	return LT_VERSION_STR;
}

// this is defined here so that compiler caches can work
#ifndef LT_BUILD_TIMESTAMP
#define LT_BUILD_TIMESTAMP __DATE__ " " __TIME__
#endif

const char *lt_get_build_timestamp() {
	return LT_BUILD_TIMESTAMP;
}

const char *lt_get_board_code() {
	return LT_BOARD_STR;
}
