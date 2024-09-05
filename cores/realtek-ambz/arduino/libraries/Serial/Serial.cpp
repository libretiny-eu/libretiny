/* Copyright (c) Kuba Szczodrzyński 2022-07-03. */

#if LT_ARD_HAS_SERIAL || DOXYGEN

#include "SerialPrivate.h"

static uint32_t callback(SerialData *data) {
	UART_TypeDef *uart = data->uart;

	uint32_t intcr	= uart->DLH_INTCR;
	uart->DLH_INTCR = 0;

	uint8_t c;
	while (UART_Readable(uart)) {
		UART_CharGet(uart, &c);
#if LT_AUTO_DOWNLOAD_REBOOT && defined(LT_UART_ADR_PATTERN) && PIN_SERIAL2_RX != PIN_INVALID
		// parse UART protocol commands on UART2
		if (uart == UART2_DEV)
			SerialClass::adrParse(c);
#endif
		data->buf->store_char(c);
	}

	uart->DLH_INTCR = intcr;
	return 0;
}

void SerialClass::beginPrivate(unsigned long baudrate, uint16_t config) {
	if (!this->data)
		return;
	this->data->buf	 = this->rxBuf;
	this->data->uart = UART_DEV_TABLE[this->port].UARTx;
	this->data->irq	 = (IRQn)UART_DEV_TABLE[this->port].IrqNum;

	switch (this->port) {
		case 0:
			RCC_PeriphClockCmd(APBPeriph_UART0, APBPeriph_UART0_CLOCK, ENABLE);
			break;
		case 1:
			RCC_PeriphClockCmd(APBPeriph_UART1, APBPeriph_UART1_CLOCK, ENABLE);
			break;
	}

	if (this->tx != PIN_INVALID) {
		Pinmux_Config(this->tx, PINMUX_FUNCTION_UART);
	}
	if (this->rx != PIN_INVALID) {
		Pinmux_Config(this->rx, PINMUX_FUNCTION_UART);
		PAD_PullCtrl(this->rx, GPIO_PuPd_UP);
	}

	if (this->rx != PIN_INVALID) {
		UART_TypeDef *uart = this->data->uart;
		IRQn irq		   = this->data->irq;

		VECTOR_IrqUnRegister(irq);
		VECTOR_IrqRegister((IRQ_FUN)callback, irq, (uint32_t)this->data, 10);
		VECTOR_IrqEn(irq, 10);
		UART_RxCmd(uart, ENABLE);
		UART_INTConfig(uart, RUART_IER_ERBI, ENABLE);
	}
}

void SerialClass::configure(unsigned long baudrate, uint16_t config) {
	if (!this->data)
		return;
	UART_TypeDef *uart = this->data->uart;

	// RUART_WLS_7BITS / RUART_WLS_8BITS
	uint8_t dataWidth = (config & SERIAL_DATA_MASK) == SERIAL_DATA_8;
	// RUART_PARITY_DISABLE / RUART_PARITY_ENABLE
	uint8_t parity = (config & SERIAL_PARITY_MASK) != SERIAL_PARITY_NONE;
	// RUART_ODD_PARITY / RUART_EVEN_PARITY
	uint8_t parityType = (config & SERIAL_PARITY_MASK) == SERIAL_PARITY_EVEN;
	// RUART_STOP_BIT_1 / RUART_STOP_BIT_2
	uint8_t stopBits = (config & SERIAL_STOP_BIT_MASK) == SERIAL_STOP_BIT_2;

	UART_InitTypeDef cfg;
	UART_StructInit(&cfg);
	cfg.WordLen	   = dataWidth;
	cfg.Parity	   = parity;
	cfg.ParityType = parityType;
	cfg.StopBit	   = stopBits;
	UART_Init(uart, &cfg);
	UART_SetBaud(uart, baudrate);

	this->baudrate = baudrate;
	this->config   = config;
}

void SerialClass::endPrivate() {
	if (!this->data)
		return;
	UART_TypeDef *uart = this->data->uart;
	IRQn irq		   = this->data->irq;

	UART_INTConfig(uart, RUART_IER_ERBI, DISABLE);
	UART_RxCmd(uart, DISABLE);
	VECTOR_IrqDis(irq);
	VECTOR_IrqUnRegister(irq);
	UART_DeInit(uart);

	if (uart == UART2_DEV) {
		// restore command line mode
		DIAG_UartReInit((IRQ_FUN)UartLogIrqHandle);
	}
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
