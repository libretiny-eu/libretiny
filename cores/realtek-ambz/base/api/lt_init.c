/* Copyright (c) Kuba Szczodrzyński 2023-02-27. */

#include <libretiny.h>
#include <sdk_private.h>

extern uint32_t GlobalDebugEnable;
extern uint16_t GlobalDebugLevel;
extern uint8_t GlobalPrivateLog;
extern uint8_t lt_uart_port;

void lt_init_family() {
	// make the SDK less verbose by default
	GlobalDebugEnable = 0;
	GlobalPrivateLog  = 0;
	lt_uart_port	  = LT_UART_DEFAULT_PORT;
}
