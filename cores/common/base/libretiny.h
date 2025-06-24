/* Copyright (c) Kuba Szczodrzyński 2022-04-28. */

#pragma once

// C standard libraries
#include <errno.h>
#include <inttypes.h>
#include <math.h>
#include <stdarg.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// LibreTiny version macros
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
#define GCC_VERSION_STR \
	STRINGIFY_MACRO(__GNUC__) "." STRINGIFY_MACRO(__GNUC_MINOR__) "." STRINGIFY_MACRO(__GNUC_PATCHLEVEL__)
#define LT_BANNER_STR                                                                                                \
	"LibreTiny v" LT_VERSION_STR " on " LT_BOARD_STR ", compiled at " __DATE__ " " __TIME__ ", GCC " GCC_VERSION_STR \
	" (-O" STRINGIFY_MACRO(__OPTIMIZE_LEVEL__) ")"

// Functional macros
#define LT_BANNER() LT_LOG(LT_LEVEL_INFO, __FUNCTION__, __LINE__, LT_BANNER_STR)

// Types & macros
#include "lt_config.h" // platform configuration options
#include "lt_types.h"  // types & enums
// Family-specific macros
#include <lt_family.h>
// Board variant (pin definitions)
#include LT_VARIANT_H
// APIs
#include "lt_api.h"		  // main API function definitions
#include "lt_logger.h"	  // UART logger utility
#include "lt_pins.h"	  // additional pin macros
#include "lt_posix_api.h" // POSIX compat functions
// printf silencing methods
#include <printf_port.h>
