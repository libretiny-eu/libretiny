/* Copyright (c) Kuba Szczodrzyński 2022-04-28. */

#pragma once

// C standard libraries
#include <inttypes.h>
#include <stdarg.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// C++ standard libraries
#ifdef __cplusplus
#include <algorithm>
#include <cmath>
using ::round;
using std::abs;
using std::isinf;
using std::isnan;
using std::max;
using std::min;
#endif

// LibreTuya version macros
#ifndef LT_VERSION
#define LT_VERSION 1.0.0
#endif
#ifndef LT_BOARD
#define LT_BOARD unknown
#endif
#define STRINGIFY(x)	   #x
#define STRINGIFY_MACRO(x) STRINGIFY(x)
#define LT_VERSION_STR	   STRINGIFY_MACRO(LT_VERSION)
#define LT_BOARD_STR	   STRINGIFY_MACRO(LT_BOARD)

// Includes
#include "LibreTuyaClass.h"	 // global LT class
#include "LibreTuyaCompat.h" // compatibility methods
#include "LibreTuyaConfig.h" // configuration macros
#include "LibreTuyaCustom.h" // family-defined methods (Wiring custom)
#include <Arduino.h>

// C includes
#ifdef __cplusplus
extern "C" {
#endif // __cplusplus

#include "lt_logger.h"
#include "lt_posix_api.h"

#ifdef __cplusplus
} // extern "C"
#endif

// Functional macros
#define LT_BANNER()                                                                                                    \
	LT_LOG(                                                                                                            \
		LT_LEVEL_INFO,                                                                                                 \
		__FUNCTION__,                                                                                                  \
		__LINE__,                                                                                                      \
		"LibreTuya v" LT_VERSION_STR " on " LT_BOARD_STR ", compiled at " __DATE__ " " __TIME__                        \
	)

#ifdef __cplusplus
String ipToString(const IPAddress &ip);

extern "C" {
void lt_rand_bytes(uint8_t *buf, size_t len);
void hexdump(const uint8_t *buf, size_t len, uint32_t offset = 0, uint8_t width = 16);
}

#else

void lt_rand_bytes(uint8_t *buf, size_t len);
void hexdump(const uint8_t *buf, size_t len, uint32_t offset, uint8_t width);

#endif
