/* Copyright (c) Kuba Szczodrzyński 2022-05-06. */

#pragma once

#ifdef __cplusplus
extern "C" {
#endif // __cplusplus

// va_list is declared by SDK and conflicting
#include <stdarg.h>

// disable typedef in basic_types.h
#define boolean boolean_rtl
// fix conflicts with Arduino's PinMode enum
#define PinMode PinModeRTL

#include <strproc.h> // define string macros first
#undef isdigit		 // then remove them, as they conflict
#undef islower		 // with ctype.h macros
#undef isprint
#undef isspace
#undef isxdigit
#undef strtol
#undef strtoul

#include <ameba_soc.h>
#include <analogin_api.h>
#include <analogout_api.h>
#include <flash_api.h>
#include <gpio_api.h>
#include <gpio_ex_api.h>
#include <gpio_irq_api.h>
#include <gpio_irq_ex_api.h>
#include <i2c_api.h>
#include <main.h>
#include <objects.h>
#include <pwmout_api.h>
#include <rand.h>
#include <rt_lib_rom.h>
#include <rtl8710b.h>
#include <rtl_lib.h>
#include <sys_api.h>
#include <timer_api.h>
#include <us_ticker_api.h>
#include <wait_api.h>
#include <wdt_api.h>

// remove previously defined workarounds
#undef boolean
#undef PinMode

// undefine ROM stdio in favor of printf() library (wrappers)
#undef printf
#undef sprintf
#undef vsprintf
#undef snprintf
#undef vsnprintf
#undef vprintf
#include <stdio.h>

// moved from syscalls.h
#define _close	__rtl_close
#define _fstat	__rtl_fstat
#define _isatty __rtl_isatty
#define _lseek	__rtl_lseek
#define _open	__rtl_open
#define _read	__rtl_read
#define _write	__rtl_write
#define _sbrk	__rtl_sbrk

#define delay_us wait_us

extern void wait_us(int us);
extern int LOGUART_SetBaud(uint32_t BaudRate);	  // from fixups/log_uart.c
extern void DumpForOneBytes(void *addr, int cnt); // cnt max 0x70!
extern void SystemCoreClockUpdate(void);

extern int _sscanf_patch(const char *buf, const char *fmt, ...);

#ifdef __cplusplus
} // extern "C"
#endif
