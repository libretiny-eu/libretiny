/* Copyright (c) Bl00d-B0b 2026-08-06. */

#include <libretiny.h>

#include <printf/printf.h>

// AmebaD (RTL8721D) UART devices, KM4 view (hal_platform.h):
// UART0 0x40004000, UART1 0x40004400 (BT), KM0 LOG UART 0x4800C000.
// The ROM exports LOGUART_PutChar/UART_CharPut as _LONG_CALL_ symbols.
extern void LOGUART_PutChar(unsigned char c);
extern unsigned int UART_Writable(void *UARTx);
extern void UART_CharPut(void *UARTx, unsigned char TxData);

#define UART0_REG_BASE 0x40004000
#define UART1_REG_BASE 0x40004400

static void *uart_dev[2] = {
	(void *)UART0_REG_BASE,
	(void *)UART1_REG_BASE,
};

// Port 2 = LOG UART (the KM0-owned console the ROM already initialized).
uint8_t lt_uart_port = 2;

void putchar_(char c) {
	putchar_p(c, lt_uart_port);
}

void putchar_p(char c, unsigned long port) {
	if (port >= 2) {
		LOGUART_PutChar((unsigned char)c);
		return;
	}
	while (UART_Writable(uart_dev[port]) == 0) {}
	UART_CharPut(uart_dev[port], (unsigned char)c);
}

WRAP_PRINTF(DiagPrintf);
WRAP_PRINTF(DiagPrintfD);
WRAP_SPRINTF(DiagSPrintf);
WRAP_PRINTF(rtl_printf);
