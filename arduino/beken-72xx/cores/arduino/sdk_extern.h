/* Copyright (c) Kuba Szczodrzyński 2022-06-18. */

#pragma once

// for printf() etc (they are wrapped anyway)
#include <stdio.h>

// most stuff is here
#include <include.h>
// for os_printf
#include <uart_pub.h>
// for GPIO names
#include <gpio_pub.h>

// conflict with stl_algobase.h
#undef min
#undef max

// include printf() wrapper disable methods
#include <printf_port.h>

// make non-SDK code call the proper printf()
#undef bk_printf
#undef os_printf
#undef warning_prf
#undef fatal_prf
#define bk_printf	printf
#define os_printf	printf
#define warning_prf printf
#define fatal_prf	printf
