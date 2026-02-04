/* Copyright (c) Kuba Szczodrzyński 2022-06-18. */

#pragma once

#ifdef __cplusplus
extern "C" {
#endif // __cplusplus

// most stuff is here - this has to be before other includes!
#include <include.h>
// other includes
#include <arm_arch.h>
#include <bk_timer.h>
#include <bk_timer_pub.h>
#include <flash_pub.h>
#include <gpio_pub.h>
#include <manual_ps_pub.h>
#include <param_config.h>
#include <pwm_pub.h>
#include <rtos_pub.h>
#include <saradc_pub.h>
#include <start_type_pub.h>
#include <sys_ctrl.h>
#include <sys_rtos.h>
#include <uart_pub.h>
#include <wdt_pub.h>

// BDK 3.0.76+ has a conflicting structure
// (but it's not actually used in source code)
#define wifi_event_sta_disconnected_t bk_wifi_event_sta_disconnected_t
#include <wlan_ui_pub.h>
#undef wifi_event_sta_disconnected_t

#include <sdk_extern.h>

// conflict with stl_algobase.h
#undef min
#undef max

// from fixups/arch_main.c
extern unsigned char __bk_rf_is_init;

#ifdef __cplusplus
} // extern "C"
#endif
