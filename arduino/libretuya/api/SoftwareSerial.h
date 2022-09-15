/* Copyright (c) Kuba Szczodrzyński 2022-07-03. */

#pragma once

#ifdef LT_ARD_HAS_SOFTSERIAL

#include <Arduino.h>
#include <api/HardwareSerial.h>
#include <api/RingBuffer.h>

using namespace arduino;

typedef enum {
	SS_IDLE = 0,
	SS_START,
	SS_DATA0,
	SS_DATA1,
	SS_DATA2,
	SS_DATA3,
	SS_DATA4,
	SS_DATA5,
	SS_DATA6,
	SS_DATA7,
	SS_STOP,
	SS_END,
} SoftState;

typedef struct {
	SoftState state;
	RingBuffer *buf;
	uint8_t byte;
	pin_size_t pin;
	void *param;
} SoftData;

typedef struct {
	SoftData rx;
	SoftData tx;
	uint8_t invert;
	void *param;
} SoftSerial;

class SoftwareSerial : public HardwareSerial {
  private:
	SoftSerial data;
	void *param;

  public:
	SoftwareSerial(pin_size_t receivePin, pin_size_t transmitPin, bool inverted = false);

	inline void begin(unsigned long baudrate) {
		begin(baudrate, SERIAL_8N1);
	}

	int available();
	int peek();
	int read();
	void flush();
	size_t write(uint8_t c);

	operator bool() {
		return data.rx.buf || data.tx.buf;
	}

  public: // Family needs to implement these methods only
	void begin(unsigned long baudrate, uint16_t config);
	void end();

  private:
	void startTx();
	void endTx();

	using Print::write;
};

#endif
