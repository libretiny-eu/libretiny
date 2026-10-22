/* Copyright (c) Etienne Le Cousin 2025-01-19. */

#pragma once

#include <Arduino.h>
#include <HardwareI2C.h>
#include <api/RingBuffer.h>

#define Wire1 Wire

#define WIRE_HAS_END	  1
#define WIRE_DEFAULT_FREQ 100000

#ifndef I2C_PRIV
#define I2C_PRIV void
#endif

using arduino::RingBuffer;

class TwoWire : public HardwareI2C {
  private:
	I2C_PRIV *_i2c = NULL;

	RingBuffer _rxBuf;
	RingBuffer _txBuf;
	uint8_t _txAddr = 0;
	bool _inSetPins = false;

  public:
	TwoWire();
	TwoWire(int8_t sda, int8_t scl);
	~TwoWire();

	bool setPins(int8_t sda, int8_t scl);

	bool begin(int8_t sda, int8_t scl, uint32_t frequency = 0);
	bool begin(uint8_t address, int8_t sda, int8_t scl, uint32_t frequency = 0);
	bool end();

	bool setClock(uint32_t freq);

	void beginTransmission(uint8_t address);
	uint8_t endTransmission(bool stopBit);

	size_t requestFrom(uint8_t address, size_t len, bool stopBit);
	size_t write(uint8_t data);
	size_t write(const uint8_t *data, size_t len);

	int available();
	int read();
	int peek();
	void flush();

	using HardwareI2C::begin;
	using HardwareI2C::endTransmission;
	using HardwareI2C::requestFrom;
	using HardwareI2C::write;
	using Print::write;
};

#ifdef PIN_WIRE0_SDA
extern TwoWire Wire;
#endif
#ifdef PIN_WIRE1_SDA
extern TwoWire Wire1;
#endif
