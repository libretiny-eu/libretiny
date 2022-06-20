/* Copyright (c) Kuba Szczodrzyński 2022-06-19. */

#include <printf_config.h>

#include <printf/printf.h>
#include <stdarg.h>
#include <stdint.h>
#include <stdio.h>

extern void bk_send_byte(uint8_t uport, uint8_t data);
extern int uart_print_port;

void putchar_(char c) {
	bk_send_byte(uart_print_port, c);
}

WRAP_PRINTF(bk_printf);
