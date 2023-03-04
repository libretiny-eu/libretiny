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

// Functional macros
#define LT_BANNER()                                                                                                    \
	LT_LOG(                                                                                                            \
		LT_LEVEL_INFO,                                                                                                 \
		__FUNCTION__,                                                                                                  \
		__LINE__,                                                                                                      \
		"LibreTuya v" LT_VERSION_STR " on " LT_BOARD_STR ", compiled at " __DATE__ " " __TIME__                        \
	)

// Types & macros
#include "lt_chip.h"   // ChipType enum
#include "lt_config.h" // configuration macros
#include "lt_types.h"  // other types & enums
// Family-specific macros
#include <lt_family.h>
// Board variant (pin definitions)
#include <variant.h>
// APIs
#include "lt_common_api.h" // common APIs
#include "lt_family_api.h" // family-specific APIs
#include "lt_logger.h"	   // UART logger utility
#include "lt_posix_api.h"  // POSIX compat functions
// printf silencing methods
#include <printf_port.h>
