/* Copyright (c) Etienne Le Cousin 2024-02-19. */

#include <libretiny.h>
#include <sdk_private.h>

#include <printf/printf.h>

extern Serial_t *serial_handles[SER_PORT_NUM];

extern uint32_t lt_uart_port;

void putchar_(char c) {
	putchar_p(c, lt_uart_port);
}

void putchar_p(char c, unsigned long port) {
	if (port >= SER_PORT_NUM) {
		return;
	}
	serial_putchar(serial_handles[port], c);
}

WRAP_PRINTF(ln_printf);
WRAP_VPRINTF(ln_vprintf);
