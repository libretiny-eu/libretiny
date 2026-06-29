/* Copyright (c) Etienne Le Cousin 2024-02-19. */

#include <libretiny.h>
#include <sdk_private.h>

#include <printf/printf.h>

extern Serial_t *serial_handles[SER_PORT_NUM];

uint8_t uart_print_port = LT_UART_DEFAULT_LOGGER;

void putchar_(char c) {
	putchar_p(c, uart_print_port);
}

void putchar_p(char c, unsigned long port) {
	serial_putchar(serial_handles[port], c);
}

WRAP_PRINTF(ln_printf);
WRAP_VPRINTF(ln_vprintf);
