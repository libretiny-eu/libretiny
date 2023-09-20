/* Copyright (c) Kuba Szczodrzyński 2023-05-23. */

#include "Serial.h"

SerialClass::SerialClass(uint32_t port, pin_size_t rx, pin_size_t tx) {
	this->port = port;
	this->rx   = rx;
	this->tx   = tx;
	this->buf  = NULL;
	this->data = NULL;
}

#if LT_AUTO_DOWNLOAD_REBOOT && defined(LT_UART_ADR_PATTERN)
static uint8_t adrState = 0;
static uint8_t adrCmd[] = {LT_UART_ADR_PATTERN};

void SerialClass::adrParse(uint8_t c) {
	adrState = (adrState + 1) * (c == adrCmd[adrState]);
	if (adrState == sizeof(adrCmd) / sizeof(uint8_t)) {
		LT_I("Auto download mode: rebooting");
		LT.restartDownloadMode();
	}
}
#endif

int SerialClass::available() {
	return this->buf ? this->buf->available() : 0;
}

int SerialClass::peek() {
	return this->buf ? this->buf->peek() : -1;
}

int SerialClass::read() {
	return this->buf ? this->buf->read_char() : -1;
}
