/* Copyright (c) Kuba Szczodrzyński 2023-05-22. */

#include <libretiny.h>
#include <sdk_private.h>

extern uint8_t lt_uart_port;

void lt_init_family() {
	// make the SDK less verbose by default
	ConfigDebugErr	= 0;
	ConfigDebugWarn = 0;
	ConfigDebugInfo = 0;
	lt_uart_port	= LT_UART_DEFAULT_PORT;
}

lt_reboot_reason_t lt_get_reboot_reason() {
	// TODO
	return REBOOT_REASON_UNKNOWN;
}
