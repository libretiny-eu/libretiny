/* Copyright (c) Kuba Szczodrzyński 2022-06-20. */

#pragma once

#include <printf_config.h>

#ifdef __cplusplus
extern "C" {
#endif // __cplusplus

WRAP_DISABLE_DEF(rtl_printf);
WRAP_DISABLE_DEF(DiagPrintf);
WRAP_DISABLE_DEF(prvDiagPrintf);
WRAP_DISABLE_DEF(LOG_PRINTF);

#ifdef __cplusplus
} // extern "C"
#endif
