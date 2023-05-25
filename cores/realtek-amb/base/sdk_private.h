/* Copyright (c) Kuba Szczodrzyński 2022-05-06. */

#pragma once

#ifdef __cplusplus
extern "C" {
#endif // __cplusplus

// note: this is *not* replacing any stdlib functions :)
#include <platform_stdlib.h>

// fix conflicts with Arduino's PinMode enum
#define PinMode PinModeRTL
// remove log_printf() if included before sdk_private.h
#undef log_printf

// CMSIS & Realtek APIs
#if LT_RTL8710B
#include <ameba_soc.h>
#include <rand.h>
#include <rtl8710b.h>
#endif
#if LT_RTL8720C
#include <hal.h>
#include <hal_sys_ctrl.h>
#include <rtl8710c.h>
#endif

#include <cmsis_os.h>
#undef malloc
#undef free
#undef calloc

// mbed APIs
#include <gpio_api.h>
#undef MBED_GPIO_API_H // ..no comment
#include <gpio_ex_api.h>

#include <analogin_api.h>
#include <analogout_api.h>
#include <efuse_api.h>
#if LT_RTL8720C
#include <efuse_logical_api.h>
#endif
#include <flash_api.h>
#include <gpio_irq_api.h>
#include <gpio_irq_ex_api.h>
#include <i2c_api.h>
#include <main.h>
#include <objects.h>
#include <pwmout_api.h>
#include <serial_api.h>
#include <sys_api.h>
#include <timer_api.h>
#include <us_ticker_api.h>
#include <wait_api.h>
#include <wdt_api.h>

// other SDK APIs
#if __has_include(<sdk_extern.h>)
#include <sdk_extern.h>
#endif

// remove previously defined workarounds
#undef PinMode

// undefine ROM stdio in favor of printf() library (wrappers)
#undef printf
#undef sprintf
#undef vsprintf
#undef snprintf
#undef vsnprintf
#undef vprintf
#include <stdio.h>
// conflict with lt_logger.h
#undef log_printf

// moved from syscalls.h
#define _close	__rtl_close
#define _fstat	__rtl_fstat
#define _isatty __rtl_isatty
#define _lseek	__rtl_lseek
#define _open	__rtl_open
#define _read	__rtl_read
#define _write	__rtl_write
#define _sbrk	__rtl_sbrk

extern flash_t lt_flash_obj;

#ifdef __cplusplus
} // extern "C"
#endif
