/* Copyright (c) Kuba Szczodrzyński 2022-05-09. */

#include <api/Stream.h>

class HardwareI2C : public Stream {
  protected:
	int8_t _sda	   = -1;
	int8_t _scl	   = -1;
	uint32_t _freq = 0;

	void (*onRequestCallback)(void);
	void (*onReceiveCallback)(int);

  public:
	bool begin() {
		return begin(_sda, _scl, _freq);
	}

	bool begin(uint8_t address) {
		return begin(address, _sda, _scl, _freq);
	}

	virtual bool setPins(int8_t sda, int8_t scl) = 0;

	virtual bool begin(int8_t sda, int8_t scl, uint32_t frequency = 0)					= 0;
	virtual bool begin(uint8_t address, int8_t sda, int8_t scl, uint32_t frequency = 0) = 0;
	virtual bool end()																	= 0;

	virtual bool setClock(uint32_t freq) = 0;

	virtual void beginTransmission(uint8_t address) = 0;
	virtual uint8_t endTransmission(bool stopBit)	= 0;

	virtual size_t requestFrom(uint8_t address, size_t len, bool stopBit) = 0;

	virtual size_t write(const uint8_t *data, size_t len) = 0;

	virtual int available() = 0;
	virtual int read()		= 0;
	virtual int peek()		= 0;
	virtual void flush()	= 0;

	uint32_t getClock() {
		return _freq;
	}

	uint8_t endTransmission() {
		return endTransmission(true);
	}

	size_t requestFrom(uint8_t address, size_t len) {
		return requestFrom(address, len, true);
	}

	virtual size_t write(uint8_t data) {
		return write(&data, 1);
	}

	void onReceive(void (*cb)(int)) {
		onReceiveCallback = cb;
	}

	void onRequest(void (*cb)(void)) {
		onRequestCallback = cb;
	}
};
