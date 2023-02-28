/* Copyright (c) Kuba Szczodrzyński 2022-06-23. */

#include "SerialClass.h"

extern "C" {

#include <uart_pub.h>

extern void bk_send_byte(uint8_t uport, uint8_t data);
extern void uart_hw_set_change(uint8_t uport, bk_uart_config_t *uart_config);
extern int uart_rx_callback_set(int uport, uart_callback callback, void *param);

} // extern "C"

#if HAS_SERIAL1
SerialClass Serial1(UART1_PORT);
#endif
#if HAS_SERIAL2
SerialClass Serial2(UART2_PORT);
#endif

SerialClass::SerialClass(uint8_t port) {
	this->port = port;
	this->buf  = NULL;
}

#if LT_AUTO_DOWNLOAD_REBOOT
static uint8_t adrState		  = 0;
static const uint8_t adrCmd[] = {0x01, 0xE0, 0xFC, 0x01, 0x00};

static void adrParse(uint8_t c) {
	// parse and respond to link check command (CMD_LinkCheck=0)
	adrState = (adrState + 1) * (c == adrCmd[adrState]);
	if (adrState == 5) {
		LT_I("Auto download mode: rebooting");
		LT.restart();
	}
}
#endif

static void callback(int port, void *param) {
	RingBuffer *buf = (RingBuffer *)param;
	int ch;
	while ((ch = uart_read_byte(port)) != -1) {
#if LT_AUTO_DOWNLOAD_REBOOT && defined(PIN_SERIAL1_RX)
		// parse UART protocol commands on UART1
		if (port == UART1_PORT)
			adrParse(ch);
#endif
		buf->store_char(ch);
	}
}

void SerialClass::begin(unsigned long baudrate, uint16_t config) {
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

	if (this->buf) {
		this->buf->clear();
	} else {
		this->buf = new RingBuffer();
	}

	uart_hw_set_change(port, &cfg);
	uart_rx_callback_set(port, callback, this->buf);
}

void SerialClass::end() {
	uart_rx_callback_set(port, NULL, NULL);
	switch (port) {
		case 1:
			uart1_exit();
			break;
		case 2:
			uart2_exit();
			break;
	}
	delete this->buf;
}

int SerialClass::available() {
	return buf->available();
}

int SerialClass::peek() {
	return buf->peek();
}

int SerialClass::read() {
	return buf->read_char();
}

void SerialClass::flush() {
	uart_wait_tx_over();
}

size_t SerialClass::write(uint8_t c) {
	bk_send_byte(port, c);
	return 1;
}
