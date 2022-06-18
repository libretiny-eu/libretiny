/* Copyright (c) Kuba Szczodrzyński 2022-05-08. */

#include "Wire.h"

#include <Arduino.h>

extern "C" {
#include <i2c_api.h>
extern int i2c_write_timeout(i2c_t *obj, int address, char *data, int length, int stop, int timeout_ms);
}

#ifdef PIN_WIRE0_SDA
// Wire object associated to I2C0 interface.
TwoWire Wire(PIN_WIRE0_SDA, PIN_WIRE0_SCL);
#endif

#if defined(PIN_WIRE0_SDA) && defined(PIN_WIRE1_SDA)
// Wire object associated to I2C1 interface.
TwoWire Wire1(PIN_WIRE1_SDA, PIN_WIRE1_SCL);
#endif

#if !defined(PIN_WIRE0_SDA) && defined(PIN_WIRE1_SDA)
// Wire object associated to I2C1 interface. The board doesn't support I2C0.
TwoWire Wire(PIN_WIRE1_SDA, PIN_WIRE1_SCL);
#endif

TwoWire::TwoWire() {
	_timeout = 50;
}

TwoWire::TwoWire(int8_t sda, int8_t scl) {
	_timeout = 50;
	_sda	 = sda;
	_scl	 = scl;
}

TwoWire::~TwoWire() {}

bool TwoWire::setPins(int8_t sda, int8_t scl) {
	// return true when changing pins on initialized I2C
	if (_inSetPins)
		return true;
	// check if pins are provided
	if (sda == -1 || scl == -1)
		return false;
	// set private pins
	_sda = sda;
	_scl = scl;
	sda	 = pinTable[sda].gpio;
	scl	 = pinTable[scl].gpio;

	// check if pins are valid
	if ((sda == PA_4 || sda == PA_19 || sda == PA_30) && (scl == PA_1 || scl == PA_22 || scl == PA_29)) {
		// I2C index 0
		_idx = 0;
	} else if ((sda == PA_27 || sda == PA_2 || sda == PA_23) && (scl == PA_28 || scl == PA_3 || scl == PA_18)) {
		// I2C index 1
		_idx = 1;
	} else {
		return false;
	}

	// restart I2C if changing pins
	// this will never be called from begin()
	if (_i2c) {
		_inSetPins = true;
		end();
		begin();
		_inSetPins = false;
	}
	return true;
}

bool TwoWire::begin(int8_t sda, int8_t scl, uint32_t frequency) {
	if (_i2c)
		return true;
	// set private i2c pins
	if (!setPins(sda, scl))
		return false;
	// use default frequency
	if (!frequency)
		frequency = WIRE_DEFAULT_FREQ;

	_i2c = new i2c_t;
	i2c_init(_i2c, (PinName)pinTable[_sda].gpio, (PinName)pinTable[_scl].gpio);
	i2c_frequency(_i2c, frequency);
	_freq = frequency;
	return true;
}

bool TwoWire::begin(uint8_t address, int8_t sda, int8_t scl, uint32_t frequency) {
	if (_i2c)
		return true;
	// init master bus first, return if failed (wrong pins)
	if (!begin(sda, scl, frequency))
		return false;

	i2c_slave_address(_i2c, _idx, address, 0xff);
	i2c_slave_mode(_i2c, true);
	return true;
}

bool TwoWire::end() {
	i2c_reset(_i2c);
	delete _i2c;
	_i2c = NULL;
}

bool TwoWire::setClock(uint32_t freq) {
	if (_i2c) {
		i2c_frequency(_i2c, freq);
	}
	_freq = freq;
}

void TwoWire::beginTransmission(uint8_t address) {
	_txAddr = address;
	_txBuf.clear();
}

// Errors:
//  0 : Success
//  1 : Data too long
//  2 : NACK on transmit of address
//  3 : NACK on transmit of data
//  4 : Other error
uint8_t TwoWire::endTransmission(bool stopBit) {
	if (!_i2c || !_txAddr)
		return 4;
	char *buf = (char *)malloc(_txBuf.available());
	uint8_t i = 0;
	while (_txBuf.available()) {
		buf[i++] = _txBuf.read_char();
	}
	int len = i2c_write_timeout(_i2c, _txAddr, buf, i, stopBit, _timeout);
	free(buf);
	_txAddr = 0;
	if (len == -1)
		return 2; // slave not available (if tx length == 0)
	if (len != i)
		return 3; // less bytes written
	return 0;
}

size_t TwoWire::requestFrom(uint8_t address, size_t len, bool stopBit) {
	if (!len)
		return 0;

	if (len > SERIAL_BUFFER_SIZE)
		len = SERIAL_BUFFER_SIZE;

	_rxBuf.clear();

	char *buf = (char *)malloc(_txBuf.available());
	i2c_read(_i2c, address, buf, len, stopBit);
	uint8_t i = 0;
	while (len) {
		_rxBuf.store_char(buf[i++]);
		len--;
	}
	free(buf);
	return len;
}

size_t TwoWire::write(uint8_t data) {
	if (!_txAddr || _txBuf.isFull())
		return 0;
	_txBuf.store_char(data);
	return 1;
}

size_t TwoWire::write(const uint8_t *data, size_t len) {
	for (size_t i = 0; i < len; i++) {
		if (!write(data[i]))
			return i;
	}
	return len;
}

int TwoWire::available() {
	return _rxBuf.available();
}

int TwoWire::read() {
	return _rxBuf.read_char();
}

int TwoWire::peek() {
	return _rxBuf.peek();
}

void TwoWire::flush() {}
