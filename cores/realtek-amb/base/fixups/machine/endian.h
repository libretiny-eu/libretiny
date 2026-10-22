/* Copyright (c) Kuba Szczodrzyński 2023-03-12. */

#include_next <machine/endian.h>

#pragma once

// GCC versions newer than 5.x.x specify the LITTLE_ENDIAN macro
// as an alias to _LITTLE_ENDIAN (which in turn holds the actual numeric value).
// Realtek's rtl8711b_crypto.h redefines _LITTLE_ENDIAN as a macro without
// any value, which makes comparisons like '#if BYTE_ORDER == LITTLE_ENDIAN' impossible.

#if __GNUC__ > 5
#undef _LITTLE_ENDIAN
#undef _BIG_ENDIAN
#undef LITTLE_ENDIAN
#undef BIG_ENDIAN
#undef BYTE_ORDER
#define LITTLE_ENDIAN 1234
#define BIG_ENDIAN	  4321
#define BYTE_ORDER	  LITTLE_ENDIAN
#endif
