/* Copyright (c) WGM160P-LibreTiny port 2026-06-11.
 *
 * lwIP arch/cc.h for silabs-efm32gg11 (GCC/newlib).
 *
 * This deliberately SHADOWS the GSDK lwip-contrib freertos port's
 * include/arch/cc.h: the family base/config dir is prepended to the include
 * path (ConfigureFamily), while the contrib include dir is appended by the
 * lwIP library block, so this file wins for `#include "arch/cc.h"`.
 * (arch/sys_arch.h has no shadow here and still resolves to the contrib port.)
 *
 * Why shadow: the contrib cc.h force-defines LWIP_PROVIDE_ERRNO (empty,
 * unconditional). That makes lwIP define its own Linux-numbered errno
 * constants plus `extern int errno;` — both clash with newlib, whose errno is
 * a macro (*__errno()) and whose E* values differ (EINPROGRESS 119 vs 115,
 * ETIMEDOUT 116 vs 110, ...). Common LT code (LwIPClient) compares errno
 * against newlib's <errno.h> values, so lwIP must use newlib errno too:
 * lwipopts.h sets LWIP_ERRNO_STDINCLUDE 1 and this file stays out of it.
 */
#ifndef LT_SILABS_LWIP_ARCH_CC_H
#define LT_SILABS_LWIP_ARCH_CC_H

#include <stdio.h>	// LWIP_PLATFORM_ASSERT / LWIP_PLATFORM_DIAG
#include <stdlib.h> // LWIP_RAND (defined in lwipopts.h)

/* Cortex-M4 is little-endian; LITTLE_ENDIAN is provided by lwip/arch.h. */
#ifndef BYTE_ORDER
#define BYTE_ORDER LITTLE_ENDIAN
#endif

/* GCC packing for protocol structs. */
#define PACK_STRUCT_BEGIN
#define PACK_STRUCT_STRUCT __attribute__((__packed__))
#define PACK_STRUCT_END
#define PACK_STRUCT_FIELD(x) x

#define LWIP_PLATFORM_ASSERT(x)                                                        \
	do {                                                                               \
		printf("lwIP assert \"%s\" failed at line %d in %s\n", x, __LINE__, __FILE__); \
		while (1) {}                                                                   \
	} while (0)

#endif /* LT_SILABS_LWIP_ARCH_CC_H */
