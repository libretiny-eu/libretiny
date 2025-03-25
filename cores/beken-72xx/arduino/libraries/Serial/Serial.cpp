/* Copyright (c) Kuba Szczodrzyński 2022-06-23. */

#include "SerialPrivate.h"

#if LT_HW_UART1
SerialClass Serial1(UART1_PORT);
#endif
#if LT_HW_UART2
SerialClass Serial2(UART2_PORT);
#endif

static void callback(int port, void *param) {
	int ch;
	while ((ch = uart_read_byte(port)) != -1) {
#if LT_AUTO_DOWNLOAD_REBOOT && defined(LT_UART_ADR_PATTERN) && PIN_SERIAL1_RX != PIN_INVALID
		// parse UART protocol commands on UART1
		if (port == UART1_PORT)
			SerialClass::adrParse(ch);
#endif
		pBUF->store_char(ch);
	}
}

void SerialClass::begin(unsigned long baudrate, uint16_t config) {
	if (!this->data) {
		this->data = new SerialData();
		this->buf  = &BUF;
	}

	if (this->baudrate != baudrate || this->config != config)
		this->configure(baudrate, config);
}

void SerialClass::configure(unsigned long baudrate, uint16_t config) {
	if (!this->data)
		return;

	uint8_t dataWidth = ((config & SERIAL_DATA_MASK) >> 8) - 1;				  // 0x100..0x400 -> 0..3
	uint8_t parity	  = 3 - (config & SERIAL_PARITY_MASK);					  // 0x3..0x1 -> 0..2
	uint8_t stopBits  = (config & SERIAL_STOP_BIT_MASK) == SERIAL_STOP_BIT_2; // 0x10..0x30 -> 0..1

	bk_uart_config_t cfg = {
		.baud_rate	  = baudrate,
		.data_width	  = (uart_data_width_t)dataWidth,
		.parity		  = (uart_parity_t)parity,
		.stop_bits	  = (uart_stop_bits_t)stopBits,
		.flow_control = FLOW_CTRL_DISABLED,
	};

	if (port == 1)
		uart1_init();
	else if (port == 2)
		uart2_init();
	uart_hw_set_change(port, &cfg);
	uart_rx_callback_set(port, callback, &BUF);

	this->baudrate = baudrate;
	this->config   = config;
}

void SerialClass::end() {
	if (!this->data)
		return;

	uart_rx_callback_set(port, NULL, NULL);
	switch (port) {
		case 1:
			uart1_exit();
			break;
		case 2:
			uart2_exit();
			break;
	}

	delete DATA;
	this->data	   = NULL;
	this->buf	   = NULL;
	this->baudrate = 0;
}

void SerialClass::flush() {
	if (!this->data)
		return;
	uart_wait_tx_over();
}

size_t SerialClass::write(uint8_t c) {
	if (!this->data)
		return 0;
	bk_send_byte(port, c);
	return 1;
}
