/* Copyright (c) Etienne Le Cousin 2024-02-10. */

#include "SerialPrivate.h"

extern Serial_t *serial_handles[SER_PORT_NUM];

#if LT_HW_UART0
SerialClass Serial0(0, PIN_SERIAL0_RX, PIN_SERIAL0_TX);

static void callback_uart0(void) {
	SerialData *data = (SerialData *)Serial0.data;
	char ch;
	while (serial_read(serial_handles[0], &ch, 1)) {
		data->buf.store_char(ch);
	}
}
#else
#define callback_uart0 NULL
#endif
#if LT_HW_UART1
SerialClass Serial1(1, PIN_SERIAL1_RX, PIN_SERIAL1_TX);

static void callback_uart1(void) {
	SerialData *data = (SerialData *)Serial1.data;
	char ch;
	while (serial_read(serial_handles[1], &ch, 1)) {
		data->buf.store_char(ch);
	}
}
#else
#define callback_uart1 NULL
#endif
#if LT_HW_UART2
SerialClass Serial2(2, PIN_SERIAL2_RX, PIN_SERIAL2_TX);

static void callback_uart2(void) {
	SerialData *data = (SerialData *)Serial2.data;
	char ch;
	while (serial_read(serial_handles[2], &ch, 1)) {
		data->buf.store_char(ch);
	}
}
#else
#define callback_uart2 NULL
#endif

// clang-format off
static const serial_rx_callbcak serial_rx_callbacks[SER_PORT_NUM] = {
	callback_uart0,
	callback_uart1,
	callback_uart2
};
// clang-format on

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

	serial_init(serial_handles[port], (serial_port_id_t)port, baudrate, serial_rx_callbacks[port]);

	this->baudrate = baudrate;
	this->config   = config;
}

void SerialClass::end() {
	if (!this->data)
		return;

	serial_deinit(serial_handles[port]);

	delete (SerialData *)this->data;
	delete this->buf;
	this->data	   = NULL;
	this->buf	   = NULL;
	this->baudrate = 0;
}

void SerialClass::flush() {
	if (!this->data)
		return;
	serial_flush(serial_handles[port]);
}

size_t SerialClass::write(uint8_t c) {
	if (!this->data)
		return 0;
	serial_putchar(serial_handles[port], c);
	return 1;
}
