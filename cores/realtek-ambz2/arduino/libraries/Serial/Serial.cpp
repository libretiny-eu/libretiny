/* Copyright (c) Kuba Szczodrzyński 2023-05-24. */

#if LT_ARD_HAS_SERIAL || DOXYGEN

#include "SerialPrivate.h"

static void callback(SerialData *data, uint32_t event) {
	if (event != RxIrq)
		return;
	hal_uart_adapter_t *uart = data->uart;

	uint8_t c;
	while (hal_uart_rgetc(uart, (char *)&c)) {
#if LT_AUTO_DOWNLOAD_REBOOT && defined(LT_UART_ADR_PATTERN) && PIN_SERIAL2_RX != PIN_INVALID
		// parse UART protocol commands on UART2
		if (uart->base_addr == UART2)
			SerialClass::adrParse(c);
#endif
		data->buf->store_char(c);
	}
}

void SerialClass::beginPrivate(unsigned long baudrate, uint16_t config) {
	if (!this->data)
		return;
	this->data->buf = this->rxBuf;

	hal_uart_adapter_t *uart;
	if (this->port == 2) {
		this->data->uart = uart = &log_uart;
	} else {
		this->data->uart = uart = new hal_uart_adapter_t();
		// TODO handle PIN_INVALID
		hal_uart_init(uart, this->tx, this->rx, nullptr);
	}

	if (this->rx != PIN_INVALID) {
		hal_uart_enter_critical();
		hal_uart_rxind_hook(uart, (uart_irq_callback_t)callback, (uint32_t)this->data, RxIrq);
		uart->base_addr->ier_b.erbi	 = 1;
		uart->base_addr->ier_b.etbei = 0;
		hal_uart_exit_critical();
	}
}

void SerialClass::configure(unsigned long baudrate, uint16_t config) {
	if (!this->data)
		return;

	uint8_t dataWidth = (config & SERIAL_DATA_MASK) == SERIAL_DATA_7 ? 7 : 8;
	uint8_t parity	  = (config & SERIAL_PARITY_MASK) ^ 0b11;
	uint8_t stopBits  = (config & SERIAL_STOP_BIT_MASK) == SERIAL_STOP_BIT_2 ? 2 : 1;

	hal_uart_set_baudrate(this->data->uart, baudrate);
	hal_uart_set_format(this->data->uart, dataWidth, parity, stopBits);

	this->baudrate = baudrate;
	this->config   = config;
}

void SerialClass::endPrivate() {
	if (!this->data)
		return;

	hal_uart_adapter_t *uart = this->data->uart;
	if (this->port == 2) {
		uart->base_addr->ier_b.erbi = 0;
		hal_uart_rxind_hook(uart, nullptr, 0, RxIrq);
	} else {
		hal_uart_deinit(uart);
		delete this->data->uart;
	}
}

void SerialClass::flush() {
	if (!this->data)
		return;
	while (this->data->uart->base_addr->tflvr_b.tx_fifo_lv != 0) {}
}

size_t SerialClass::write(uint8_t c) {
	if (!this->data)
		return 0;
	while (!hal_uart_writeable(this->data->uart)) {}
	hal_uart_putc(this->data->uart, c);
	return 1;
}

#endif
