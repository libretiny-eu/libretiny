#pragma once

#include <Arduino.h>

class base64 {
  public:
	static String encode(const uint8_t *data, size_t length);
	static String encode(const String &text);
};
