/* Copyright (c) Kuba Szczodrzyński 2022-06-23. */

#pragma once

#include <api/ArduinoAPI.h>
#include <api/HardwareSerial.h>
#include <api/RingBuffer.h>

using namespace arduino;

class SerialClass : public HardwareSerial {
  private:
	uint8_t port;
	RingBuffer *buf;

  public:
	SerialClass(uint8_t port);

	inline void begin(unsigned long baudrate) {
		begin(baudrate, SERIAL_8N1);
	}

	void begin(unsigned long baudrate, uint16_t config);
	void end();
	int available();
	int peek();
	int read();
	void flush();
	size_t write(uint8_t c);

	operator bool() {
		return !!buf;
	}

	using Print::write;
};
