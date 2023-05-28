/* Copyright (c) Kuba Szczodrzyński 2022-06-23. */

#pragma once

#include <Arduino.h>
#include <api/HardwareSerial.h>
#include <api/RingBuffer.h>

using namespace arduino;

#if HAS_SERIAL0
#ifndef PIN_SERIAL0_RX
#define PIN_SERIAL0_RX PIN_INVALID
#endif
#ifndef PIN_SERIAL0_TX
#define PIN_SERIAL0_TX PIN_INVALID
#endif
#endif
#if HAS_SERIAL1
#ifndef PIN_SERIAL1_RX
#define PIN_SERIAL1_RX PIN_INVALID
#endif
#ifndef PIN_SERIAL1_TX
#define PIN_SERIAL1_TX PIN_INVALID
#endif
#endif
#if HAS_SERIAL2
#ifndef PIN_SERIAL2_RX
#define PIN_SERIAL2_RX PIN_INVALID
#endif
#ifndef PIN_SERIAL2_TX
#define PIN_SERIAL2_TX PIN_INVALID
#endif
#endif

class SerialClass : public HardwareSerial {
  private:
	uint32_t port;
	pin_size_t rx;
	pin_size_t tx;

  public:
	void *data;

  private:
	RingBuffer *buf;
	uint32_t baudrate;
	uint16_t config;

  public:
	SerialClass(uint32_t port, pin_size_t rx = PIN_INVALID, pin_size_t tx = PIN_INVALID);

	inline void begin(unsigned long baudrate) {
		begin(baudrate, SERIAL_8N1);
	}

	inline void configure(unsigned long baudrate) {
		configure(baudrate, SERIAL_8N1);
	}

	void begin(unsigned long baudrate, uint16_t config);
	void configure(unsigned long baudrate, uint16_t config);
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
