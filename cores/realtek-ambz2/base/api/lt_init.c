/* Copyright (c) Kuba Szczodrzyński 2023-05-22. */

#include <libretiny.h>
#include <sdk_private.h>

extern uint8_t lt_uart_port;

void lt_init_family() {
	// make the SDK less verbose by default
	ConfigDebugErr	= _DBG_MISC_ | _DBG_FAULT_ | _DBG_BOOT_;
	ConfigDebugWarn = 0;
	ConfigDebugInfo = 0;
	lt_uart_port	= LT_UART_DEFAULT_PORT;
}
