/* Copyright (c) Kuba Szczodrzyński 2022-07-03. */

#include "SerialPrivate.h"

#if HAS_SERIAL0
SerialClass Serial0(0);
#endif
#if HAS_SERIAL1
SerialClass Serial1(1);
#endif
#if HAS_SERIAL2
SerialClass Serial2(2);
#endif

static UART_TypeDef *PORT_UART[3]  = {UART0_DEV, UART1_DEV, UART2_DEV};
static const IRQn PORT_IRQ[3]	   = {UART0_IRQ, UART1_IRQ, UART_LOG_IRQ};
static const pin_size_t PORT_RX[3] = {
#ifdef PIN_SERIAL0_RX
	PIN_SERIAL0_RX,
#else
	PIN_INVALID,
#endif
#ifdef PIN_SERIAL1_RX
	PIN_SERIAL1_RX,
#else
	PIN_INVALID,
#endif
#ifdef PIN_SERIAL2_RX
	PIN_SERIAL2_RX,
#else
	PIN_INVALID,
#endif
};
static const pin_size_t PORT_TX[3] = {
#ifdef PIN_SERIAL0_TX
	PIN_SERIAL0_TX,
#else
	PIN_INVALID,
#endif
#ifdef PIN_SERIAL1_TX
	PIN_SERIAL1_TX,
#else
	PIN_INVALID,
#endif
#ifdef PIN_SERIAL2_TX
	PIN_SERIAL2_TX,
#else
	PIN_INVALID,
#endif
};

static uint32_t callback(void *param) {
	UART_TypeDef *uart = pdUART;

	uint32_t intcr	= uart->DLH_INTCR;
	uart->DLH_INTCR = 0;

	uint8_t c;
	while (UART_Readable(uart)) {
		UART_CharGet(uart, &c);
#if LT_AUTO_DOWNLOAD_REBOOT && defined(LT_UART_ADR_PATTERN) && defined(PIN_SERIAL2_RX)
		// parse UART protocol commands on UART2
		if (uart == UART2_DEV)
			SerialClass::adrParse(c);
#endif
		pdBUF.store_char(c);
	}

	uart->DLH_INTCR = intcr;
	return 0;
}

void SerialClass::begin(unsigned long baudrate, uint16_t config) {
	this->data = new SerialData();
	this->buf  = &BUF;
	DATA->uart = PORT_UART[this->port];
	DATA->irq  = PORT_IRQ[this->port];
	DATA->rx   = PORT_RX[this->port];
	DATA->tx   = PORT_TX[this->port];

	// RUART_WLS_7BITS / RUART_WLS_8BITS
	uint8_t dataWidth = (config & SERIAL_DATA_MASK) == SERIAL_DATA_8;
	// RUART_PARITY_DISABLE / RUART_PARITY_ENABLE
	uint8_t parity = (config & SERIAL_PARITY_MASK) != SERIAL_PARITY_NONE;
	// RUART_ODD_PARITY / RUART_EVEN_PARITY
	uint8_t parityType = (config & SERIAL_PARITY_MASK) == SERIAL_PARITY_EVEN;
	// RUART_STOP_BIT_1 / RUART_STOP_BIT_2
	uint8_t stopBits = (config & SERIAL_STOP_BIT_MASK) == SERIAL_STOP_BIT_2;

	switch ((uint32_t)DATA->uart) {
		case UART0_REG_BASE:
			RCC_PeriphClockCmd(APBPeriph_UART0, APBPeriph_UART0_CLOCK, ENABLE);
			break;
		case UART1_REG_BASE:
			RCC_PeriphClockCmd(APBPeriph_UART1, APBPeriph_UART1_CLOCK, ENABLE);
			break;
	}

	if (DATA->tx != PIN_INVALID) {
		Pinmux_Config(DATA->tx, PINMUX_FUNCTION_UART);
	}
	if (DATA->rx != PIN_INVALID) {
		Pinmux_Config(DATA->rx, PINMUX_FUNCTION_UART);
		PAD_PullCtrl(DATA->rx, GPIO_PuPd_UP);
	}

	UART_InitTypeDef cfg;
	UART_StructInit(&cfg);
	cfg.WordLen	   = dataWidth;
	cfg.Parity	   = parity;
	cfg.ParityType = parityType;
	cfg.StopBit	   = stopBits;
	UART_Init(UART, &cfg);
	UART_SetBaud(UART, baudrate);

	if (DATA->rx != PIN_INVALID) {
		VECTOR_IrqUnRegister(DATA->irq);
		VECTOR_IrqRegister(callback, DATA->irq, (uint32_t)&data, 10);
		VECTOR_IrqEn(DATA->irq, 10);
		UART_RxCmd(UART, ENABLE);
		UART_INTConfig(UART, RUART_IER_ERBI, ENABLE);
	}
}

void SerialClass::end() {
	if (UART == UART2_DEV) {
		// restore command line mode
		DIAG_UartReInit((IRQ_FUN)UartLogIrqHandle);
	}
	this->buf = NULL;
	delete DATA;
}

void SerialClass::flush() {
	UART_WaitBusy(UART, 10);
}

size_t SerialClass::write(uint8_t c) {
	while (UART_Writable(UART) == 0) {}
	UART_CharPut(UART, c);
	return 1;
}
