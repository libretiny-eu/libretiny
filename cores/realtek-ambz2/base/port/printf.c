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
	while (uart_dev[port]->tflvr_b.tx_fifo_lv >= Uart_Tx_FIFO_Size) {}
	uart_dev[port]->thr = c;
}

WRAP_PRINTF(rt_printf);
WRAP_SPRINTF(rt_sprintf);
WRAP_SNPRINTF(rt_snprintf);
WRAP_PRINTF(rt_log_printf);
