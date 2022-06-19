/* Copyright (c) Kuba Szczodrzyński 2022-06-19. */

#include <stdarg.h>
#include <stdio.h>

// include bk_send_byte()
#include <uart_pub.h>

void putchar_(char c) {
	uart_write_byte(uart_print_port, c);
}
