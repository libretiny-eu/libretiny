/* Copyright (c) WGM160P-LibreTiny port 2026-05-26.
 *
 * Family-side SerialClass implementation for silabs-efm32gg11.
 *
 * Wraps emlib USART (em_usart) into LibreTiny's common SerialClass surface:
 *   - beginPrivate(): stash USART/loc/rxBuf pointers into SerialData, set up
 *                     GPIO routing. The actual USART/IRQ init happens in
 *                     configure() so that re-configure (e.g. baud change)
 *                     just re-runs USART_InitAsync().
 *   - configure():    USART_InitAsync at the requested baudrate, set ROUTELOC0
 *                     and ROUTEPEN, enable RXDATAV IRQ + NVIC line.
 *   - endPrivate():   disable IRQ + reset USART, clear ISR dispatch table.
 *   - flush():        wait for TXC (TX complete) status bit.
 *   - write():        blocking TX (wait for TXBL slot, then push one byte).
 *
 * RX path is IRQ-driven: USART0_RX_IRQHandler drains every available byte via
 * USART_Rx() and feeds the common SerialRingBuffer through store_char(). If
 * the ring is full we set SerialData::rx_overflow; user code can poll-and-clear
 * via SerialClass_getOverflow().
 *
 * Phase 1 scope: only USART0 LOC1 (WSTK VCOM, PE7=TX PE6=RX) is wired. The
 * code is structured so adding USART1..USARTn is a matter of extending the
 * port -> {USART_TypeDef *, IRQn} dispatch in beginPrivate() and adding more
 * named IRQ handlers.
 */

#if LT_ARD_HAS_SERIAL || DOXYGEN

#include "ArduinoFamily.h"
#include "SerialPrivate.h"

#include "em_cmu.h"
#include "em_gpio.h"
#include "em_usart.h"

/* Dispatch table: port number -> SerialData *. Populated by beginPrivate,
 * cleared by endPrivate. ISR consults this to find the SerialRingBuffer it
 * should fill. */
static SerialData *isr_data[SERIAL_INTERFACES_COUNT] = {nullptr};

void SerialClass::beginPrivate(unsigned long baudrate, uint16_t config) {
	(void)baudrate;
	(void)config;
	if (!this->data)
		return;

	/* Phase 1: only port 0 (USART0 LOC1) is wired. Reject other ports loudly
	 * so future additions don't silently mis-route. */
	if (this->port != 0) {
		LT_E("Serial port %u not supported on silabs-efm32gg11 (Phase 1: USART0 only)", this->port);
		return;
	}

	/* Populate SerialData for this port. */
	this->data->uart		= USART0;
	this->data->loc			= 1U;		   /* WSTK VCOM is USART0 LOC1 */
	this->data->buf			= this->rxBuf; /* stash for the ISR */
	this->data->rx_overflow = false;

	/* Enable peripheral clocks and configure GPIO via the variant pin
	 * encoding (port_index << 4 | pin). For the WSTK that's PE7=0x47 (TX)
	 * and PE6=0x46 (RX); ArduinoFamily.h's pin_port()/pin_index() decode
	 * those into emlib GPIO_Port_TypeDef + bit. */
	CMU_ClockEnable(cmuClock_GPIO, true);
	CMU_ClockEnable(cmuClock_USART0, true);
	if (this->tx != PIN_INVALID) {
		GPIO_PinModeSet(pin_port(this->tx), pin_index(this->tx), gpioModePushPull, 1U);
	}
	if (this->rx != PIN_INVALID) {
		GPIO_PinModeSet(pin_port(this->rx), pin_index(this->rx), gpioModeInput, 0U);
	}

	/* Publish the ISR dispatch entry. The NVIC line itself is unmasked in
	 * configure() once USART_InitAsync has happened. */
	isr_data[this->port] = this->data;
}

