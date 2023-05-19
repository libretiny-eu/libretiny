/* Copyright (c) Kuba Szczodrzyński 2023-04-12. */

#include <libretiny.h>
#include <sdk_private.h>

#include <printf/printf.h>

static UART0_Type *uart_dev[4] = {
	UART0,
	UART1,
	UART2,
	UART3,
};

uint8_t lt_uart_port = 2;

void putchar_(char c) {
	putchar_p(c, lt_uart_port);
}

void putchar_p(char c, unsigned long port) {
	// while (UART_Writable(uart_dev[port]) == 0) {}
	uart_dev[port]->thr = c;
}

WRAP_PRINTF(rtl_printf);
WRAP_SPRINTF(rtl_sprintf);
WRAP_SNPRINTF(rtl_snprintf);
WRAP_VSNPRINTF(rtl_vsnprintf);
WRAP_VSNPRINTF(rtl_vsnprintf_r);
WRAP_VPRINTF(rtl_vprintf);
WRAP_PRINTF(DiagPrintf);
WRAP_SPRINTF(DiagSPrintf);
WRAP_SNPRINTF(DiagSnPrintf);
WRAP_PRINTF(prvDiagPrintf);
WRAP_SPRINTF(prvDiagSPrintf);
WRAP_VSPRINTF(VSprintf);
WRAP_PRINTF(LOG_PRINTF);
