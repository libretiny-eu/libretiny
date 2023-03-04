/* Copyright (c) Kuba Szczodrzyński 2022-06-19. */

#include <libretuya.h>

#include <printf/printf.h>

extern void bk_send_byte(uint8_t uport, uint8_t data);
extern int uart_print_port;

void putchar_(char c) {
	bk_send_byte(uart_print_port, c);
}

void putchar_p(char c, unsigned long port) {
	bk_send_byte((port & 0xFF) - 1, c);
}

WRAP_PRINTF(bk_printf);
