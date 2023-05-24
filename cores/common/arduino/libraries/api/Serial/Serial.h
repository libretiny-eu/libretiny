/* Copyright (c) Kuba Szczodrzyński 2022-06-23. */

#pragma once

#include <Arduino.h>
#include <api/HardwareSerial.h>
#include <api/RingBuffer.h>

using namespace arduino;

class SerialClass : public HardwareSerial {
  private:
	uint32_t port;
	RingBuffer *buf;

  public:
	void *data;

  public:
	SerialClass(uint32_t port);

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

  public:
#if LT_AUTO_DOWNLOAD_REBOOT
	static void adrParse(uint8_t c);
#endif

	using Print::write;
};

#define HAS_SERIAL_CLASS 1
