/* Copyright (c) Kuba Szczodrzyński 2022-06-20. */

#pragma once

#include_next "uart_pub.h"

#ifdef LIBRETUYA_ARDUINO
// make uart.c call __wrap_bk_printf() instead of bk_printf()
// standard wrapping does not work in this case, as bk_printf()
// is implemented in the same translation unit
extern void __wrap_bk_printf(const char *fmt, ...);
#undef bk_printf
#undef os_printf
#undef as_printf
// not defining bk_printf() again, as this would just change the impl name
#define os_printf __wrap_bk_printf
#define as_printf (__wrap_bk_printf("%s:%d\r\n", __FUNCTION__, __LINE__))
#endif
