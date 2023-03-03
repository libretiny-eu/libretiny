/* Copyright (c) Kuba Szczodrzyński 2022-06-19. */

#include <libretuya.h>

#include <printf/printf.h>

#define LOG_UART_REG_BASE 0x40003000
#define UART0_REG_BASE	  0x40040000
#define UART1_REG_BASE	  0x40040400
#define UART2_REG_BASE	  LOG_UART_REG_BASE

extern uint32_t UART_Writable(void *UARTx);
extern void UART_CharPut(void *UARTx, uint8_t TxData);

static void *uart_dev[3] = {
	(void *)UART0_REG_BASE,
	(void *)UART1_REG_BASE,
	(void *)LOG_UART_REG_BASE,
};

uint8_t lt_uart_port = 2;

void putchar_(char c) {
	putchar_p(c, lt_uart_port);
}

void putchar_p(char c, unsigned long port) {
	while (UART_Writable(uart_dev[port]) == 0) {}
	UART_CharPut(uart_dev[port], c);
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
