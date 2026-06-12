/* Copyright (c) WGM160P-LibreTiny port 2026-05-26.
 *
 * Phase 1 printf shim for silabs-efm32gg11.
 *
 * LibreTiny uses the mpaland/printf package — implement putchar_() and
 * putchar_p() to route characters to USART0 (WSTK virtual COM, LOC1:
 * PE7=TX, PE6=RX).
 *
 * NOTE: The plan called for _putchar(char), but LibreTiny's printf wiring
 * actually uses putchar_(char c) and putchar_p(char c, unsigned long port)
 * (see cores/common/base/config/printf_config.h and existing families).
 * Names adapted accordingly.
 */

#include "em_cmu.h"
#include "em_gpio.h"
#include "em_usart.h"
#include "lt_family.h"
#include <libretiny.h>

#include <printf/printf.h>

static void ensure_usart0_initialised(void) {
	/* USART0 was clock-enabled and pins configured in lt_init_family.
	 * USART itself is initialised lazily on first use here so framework=base
	 * sketches that bypass lt_init_family still get a working printf.
	 */
	static bool initialised = false;
	if (initialised) {
		return;
	}

	// Defensive: enable peripheral clocks even if lt_init_family hasn't run.
	CMU_ClockEnable(cmuClock_GPIO, true);
	CMU_ClockEnable(cmuClock_USART0, true);

	// VCOM pins (PE7=TX, PE6=RX) — safe to re-configure.
	GPIO_PinModeSet(gpioPortE, 7U, gpioModePushPull, 1U);
	GPIO_PinModeSet(gpioPortE, 6U, gpioModeInput, 0U);

	USART_InitAsync_TypeDef init = USART_INITASYNC_DEFAULT;
	init.baudrate				 = 115200;
	USART_InitAsync(USART0, &init);

	USART0->ROUTELOC0 = (USART0->ROUTELOC0 & ~(_USART_ROUTELOC0_TXLOC_MASK | _USART_ROUTELOC0_RXLOC_MASK)) |
						(1U << _USART_ROUTELOC0_TXLOC_SHIFT) | (1U << _USART_ROUTELOC0_RXLOC_SHIFT);
	USART0->ROUTEPEN = USART_ROUTEPEN_TXPEN | USART_ROUTEPEN_RXPEN;

	initialised = true;
}

void putchar_(char c) {
	ensure_usart0_initialised();
	USART_Tx(USART0, (uint8_t)c);
}

void putchar_p(char c, unsigned long port) {
	// Phase 1: single-port; ignore the port selector and route everything to VCOM.
	(void)port;
	putchar_(c);
}
