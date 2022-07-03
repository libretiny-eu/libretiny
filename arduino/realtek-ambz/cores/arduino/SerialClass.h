/* Copyright (c) Kuba Szczodrzyński 2022-07-03. */

#pragma once

#include <Arduino.h>
#include <api/HardwareSerial.h>
#include <api/RingBuffer.h>

using namespace arduino;

typedef struct {
	UART_TypeDef *uart;
	RingBuffer *buf;
} SerialData;

class SerialClass : public HardwareSerial {
  private:
	// data accessible to IRQ handler
	SerialData data;
	IRQn irq;
	pin_size_t rx;
	pin_size_t tx;

  public:
	SerialClass(UART_TypeDef *uart, IRQn irq, pin_size_t rx, pin_size_t tx);

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
		return !!data.buf;
	}

	using Print::write;
};

#define HAS_SERIAL_CLASS 1
