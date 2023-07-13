/* Copyright (c) Kuba Szczodrzyński 2023-02-27. */

#include <libretiny.h>
#include <sdk_private.h>

void lt_init_family() {
	// set default UART output port
	uart_print_port = LT_UART_DEFAULT_PORT - 1;
}
