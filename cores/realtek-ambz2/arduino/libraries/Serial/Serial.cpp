/* Copyright (c) Kuba Szczodrzyński 2023-05-24. */

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

static void callback(uint32_t param, uint32_t event) {
	if (event != RxIrq)
		return;
	hal_uart_adapter_t *uart = pdUART;

	uint8_t c;
	while (hal_uart_rgetc(uart, (char *)&c)) {
#if LT_AUTO_DOWNLOAD_REBOOT && defined(LT_UART_ADR_PATTERN) && PIN_SERIAL2_RX != PIN_INVALID
		// parse UART protocol commands on UART2
		if (uart->base_addr == UART2)
			SerialClass::adrParse(c);
#endif
		pdBUF.store_char(c);
	}
}

void SerialClass::begin(unsigned long baudrate, uint16_t config) {
	if (!this->data) {
		this->data = new SerialData();
		this->buf  = &BUF;

		if (this->port == 2) {
			DATA->uart = &log_uart;
		} else {
			UART = new hal_uart_adapter_t();
			// TODO handle PIN_INVALID
			hal_uart_init(UART, this->tx, this->rx, NULL);
		}

		if (this->rx != PIN_INVALID) {
			hal_uart_enter_critical();
			hal_uart_rxind_hook(UART, callback, (uint32_t)this->data, RxIrq);
			UART->base_addr->ier_b.erbi	 = 1;
			UART->base_addr->ier_b.etbei = 0;
			hal_uart_exit_critical();
		}
	}

	if (this->baudrate != baudrate || this->config != config)
		this->configure(baudrate, config);
}

void SerialClass::configure(unsigned long baudrate, uint16_t config) {
	if (!this->data)
		return;

	uint8_t dataWidth = (config & SERIAL_DATA_MASK) == SERIAL_DATA_7 ? 7 : 8;
	uint8_t parity	  = (config & SERIAL_PARITY_MASK) ^ 0b11;
	uint8_t stopBits  = (config & SERIAL_STOP_BIT_MASK) == SERIAL_STOP_BIT_2 ? 2 : 1;

	hal_uart_set_baudrate(UART, baudrate);
	hal_uart_set_format(UART, dataWidth, parity, stopBits);

	this->baudrate = baudrate;
	this->config   = config;
}

void SerialClass::end() {
	if (!this->data)
		return;

	if (this->port == 2) {
		UART->base_addr->ier_b.erbi = 0;
		hal_uart_rxind_hook(UART, NULL, 0, RxIrq);
	} else {
		hal_uart_deinit(UART);
		delete UART;
	}

	delete DATA;
	this->data	   = NULL;
	this->buf	   = NULL;
	this->baudrate = 0;
}

void SerialClass::flush() {
	if (!this->data)
		return;
	while (UART->base_addr->tflvr_b.tx_fifo_lv != 0) {}
}

size_t SerialClass::write(uint8_t c) {
	if (!this->data)
		return 0;
	while (!hal_uart_writeable(UART)) {}
	hal_uart_putc(UART, c);
	return 1;
}
