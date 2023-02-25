/* Copyright (c) Kuba Szczodrzyński 2022-07-03. */

#include "SerialClass.h"

#ifdef PIN_SERIAL0_TX
SerialClass Serial0(UART0_DEV, UART0_IRQ, PIN_SERIAL0_RX, PIN_SERIAL0_TX);
#endif
#ifdef PIN_SERIAL1_TX
SerialClass Serial1(UART1_DEV, UART1_IRQ, PIN_SERIAL1_RX, PIN_SERIAL1_TX);
#endif
#ifdef PIN_SERIAL2_TX
SerialClass Serial2(UART2_DEV, UART_LOG_IRQ, PIN_SERIAL2_RX, PIN_SERIAL2_TX);
#endif

SerialClass::SerialClass(UART_TypeDef *uart, IRQn irq, pin_size_t rx, pin_size_t tx) {
	data.uart = uart;
	data.buf  = NULL;
	this->irq = irq;
	this->rx  = rx;
	this->tx  = tx;
}

static uint32_t callback(void *param) {
	SerialData *data = (SerialData *)param;

	uint32_t intcr		  = data->uart->DLH_INTCR;
	data->uart->DLH_INTCR = 0;

	uint8_t c;
	UART_CharGet(data->uart, &c);
	if (c)
		data->buf->store_char(c);

	data->uart->DLH_INTCR = intcr;
	return 0;
}

void SerialClass::begin(unsigned long baudrate, uint16_t config) {
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
	UART_Init(data.uart, &cfg);
	UART_SetBaud(data.uart, baudrate);

	if (data.buf) {
		data.buf->clear();
	} else {
		data.buf = new RingBuffer();
	}

	Pinmux_Config(pinInfo(this->rx)->gpio, PINMUX_FUNCTION_UART);
	Pinmux_Config(pinInfo(this->tx)->gpio, PINMUX_FUNCTION_UART);

	VECTOR_IrqUnRegister(this->irq);
	VECTOR_IrqRegister(callback, this->irq, (uint32_t)&data, 10);
}

void SerialClass::end() {
	if (data.uart == UART2_DEV) {
		// restore command line mode
		DIAG_UartReInit((IRQ_FUN)UartLogIrqHandle);
	}
	delete data.buf;
}

int SerialClass::available() {
	return data.buf->available();
}

int SerialClass::peek() {
	return data.buf->peek();
}

int SerialClass::read() {
	return data.buf->read_char();
}

void SerialClass::flush() {
	UART_WaitBusy(data.uart, 10);
}

size_t SerialClass::write(uint8_t c) {
	while (UART_Writable(data.uart) == 0) {}
	UART_CharPut(data.uart, c);
	return 1;
}