void SerialClass::configure(unsigned long baudrate, uint16_t config) {
	if (!this->data || !this->data->uart)
		return;

	/* (Re-)initialise USART for async mode at the requested baudrate. Phase 1
	 * is fixed at 8N1 — the config arg is accepted but only its baudrate is
	 * acted on. Honouring config (parity, stop bits) is a follow-up. */
	(void)config;
	USART_InitAsync_TypeDef init = USART_INITASYNC_DEFAULT;
	init.baudrate				 = baudrate;
	USART_InitAsync(this->data->uart, &init);

	/* Route TX/RX to the configured LOC. ROUTELOC0 holds 5-bit LOC fields for
	 * both TX and RX; ROUTEPEN enables the pin output drivers. */
	this->data->uart->ROUTELOC0 =
		(this->data->uart->ROUTELOC0 & ~(_USART_ROUTELOC0_TXLOC_MASK | _USART_ROUTELOC0_RXLOC_MASK)) |
		((uint32_t)this->data->loc << _USART_ROUTELOC0_TXLOC_SHIFT) |
		((uint32_t)this->data->loc << _USART_ROUTELOC0_RXLOC_SHIFT);
	this->data->uart->ROUTEPEN = USART_ROUTEPEN_TXPEN | USART_ROUTEPEN_RXPEN;

	/* Enable RX IRQ. Phase 1 has no TX IRQ path — write() is busy-wait. */
	USART_IntClear(this->data->uart, _USART_IF_MASK);
	USART_IntEnable(this->data->uart, USART_IEN_RXDATAV);
	NVIC_ClearPendingIRQ(USART0_RX_IRQn);
	NVIC_EnableIRQ(USART0_RX_IRQn);

	this->baudrate = baudrate;
	this->config   = config;
}

void SerialClass::endPrivate() {
	if (!this->data || !this->data->uart)
		return;

	USART_IntDisable(this->data->uart, USART_IEN_RXDATAV);
	NVIC_DisableIRQ(USART0_RX_IRQn);
	USART_Reset(this->data->uart);

	isr_data[this->port] = nullptr;
	this->data->uart	 = nullptr;
	this->data->buf		 = nullptr;
}

void SerialClass::flush() {
	if (!this->data || !this->data->uart)
		return;
	/* TXC = transmit complete (shift register empty + buffer empty). */
	while (!(this->data->uart->STATUS & USART_STATUS_TXC)) {
		/* spin */
	}
}

size_t SerialClass::write(uint8_t c) {
	if (!this->data || !this->data->uart)
		return 0;
	/* Blocking TX: wait for TXBL (transmit buffer level) then push one byte. */
	while (!(this->data->uart->STATUS & USART_STATUS_TXBL)) {
		/* spin */
	}
	USART_Tx(this->data->uart, c);
	return 1;
}

/* Family extension: read-and-clear the RX overflow flag of port 0. Free
 * function (not a member) because the common HardwareSerial surface doesn't
 * expose a hook for it. */
bool SerialClass_getOverflow() {
	SerialData *data = isr_data[0];
	if (data == nullptr)
		return false;
	bool v			  = data->rx_overflow;
	data->rx_overflow = false;
	return v;
}

/* USART0 RX IRQ handler. The vendor startup table (startup_efm32gg11b.S)
 * declares USART0_RX_IRQHandler as a .weak alias to Default_Handler, so our
 * strong definition takes precedence at link. */
extern "C" void USART0_RX_IRQHandler(void) {
	SerialData *data = isr_data[0];
	if (data == nullptr || data->uart == nullptr || data->buf == nullptr) {
		/* Spurious IRQ before begin() finished, or after end(). Clear flags
		 * defensively so we don't re-enter. */
		USART_IntClear(USART0, _USART_IF_MASK);
		return;
	}
	while (data->uart->STATUS & USART_STATUS_RXDATAV) {
		uint8_t b = (uint8_t)USART_Rx(data->uart);
#if LT_AUTO_DOWNLOAD_REBOOT && defined(LT_UART_ADR_PATTERN) && PIN_SERIAL0_RX != PIN_INVALID
		/* Snoop for the "reboot into download mode" magic pattern. Matches
		 * the convention from beken-72xx / realtek-amb* / lightning-ln882h. */
		SerialClass::adrParse(b);
#endif
		if (data->buf->isFull()) {
			/* Ring full — drop the byte and raise the overflow flag. */
			data->rx_overflow = true;
		} else {
			data->buf->store_char(b);
		}
	}
}

#endif /* LT_ARD_HAS_SERIAL */
