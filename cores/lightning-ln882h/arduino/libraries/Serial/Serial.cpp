/* Copyright (c) Etienne Le Cousin 2024-02-10. */

#if LT_ARD_HAS_SERIAL || DOXYGEN

#include "SerialPrivate.h"

extern Serial_t *serial_handles[SER_PORT_NUM];
static SerialRingBuffer *serial_buf[SER_PORT_NUM];

#if LT_HW_UART0
static void callback_uart0(void) {
	char ch;
	while (serial_read(serial_handles[0], &ch, 1)) {
		serial_buf[0]->store_char(ch);
	}
}
#else
#define callback_uart0 nullptr
#endif

#if LT_HW_UART1
static void callback_uart1(void) {
	char ch;
	while (serial_read(serial_handles[1], &ch, 1)) {
		serial_buf[1]->store_char(ch);
	}
}
#else
#define callback_uart1 nullptr
#endif

#if LT_HW_UART2
static void callback_uart2(void) {
	char ch;
	while (serial_read(serial_handles[2], &ch, 1)) {
		serial_buf[2]->store_char(ch);
	}
}
#else
#define callback_uart2 nullptr
#endif

static const serial_rx_callbcak serial_rx_callbacks[SER_PORT_NUM] = {
	callback_uart0,
	callback_uart1,
	callback_uart2,
};

void SerialClass::beginPrivate(unsigned long baudrate, uint16_t config) {
	if (!this->data)
		return;
	serial_buf[this->port] = this->rxBuf;
}

void SerialClass::configure(unsigned long baudrate, uint16_t config) {
	if (!this->data)
		return;

	serial_init(serial_handles[this->port], (serial_port_id_t)this->port, baudrate, serial_rx_callbacks[this->port]);

	this->baudrate = baudrate;
	this->config   = config;
}

void SerialClass::endPrivate() {
	if (!this->data)
		return;

	serial_deinit(serial_handles[this->port]);
}

void SerialClass::flush() {
	if (!this->data)
		return;
	serial_flush(serial_handles[this->port]);
}

size_t SerialClass::write(uint8_t c) {
	if (!this->data)
		return 0;
	serial_putchar(serial_handles[this->port], c);
	return 1;
}

#endif
