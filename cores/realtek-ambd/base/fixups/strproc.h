/* Copyright (c) Bl00d-B0b 2026-08-07. */

#pragma once

// The SDK's swlib strproc.h defines isprint/isdigit/... as function-like macros,
// which breaks C++ <locale> (std::isspace(c, loc)). Shadow the header, include
// the real one, then drop the macros for C++.

#include_next <strproc.h>

#ifdef __cplusplus
#undef in_range
#undef isprint
#undef isdigit
#undef isxdigit
#undef isspace
#undef isupper
#undef islower
#undef isalpha
#endif
