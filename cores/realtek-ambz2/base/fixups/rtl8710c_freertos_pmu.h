/* Copyright (c) Kuba Szczodrzyński 2023-05-23. */

#include <basic_types.h>

// rtl8710c_freertos_pmu.h needs the u32/u16/u8 integer typedefs,
// so it relies on the caller (freertos_pmu.c) to provide these,
// which relies on FreeRTOS.h to include FreeRTOSConfig.h,
// which relies on diag.h included to get printf() functions,
// which just happens to include basic_types.h as well,
// and that's what makes the whole thing even compile
#include_next "rtl8710c_freertos_pmu.h"
