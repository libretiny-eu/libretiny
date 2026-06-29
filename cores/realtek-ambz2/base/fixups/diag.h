/* Copyright (c) Kuba Szczodrzyński 2023-05-23. */

// remove component/soc/realtek/8710c/app/rtl_printf/include/rt_printf.h
// replace with #defines below
#ifndef _RT_PRINTF__H
#define _RT_PRINTF__H
#endif

#undef log_printf
#include_next "diag.h"
#undef log_printf

#define rt_printf(...)	   __wrap_rt_printf(__VA_ARGS__)
#define rt_printfl(...)	   __wrap_rt_printf(__VA_ARGS__)
#define rt_sprintf(...)	   __wrap_rt_sprintf(__VA_ARGS__)
#define rt_sprintfl(...)   __wrap_rt_sprintf(__VA_ARGS__)
#define rt_snprintf(...)   __wrap_rt_snprintf(__VA_ARGS__)
#define rt_snprintfl(...)  __wrap_rt_snprintf(__VA_ARGS__)
#define rt_log_printf(...) __wrap_rt_log_printf(__VA_ARGS__)
