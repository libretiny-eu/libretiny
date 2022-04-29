#pragma once

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

#include <Arduino.h>

#include "LibreTuyaConfig.h"

#ifdef __cplusplus
extern "C" {
#endif // __cplusplus

#include "lt_logger.h"

#ifdef __cplusplus
} // extern "C"
#endif

#define LT_BANNER()                                                                                                    \
	LT_LOG(                                                                                                            \
		LT_LEVEL_INFO,                                                                                                 \
		"main.cpp",                                                                                                    \
		__LINE__,                                                                                                      \
		"LibreTuya v" LT_VERSION_STR " on " LT_BOARD_STR ", compiled at " __DATE__ " " __TIME__                        \
	)

extern char *strdup(const char *);
