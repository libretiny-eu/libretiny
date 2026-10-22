/* Copyright (c) Kuba Szczodrzyński 2023-03-14. */

#pragma once

// platform_stdlib.h in amb1_sdk includes some stdlib headers,
// as well as Realtek's stdlib replacement headers. It also defines
// some macros to map stdlib functions to SDK functions, so it's
// generally just not needed at all.

// This is also the only file that publicly includes strproc.h and memproc.h,
// so this fixup resolves all these issues.

#include <stdarg.h> /* va_list */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "basic_types.h" // fixup: replaces typedef boolean for Arduino compatibility
#if __has_include(<memproc.h>)
#include <memproc.h> // fixup: redirects to stdlib
#endif
#if __has_include(<strproc.h>)
#include "strproc.h" // fixup: redirects to stdlib
#endif

#include "diag.h"
