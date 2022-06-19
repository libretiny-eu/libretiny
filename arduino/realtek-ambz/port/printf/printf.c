/* Copyright (c) Kuba Szczodrzyński 2022-06-19. */

#include <stdarg.h>
#include <stdio.h>
// include LOGUART_PutChar()
#include <rtl8710b.h>
// make sure to call the "real" vprintf instead of rtl_vprintf
#undef vprintf

void putchar_(char c) {
	LOGUART_PutChar(c);
}

// stdio wrappers for Realtek SDK

int __wrap_rtl_printf(const char *format, ...) {
	va_list va;
	va_start(va, format);
	const int ret = vprintf(format, va);
	va_end(va);
	return ret;
}

int __wrap_DiagPrintf(const char *format, ...) {
	va_list va;
	va_start(va, format);
	const int ret = vprintf(format, va);
	va_end(va);
	return ret;
}
