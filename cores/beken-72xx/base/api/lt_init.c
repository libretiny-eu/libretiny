/* Copyright (c) Kuba Szczodrzyński 2023-02-27. */

#include <libretiny.h>
#include <sdk_private.h>

void lt_init_family() {
	// set default UART output port
	uart_print_port = LT_UART_DEFAULT_PORT - 1;
	// initialize the UART (needed e.g. after deep sleep)
	if (uart_print_port == 1)
		uart1_init();
	else if (uart_print_port == 2)
		uart2_init();
}
