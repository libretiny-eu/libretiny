/* Copyright (c) Etienne Le Cousin 2024-02-10. */

#pragma once

#ifdef __cplusplus
// Fix to not include SDK reg_xx.h files in C++
// These files declare inline functions with unallowed volatile assignments in C++
#define __REG_CACHE_H__
#define __REG_GPIO_H__
#define __REG_I2C_H__
#define __REG_LN_UART_H__
#define __REG_QSPI_H__
#define __REG_WDT_H__
#define __REG_SYSC_AWO_H__
#define __REG_SYSC_CMP_H__
extern "C" {
#endif // __cplusplus

// PinNames
#include "PinNames.h"

// undefine ROM stdio in favor of printf() library (wrappers)
#undef printf
#undef sprintf
#undef vsprintf
#undef snprintf
#undef vsnprintf
#undef vprintf
#include <stdio.h>

// Conflicting types
#define WIFI_MODE_STATION			LN_WIFI_MODE_STATION
#define WIFI_MODE_AP				LN_WIFI_MODE_AP
#define WIFI_MODE_AP_STATION		LN_WIFI_MODE_AP_STATION
#define WIFI_MODE_MAX				LN_WIFI_MODE_MAX
#define WIFI_AUTH_OPEN				LN_WIFI_AUTH_OPEN
#define WIFI_AUTH_WEP				LN_WIFI_AUTH_WEP
#define WIFI_AUTH_WPA_PSK			LN_WIFI_AUTH_WPA_PSK
#define WIFI_AUTH_WPA2_PSK			LN_WIFI_AUTH_WPA2_PSK
#define WIFI_AUTH_WPA_WPA2_PSK		LN_WIFI_AUTH_WPA_WPA2_PSK
#define WIFI_AUTH_WPA2_ENTERPRISE	LN_WIFI_AUTH_WPA2_ENTERPRISE
#define WIFI_AUTH_WPA3_SAE			LN_WIFI_AUTH_WPA3_SAE
#define WIFI_AUTH_WPA2_PSK_WPA3_SAE LN_WIFI_AUTH_WPA2_PSK_WPA3_SAE
#define WIFI_AUTH_MAX				LN_WIFI_AUTH_MAX
#define wifi_mode_t					ln_wifi_mode_t
#define wifi_auth_mode_t			ln_wifi_auth_mode_t

// SDK
#include "ln_kv_api.h"
#include "ln_nvds.h"
#include "utils/debug/CmBacktrace/cm_backtrace.h"
#include "utils/ln_psk_calc.h"
#include "utils/power_mgmt/ln_pm.h"
#include "utils/reboot_trace/reboot_trace.h"
#include "utils/runtime/runtime.h"
#include "utils/system_parameter.h"
#include <hal/hal_adc.h>
#include <hal/hal_gpio.h>
#include <hal/hal_i2c.h>
#include <hal/hal_interrupt.h>
#include <hal/hal_wdt.h>
#include <ln_wifi.h>
#include <osal/osal.h>
#include <serial/ln_serial.h>
#include <wifi_manager.h>

#ifndef LN882H_SDK
#undef WIFI_MODE_STATION
#undef WIFI_MODE_AP
#undef WIFI_MODE_AP_STATION
#undef WIFI_MODE_MAX
#undef WIFI_AUTH_OPEN
#undef WIFI_AUTH_WEP
#undef WIFI_AUTH_WPA_PSK
#undef WIFI_AUTH_WPA2_PSK
#undef WIFI_AUTH_WPA_WPA2_PSK
#undef WIFI_AUTH_WPA2_ENTERPRISE
#undef WIFI_AUTH_WPA3_SAE
#undef WIFI_AUTH_WPA2_PSK_WPA3_SAE
#undef WIFI_AUTH_MAX
#undef wifi_mode_t
#undef wifi_auth_mode_t
#endif

#ifdef __cplusplus
} // extern "C"
#endif
