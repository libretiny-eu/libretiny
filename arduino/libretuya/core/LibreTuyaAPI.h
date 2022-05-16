/* Copyright (c) Kuba Szczodrzyński 2022-04-28. */

#pragma once

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
#include "LibreTuyaConfig.h"
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

// ArduinoCore-API doesn't define these anymore
#define FPSTR(pstr_pointer) (reinterpret_cast<const __FlashStringHelper *>(pstr_pointer))
#define PGM_VOID_P			const void *

// C functions
void lt_rand_bytes(uint8_t *buf, size_t len);

// C++ only functions
#ifdef __cplusplus
String ipToString(const IPAddress &ip);
#endif

// Main class
#ifdef __cplusplus
/**
 * @brief Main LibreTuya API class.
 *
 * This class contains all functions common amongst all platforms.
 * Implementations of these methods may vary between platforms.
 *
 * The class is accessible using the `LT` global object (defined by the platform).
 */
class LibreTuya {

	/* Common methods*/

  public:
	/* Platform-defined methods */

  public:
};

extern LibreTuya LT;
#endif
