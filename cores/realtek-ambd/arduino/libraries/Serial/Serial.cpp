/* Copyright (c) Bl00d-B0b 2026-08-07. */

#if LT_ARD_HAS_SERIAL || DOXYGEN

#include "SerialPrivate.h"

// AmebaD's KM4 UARTs: UART0 (PA), UART1 (BT), UART2 is the KM0-owned LOGUART.
// The IRQ table and register access mirror AmebaZ; the differences are the
// irq_register()/irq_enable() naming and the per-port clock enable.

static u32 callback(void *param) {
	SerialData *data   = (SerialData *)param;
	UART_TypeDef *uart = data->uart;

	u32 intcr		= uart->DLH_INTCR;
	uart->DLH_INTCR = 0;

	u8 c;
	while (UART_Readable(uart)) {
		UART_CharGet(uart, &c);
		data->buf->store_char(c);
	}

	uart->DLH_INTCR = intcr;
	return 0;
}

void SerialClass::beginPrivate(unsigned long baudrate, uint16_t config) {
	if (!this->data)
		return;
	this->data->buf = this->rxBuf;

	// Variant port number -> UART_DEV_TABLE index. The table order is UART0, UART1
	// (BT), LOGUART, LP UART: SERIAL0 (PA7/PA8) is the LOGUART at index 2 and
	// SERIAL2 (PB1/PB2) the LP UART at index 3.
	u8 idx;
	switch (this->port) {
		case 0:
			idx = 2; // LOGUART, brought up by the ROM
			break;
		case 1:
			idx = 1; // UART1 is wired to the BT controller
			RCC_PeriphClockCmd(APBPeriph_UART1, APBPeriph_UART1_CLOCK, ENABLE);
			break;
		default:
			idx = 3; // LP UART on PB1/PB2
			// The LP UART lives in the KM0 (LP) domain, where it is "LSYS UART1". Nothing
			// enables it for KM4, so turn on its LP-side clock and function here (the KM4
			// ROM's RCC_PeriphClockCmd only reaches the HP registers).
			HAL_WRITE32(
				SYSTEM_CTRL_BASE_LP,
				REG_LP_CLK_CTRL1,
				HAL_READ32(SYSTEM_CTRL_BASE_LP, REG_LP_CLK_CTRL1) | BIT_LSYS_UART1_CKE
			);
			HAL_WRITE32(
				SYSTEM_CTRL_BASE_LP,
				REG_LP_FUNC_EN1,
				HAL_READ32(SYSTEM_CTRL_BASE_LP, REG_LP_FUNC_EN1) | BIT_LSYS_UART1_FEN_GLB | BIT_LSYS_UART1_FEN_FUN
			);
			break;
	}
	this->data->uart = UART_DEV_TABLE[idx].UARTx;
	this->data->irq	 = UART_DEV_TABLE[idx].IrqNum;

	// PA7/PA8 carry the LOGUART only with the LOGUART pinmux function (SDK PinMap_UART_TX/RX).
	u32 function = this->port == 0 ? PINMUX_FUNCTION_LOGUART : PINMUX_FUNCTION_UART;
	if (this->tx != PIN_INVALID)
		Pinmux_Config(this->tx, function);
	if (this->rx != PIN_INVALID) {
		Pinmux_Config(this->rx, function);
		PAD_PullCtrl(this->rx, GPIO_PuPd_UP);

		IRQn_Type irq = this->data->irq;
		InterruptUnRegister(irq);
		InterruptRegister((IRQ_FUN)callback, irq, (u32)this->data, 10);
		InterruptEn(irq, 10);
	}
}

void SerialClass::configure(unsigned long baudrate, uint16_t config) {
	if (!this->data)
		return;
	UART_TypeDef *uart = this->data->uart;

	UART_InitTypeDef cfg;
	UART_StructInit(&cfg);
	cfg.WordLen	   = (config & SERIAL_DATA_MASK) == SERIAL_DATA_8;
	cfg.Parity	   = (config & SERIAL_PARITY_MASK) != SERIAL_PARITY_NONE;
	cfg.ParityType = (config & SERIAL_PARITY_MASK) == SERIAL_PARITY_EVEN;
	cfg.StopBit	   = (config & SERIAL_STOP_BIT_MASK) == SERIAL_STOP_BIT_2;
	UART_Init(uart, &cfg);
	// As the SDK's serial_baud(): run the UART from the 40 MHz XTAL before computing the
	// divisor. Left alone, the LP UART stays on the low-power oscillator and never
	// transmits. The LOGUART is already on XTAL (set up by the ROM), so leave it be.
	if (this->port != 0)
		RCC_PeriphClockSource_UART(uart, UART_RX_CLK_XTAL_40M);
	UART_SetBaud(uart, baudrate);
	// UART_Init() resets the RX path, so enable RX last (SDK order).
	if (this->rx != PIN_INVALID) {
		UART_INTConfig(uart, RUART_IER_ERBI, ENABLE);
		UART_RxCmd(uart, ENABLE);
	}

	this->baudrate = baudrate;
	this->config   = config;
}

void SerialClass::endPrivate() {
	if (!this->data || !this->data->uart)
		return;
	UART_TypeDef *uart = this->data->uart;

	UART_INTConfig(uart, RUART_IER_ERBI, DISABLE);
	UART_RxCmd(uart, DISABLE);
	InterruptDis(this->data->irq);
	InterruptUnRegister(this->data->irq);
	UART_DeInit(uart);
}

void SerialClass::flush() {
	if (!this->data)
		return;
	UART_WaitBusy(this->data->uart, 10);
}

size_t SerialClass::write(uint8_t c) {
	if (!this->data)
		return 0;
	while (UART_Writable(this->data->uart) == 0) {}
	UART_CharPut(this->data->uart, c);
	return 1;
}

#endif
