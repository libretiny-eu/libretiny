/* Early output works before constructors and FreeRTOS are running.
 *
 * Log backend: the SDK's console_hs_uart path (HalSerialPutcRtl8195a) is gated
 * on an adapter struct that the ROM re-registers/re-inits, and it forwards to
 * the mbed serial driver for the same UART -- two drivers, one UART. The log
 * UART register block (0x40003000) is the one the ROM loader, the ROM banner and
 * the SDK's own DiagPrintf already drive on this board (our captures show them),
 * and the IER/LSR accesses this core makes there take effect, so the app's own
 * print path writes it directly and polled, with a bounded spin so a wedged UART
 * cannot hang the caller. */
#include <libretiny.h>
#include <printf/printf.h>
#include <ameba_soc.h>

uint8_t lt_uart_port = 2;

#define LT_LOG_UART_BASE	 0x40003000u
#define LT_LOG_UART_THR		 0x00u /* transmit holding register */
#define LT_LOG_UART_LSR		 0x14u /* line status register */
#define LT_LOG_UART_LSR_THRE 0x20u /* bit 5: TX holding register empty */
#define LT_LOG_UART_SPIN_MAX 200000u

void lt_amb1_log_putc(char c) {
	for (uint32_t spin = 0; spin < LT_LOG_UART_SPIN_MAX; spin++) {
		if (HAL_READ32(LT_LOG_UART_BASE, LT_LOG_UART_LSR) & LT_LOG_UART_LSR_THRE)
			break;
	}
	HAL_WRITE32(LT_LOG_UART_BASE, LT_LOG_UART_THR, (uint8_t)c);
}

/* Plain string helper for bring-up diagnostics that must use the app's own path. */
void lt_amb1_log_puts(const char *s) {
	while (*s)
		lt_amb1_log_putc(*s++);
}

void putchar_(char c) {
	lt_amb1_log_putc(c);
}

void putchar_p(char c, unsigned long port) {
	/* Keep debug output off the appliance UART. */
	(void)port;
	lt_amb1_log_putc(c);
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
