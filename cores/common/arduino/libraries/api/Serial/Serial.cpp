/* Copyright (c) Kuba Szczodrzyński 2023-05-23. */

#if LT_ARD_HAS_SERIAL || DOXYGEN

#include <SerialPrivate.h>

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

bool SerialClass::validatePins(pin_size_t rx, pin_size_t tx) {
	// for each role (RX/TX) in each UART, fail validation if:
	// - a pin is specified and no such role exists on the board, or
	// - a pin is specified and is not among the supported pins for that role
	switch (this->port) {
#if LT_HW_UART0
		case 0:
			if (rx != PIN_INVALID
#ifdef PINS_SERIAL0_RX
				&& !ltArrayContains((pin_size_t[])PINS_SERIAL0_RX, rx)
#endif
			)
				return false;
			if (tx != PIN_INVALID
#ifdef PINS_SERIAL0_TX
				&& !ltArrayContains((pin_size_t[])PINS_SERIAL0_TX, tx)
#endif
			)
				return false;
			break;
#endif

#if LT_HW_UART1
		case 1:
			if (rx != PIN_INVALID
#ifdef PINS_SERIAL1_RX
				&& !ltArrayContains((pin_size_t[])PINS_SERIAL1_RX, rx)
#endif
			)
				return false;
			if (tx != PIN_INVALID
#ifdef PINS_SERIAL1_TX
				&& !ltArrayContains((pin_size_t[])PINS_SERIAL1_TX, tx)
#endif
			)
				return false;
			break;
#endif

#if LT_HW_UART2
		case 2:
			if (rx != PIN_INVALID
#ifdef PINS_SERIAL2_RX
				&& !ltArrayContains((pin_size_t[])PINS_SERIAL2_RX, rx)
#endif
			)
				return false;
			if (tx != PIN_INVALID
#ifdef PINS_SERIAL2_TX
				&& !ltArrayContains((pin_size_t[])PINS_SERIAL2_TX, tx)
#endif
			)
				return false;
			break;
#endif

		default:
			return false;
	}
	return true;
}

void SerialClass::begin(unsigned long baudrate, uint16_t config, pin_size_t rx, pin_size_t tx) {
	// let family code cleanup any resources
	this->end();

	// apply pin configuration
	if (!this->setPins(rx, tx))
		return;

	// allocate data structure and ring buffer
	if (!this->data) {
		this->data	= new SerialData();
		this->rxBuf = new SerialRingBuffer();
	}

	this->beginPrivate(baudrate, config);
	// if (this->baudrate != baudrate || this->config != config)
	this->configure(baudrate, config);
}

bool SerialClass::setPins(pin_size_t rx, pin_size_t tx) {
	// return early if configured pins are the same
	if (rx == this->rx && tx == this->tx)
		return true;
	// use provided pins or those used previously
	if (rx == PIN_INVALID)
		rx = this->rx;
	if (tx == PIN_INVALID)
		tx = this->tx;
	// nothing to configure if RX and TX are invalid
	if (rx == PIN_INVALID && tx == PIN_INVALID)
		return false;
	// validate if provided pins are valid for this UART port
	if (!this->validatePins(rx, tx)) {
		LT_E("Serial pin numbers RX=%d,TX=%d invalid for port %u", rx, tx, this->port);
		return false;
	}
	// store in SerialClass for family code to use
	this->rx = rx;
	this->tx = tx;

	// reinitialize Serial with changed pins if already started
	if (this->data) {
		this->endPrivate();
		this->beginPrivate(this->baudrate, this->config);
		this->configure(this->baudrate, this->config);
	}

	return true;
}

void SerialClass::end() {
	this->endPrivate();

	delete this->data;
	delete this->rxBuf;
	this->data	   = nullptr;
	this->rxBuf	   = nullptr;
	this->baudrate = 0;
}

int SerialClass::available() {
	return this->rxBuf ? this->rxBuf->available() : 0;
}

int SerialClass::peek() {
	return this->rxBuf ? this->rxBuf->peek() : -1;
}

int SerialClass::read() {
	return this->rxBuf ? this->rxBuf->read_char() : -1;
}

#if LT_HW_UART0
SerialClass Serial0(0, PIN_SERIAL0_RX, PIN_SERIAL0_TX);
#endif
#if LT_HW_UART1
SerialClass Serial1(1, PIN_SERIAL1_RX, PIN_SERIAL1_TX);
#endif
#if LT_HW_UART2
SerialClass Serial2(2, PIN_SERIAL2_RX, PIN_SERIAL2_TX);
#endif

#endif
