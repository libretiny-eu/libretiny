/* Copyright (c) Etienne Le Cousin 2024-02-10. */

#include "SerialPrivate.h"

extern Serial_t *serial_handles[SER_PORT_NUM];
static SerialData *serial_data[SER_PORT_NUM];

#if LT_HW_UART0
SerialClass Serial0(0, PIN_SERIAL0_RX, PIN_SERIAL0_TX);

static void callback_uart0(void) {
	SerialData *data = serial_data[0];
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
	SerialData *data = serial_data[1];
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
	SerialData *data = serial_data[2];
	char ch;
	while (serial_read(serial_handles[2], &ch, 1)) {
		data->buf.store_char(ch);
	}
}
#else
#define callback_uart2 NULL
#endif

static const serial_rx_callbcak serial_rx_callbacks[SER_PORT_NUM] = {
	callback_uart0,
	callback_uart1,
	callback_uart2,
};

void SerialClass::begin(unsigned long baudrate, uint16_t config) {
	if (!this->data) {
		this->data = new SerialData();
		this->buf  = &this->data->buf;
	}

	if (this->baudrate != baudrate || this->config != config)
		this->configure(baudrate, config);
}

void SerialClass::configure(unsigned long baudrate, uint16_t config) {
	if (!this->data)
		return;

	serial_init(serial_handles[this->port], (serial_port_id_t)this->port, baudrate, serial_rx_callbacks[this->port]);
	serial_data[this->port] = this->data;

	this->baudrate = baudrate;
	this->config   = config;
}

void SerialClass::end() {
	if (!this->data)
		return;

	serial_deinit(serial_handles[this->port]);

	delete this->data;
	this->data	   = NULL;
	this->buf	   = NULL;
	this->baudrate = 0;
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
