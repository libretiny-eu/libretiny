#pragma once

#include <Arduino.h>
#include <stdlib.h>

class LwIPRxBuffer {
  private:
	size_t _size;
	uint8_t *_buffer;
	size_t _pos;
	size_t _fill;
	int _sock;
	bool _failed;
	size_t r_available();
	size_t fillBuffer();

  public:
	LwIPRxBuffer(int sock, size_t size = 1436);
	~LwIPRxBuffer();
	bool failed();
	int read(uint8_t *dst, size_t len);
	int peek();
	size_t available();
};
