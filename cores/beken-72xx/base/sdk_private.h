/* Copyright (c) Kuba Szczodrzyński 2022-06-18. */

#pragma once

#ifdef __cplusplus
extern "C" {
#endif // __cplusplus

// most stuff is here
#include <include.h>
// other includes
#include <flash_pub.h>
#include <gpio_pub.h>
#include <param_config.h>
#include <start_type_pub.h>
#include <sys_ctrl.h>
#include <sys_rtos.h>
#include <uart_pub.h>
#include <wdt_pub.h>
#include <wlan_ui_pub.h>

#include <sdk_extern.h>

// conflict with stl_algobase.h
#undef min
#undef max

// from fixups/arch_main.c
extern unsigned char __bk_rf_is_init;

#ifdef __cplusplus
} // extern "C"
#endif
