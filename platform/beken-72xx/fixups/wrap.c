/* Copyright (c) Kuba Szczodrzyński 2022-06-14. */

#include <stdarg.h>
#include <stddef.h>

extern int __wrap_vsnprintf(char *str, size_t size, const char *format, va_list args);
extern void bk_send_string(unsigned char uport, const char *string);
extern int uart_print_port;

unsigned char __disable_bk_printf = 0;

void __wrap_bk_printf(const char *fmt, ...) {
	if (__disable_bk_printf)
		return;
	va_list ap;
	char string[128];
	va_start(ap, fmt);
	__wrap_vsnprintf(string, sizeof(string) - 1, fmt, ap);
	string[127] = 0;
	bk_send_string(uart_print_port, string);
	va_end(ap);
}
