/* Copyright (c) Kuba Szczodrzyński 2023-05-27. */

#pragma once

// lwipopts.h defines `unsigned int sys_now()`, while lwip/sys.h defines `u32_t sys_now()`
// since u32_t is unsigned long, these are incompatible
#define sys_now sys_now_dummy
#include_next "lwipopts.h"
#undef sys_now

#ifdef __cplusplus
extern "C" {
#endif
extern unsigned long sys_now(void);
#ifdef __cplusplus
}
#endif
