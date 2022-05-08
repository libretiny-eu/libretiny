/* Copyright (c) Kuba Szczodrzyński 2022-05-06. */

#pragma once

#include <stdarg.h>
#include <stddef.h>
#include <stdint.h>

#define boolean boolean_rtl
#include <ameba_soc.h>
#include <gpio_api.h>
#include <main.h>
#include <rand.h>
#include <rt_lib_rom.h>
#include <rtl_lib.h>
#include <wait_api.h>
#undef boolean

// stdio.h
#define printf	rtl_printf
#define sprintf rtl_sprintf

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
