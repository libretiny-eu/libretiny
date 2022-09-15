/* Copyright (c) Kuba Szczodrzyński 2022-07-03. */

#include "SoftwareSerial.h"

#ifdef LT_ARD_HAS_SOFTSERIAL

SoftwareSerial::SoftwareSerial(pin_size_t receivePin, pin_size_t transmitPin, bool inverted) {
	data.rx.buf = NULL;
	data.tx.buf = NULL;
	data.rx.pin = receivePin;
	data.tx.pin = transmitPin;
	data.invert = inverted == true;
}

int SoftwareSerial::available() {
	return data.rx.buf->available();
}

int SoftwareSerial::peek() {
	return data.rx.buf->peek();
}

int SoftwareSerial::read() {
	return data.rx.buf->read_char();
}

void SoftwareSerial::flush() {
	while (data.rx.buf->available()) {
		yield();
	}
}

size_t SoftwareSerial::write(uint8_t c) {
	while (data.tx.buf->isFull()) {
		yield();
	}
	data.tx.buf->store_char(c);
	if (data.tx.state == SS_IDLE) {
		data.tx.state = SS_START;
		this->startTx();
	}
	return 1;
}

#endif
