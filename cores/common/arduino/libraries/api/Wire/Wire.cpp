/* Copyright (c) Kuba Szczodrzyński 2023-09-21. */

#if LT_ARD_HAS_WIRE || DOXYGEN

#include "Wire.h"

TwoWire::~TwoWire() {}

template <typename C, typename T>
static bool contains(C &&c, T e) {
	for (auto x : c) {
		if (x == e)
			return true;
	}
	return false;
};

int TwoWire::getPortByPins(pin_size_t sda, pin_size_t scl) {
	if (sda == PIN_INVALID || scl == PIN_INVALID)
		return -1;
#if defined(PINS_WIRE0_SDA) && defined(PINS_WIRE0_SCL)
	if (contains(PINS_WIRE0_SDA, sda) && contains(PINS_WIRE0_SCL, scl))
		return 0;
#endif
#if defined(PINS_WIRE1_SDA) && defined(PINS_WIRE1_SCL)
	if (contains(PINS_WIRE1_SDA, sda) && contains(PINS_WIRE1_SCL, scl))
		return 1;
#endif
#if defined(PINS_WIRE2_SDA) && defined(PINS_WIRE2_SCL)
	if (contains(PINS_WIRE2_SDA, sda) && contains(PINS_WIRE2_SCL, scl))
		return 2;
#endif
	return -1;
}

bool TwoWire::setPinsPrivate(pin_size_t sda, pin_size_t scl) {
	// default to already set pins
	if (sda == PIN_INVALID)
		sda = this->sda;
	if (scl == PIN_INVALID)
		scl = this->scl;

	if (sda == PIN_INVALID && scl == PIN_INVALID) {
		// set pins by port number
#if defined(PINS_WIRE0_SDA) && defined(PINS_WIRE0_SCL)
		if (this->port == 0) {
			this->sda = PINS_WIRE0_SDA[0];
			this->scl = PINS_WIRE0_SCL[0];
			return true;
		}
#endif
#if defined(PINS_WIRE1_SDA) && defined(PINS_WIRE1_SCL)
		if (this->port == 1) {
			this->sda = PINS_WIRE1_SDA[0];
			this->scl = PINS_WIRE1_SCL[0];
			return true;
		}
#endif
#if defined(PINS_WIRE2_SDA) && defined(PINS_WIRE2_SCL)
		if (this->port == 2) {
			this->sda = PINS_WIRE2_SDA[0];
			this->scl = PINS_WIRE2_SCL[0];
			return true;
		}
#endif
	}

	// set port number by specified pins
	int port = this->getPortByPins(sda, scl);
	if (port == -1)
		// no such port
		return false;
	if (this->fixedPort && port != this->port)
		// allow only same port for fixed port instances
		return false;
	this->port = port;
	this->sda  = sda;
	this->scl  = scl;
	return true;
}

bool TwoWire::setPins(pin_size_t sda, pin_size_t scl) {
	if (!this->data)
		return this->setPinsPrivate(sda, scl);
	if (this->address != 0x00)
		return this->begin(this->address, sda, scl, this->frequency);
	else
		return this->begin(sda, scl, this->frequency);
}

void TwoWire::beginTransmission(uint16_t address) {
	if (!this->txBuf || this->address != 0x00)
		// cannot begin if not started in master mode
		return;
	this->txBuf->clear();
	this->txAddress = address;
}

size_t TwoWire::write(uint8_t data) {
	if (!this->txBuf || this->txBuf->isFull())
		return 0;
	this->txBuf->store_char(data);
	return 1;
}

size_t TwoWire::write(const uint8_t *data, size_t len) {
	for (size_t i = 0; i < len; i++) {
		if (!this->write(data[i])) {
			return i;
		}
	}
	return len;
}

void TwoWire::flush() {
	if (!this->txBuf)
		return;
	this->txBuf->clear();
}

int TwoWire::available() {
	return this->rxBuf ? this->rxBuf->available() : 0;
}

int TwoWire::peek() {
	return this->rxBuf ? this->rxBuf->peek() : -1;
}

int TwoWire::read() {
	return this->rxBuf ? this->rxBuf->read_char() : -1;
}

#if LT_HW_I2C0
TwoWire Wire(0);
#elif LT_HW_I2C1
TwoWire Wire(1);
#elif LT_HW_I2C2
TwoWire Wire(2);
#endif

#if LT_HW_I2C0
TwoWire Wire0(WIRE_PORT_FIXED_BIT | 0);
#endif
#if LT_HW_I2C1
TwoWire Wire1(WIRE_PORT_FIXED_BIT | 1);
#endif
#if LT_HW_I2C2
TwoWire Wire2(WIRE_PORT_FIXED_BIT | 2);
#endif

#endif
