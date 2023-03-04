/* Copyright (c) Kuba Szczodrzyński 2022-06-18. */

#pragma once

// most stuff is here
#include <include.h>
// other includes
#include <flash_pub.h>
#include <gpio_pub.h>
#include <uart_pub.h>

// conflict with stl_algobase.h
#undef min
#undef max

// make non-SDK code call the proper printf()
#undef bk_printf
#undef os_printf
#undef warning_prf
#undef fatal_prf
#define bk_printf	printf
#define os_printf	printf
#define warning_prf printf
#define fatal_prf	printf
