/* Copyright (c) Kuba Szczodrzyński 2022-05-08. */

#pragma once

#include <api/RingBuffer.h>
#include <api/Wire.h>

#ifdef __cplusplus
extern "C" {
#endif
// #include <i2c_api.h>
#ifdef __cplusplus
}
#endif

#if !defined(PIN_WIRE0_SDA) && defined(PIN_WIRE1_SDA)
#define Wire1 Wire
#endif

#define WIRE_HAS_END	  1
#define WIRE_DEFAULT_FREQ 100000

struct i2c_s;
typedef struct i2c_s i2c_t;

using arduino::RingBuffer;

class TwoWire : public ITwoWire {
  private:
	i2c_t *_i2c	 = NULL;
	uint8_t _idx = 0;

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

	using ITwoWire::begin;
	using ITwoWire::endTransmission;
	using ITwoWire::requestFrom;
	using ITwoWire::write;
	using Print::write;
};

#ifdef PIN_WIRE0_SDA
extern TwoWire Wire;
#endif
#ifdef PIN_WIRE1_SDA
extern TwoWire Wire1;
#endif
