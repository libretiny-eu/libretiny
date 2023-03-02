/* Copyright (c) Kuba Szczodrzyński 2022-05-06. */

#pragma once

#ifdef __cplusplus
extern "C" {
#endif // __cplusplus

// va_list is declared by SDK and conflicting
#include <stdarg.h>

// disable typedef in basic_types.h
#define boolean boolean_rtl

#include <strproc.h> // define string macros first
#undef isdigit		 // then remove them, as they conflict
#undef islower		 // with ctype.h macros
#undef isprint
#undef isspace
#undef isxdigit
#undef strtol
#undef strtoul

#include <ameba_soc.h>
#include <flash_api.h>
#include <gpio_api.h>
#include <main.h>
#include <rand.h>
#include <rt_lib_rom.h>
#include <rtl_lib.h>
#include <wait_api.h>

// remove previously defined workaround
#undef boolean

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
