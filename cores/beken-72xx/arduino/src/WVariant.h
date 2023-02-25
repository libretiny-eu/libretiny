/* Copyright (c) Kuba Szczodrzyński 2022-06-18. */

#pragma once

#ifdef __cplusplus
extern "C" {
#endif

#include "sdk_extern.h"
#include "sdk_mem.h"

// define an inline delay() which overrides BDK's delay()
static inline __attribute__((always_inline)) void delay(unsigned long ms) {
	delayMilliseconds(ms);
}

// from fixups/arch_main.c
extern unsigned char __bk_rf_is_init;

#ifdef __cplusplus
} // extern "C"
#endif
