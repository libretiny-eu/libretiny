/* Copyright (c) Kuba Szczodrzyński 2022-06-18. */

#pragma once

#ifdef __cplusplus
extern "C" {
#endif

#include "sdk_extern.h"
#include "sdk_mem.h"

// allow BDK use its own delay() and let Arduino code use delayMilliseconds()
void delayMilliseconds(unsigned long);
#define delay delayMilliseconds

// from fixups/arch_main.c
extern unsigned char __bk_rf_is_init;

#ifdef __cplusplus
} // extern "C"
#endif
