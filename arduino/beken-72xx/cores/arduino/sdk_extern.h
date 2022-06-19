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

extern unsigned char __disable_bk_printf;
