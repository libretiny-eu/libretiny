/* Copyright (c) Bl00d-B0b 2026-08-07. */

#include <libretiny.h>
#include <sdk_private.h>

extern uint8_t lt_uart_port;

#if LT_AMBD_FAULT_DUMP
extern void lt_fault_dump_install(void);
#endif

void lt_init_family() {
#if LT_AMBD_FAULT_DUMP
	lt_fault_dump_install();
#endif
	// route LibreTiny's printf port to the family default (LOGUART on AmebaD)
	lt_uart_port = LT_UART_DEFAULT_PORT;
	// NOTE: ConfigDebugClose shuts the LOGUART down entirely (verified on
	// hardware: all output stops, including LibreTiny's), so SDK verbosity is
	// left alone here.
}
