/* Copyright (c) WGM160P-LibreTiny port 2026-05-26.
 *
 * Required by cores/common/base/libretiny.h. Declares WRAP_DISABLE_DEF()
 * entries for any wrapped SDK printf names. Phase 1 silabs-efm32gg11 does
 * not wrap a vendor printf (the Gecko SDK uses standard newlib printf),
 * so this file is intentionally minimal — it just pulls in printf_config.h
 * to satisfy the include chain.
 */

#pragma once

#include <printf_config.h>

#ifdef __cplusplus
extern "C" {
#endif

/* No vendor printf to silence on this family — nothing to declare. */

#ifdef __cplusplus
} // extern "C"
#endif
