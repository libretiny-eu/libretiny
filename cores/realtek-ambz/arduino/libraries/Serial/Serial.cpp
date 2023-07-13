/* Copyright (c) Kuba Szczodrzyński 2022-07-03. */

#include "SerialPrivate.h"

#if LT_HW_UART0
SerialClass Serial0(0, PIN_SERIAL0_RX, PIN_SERIAL0_TX);
#endif
#if LT_HW_UART1
SerialClass Serial1(1, PIN_SERIAL1_RX, PIN_SERIAL1_TX);
#endif
#if LT_HW_UART2
SerialClass Serial2(2, PIN_SERIAL2_RX, PIN_SERIAL2_TX);
#endif

static UART_TypeDef *PORT_UART[3] = {UART0_DEV, UART1_DEV, UART2_DEV};
static const IRQn PORT_IRQ[3]	  = {UART0_IRQ, UART1_IRQ, UART_LOG_IRQ};

static uint32_t callback(void *param) {
	UART_TypeDef *uart = pdUART;

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
		pdBUF.store_char(c);
	}

	uart->DLH_INTCR = intcr;
	return 0;
}

void SerialClass::begin(unsigned long baudrate, uint16_t config) {
	if (!this->data) {
		this->data = new SerialData();
		this->buf  = &BUF;
		DATA->uart = PORT_UART[this->port];
		DATA->irq  = PORT_IRQ[this->port];

		switch ((uint32_t)DATA->uart) {
			case UART0_REG_BASE:
				RCC_PeriphClockCmd(APBPeriph_UART0, APBPeriph_UART0_CLOCK, ENABLE);
				break;
			case UART1_REG_BASE:
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
	}

	if (this->baudrate != baudrate || this->config != config)
		this->configure(baudrate, config);

	if (this->rx != PIN_INVALID) {
		VECTOR_IrqUnRegister(DATA->irq);
		VECTOR_IrqRegister(callback, DATA->irq, (uint32_t)this->data, 10);
		VECTOR_IrqEn(DATA->irq, 10);
		UART_RxCmd(UART, ENABLE);
		UART_INTConfig(UART, RUART_IER_ERBI, ENABLE);
	}
}

void SerialClass::configure(unsigned long baudrate, uint16_t config) {
	if (!this->data)
		return;

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
	UART_Init(UART, &cfg);
	UART_SetBaud(UART, baudrate);

	this->baudrate = baudrate;
	this->config   = config;
}

void SerialClass::end() {
	if (!this->data)
		return;

	UART_INTConfig(UART, RUART_IER_ERBI, DISABLE);
	UART_RxCmd(UART, DISABLE);
	VECTOR_IrqDis(DATA->irq);
	VECTOR_IrqUnRegister(DATA->irq);
	UART_DeInit(UART);

	if (UART == UART2_DEV) {
		// restore command line mode
		DIAG_UartReInit((IRQ_FUN)UartLogIrqHandle);
	}

	delete DATA;
	this->data	   = NULL;
	this->buf	   = NULL;
	this->baudrate = 0;
}

void SerialClass::flush() {
	if (!this->data)
		return;
	UART_WaitBusy(UART, 10);
}

size_t SerialClass::write(uint8_t c) {
	if (!this->data)
		return 0;
	while (UART_Writable(UART) == 0) {}
	UART_CharPut(UART, c);
	return 1;
}
