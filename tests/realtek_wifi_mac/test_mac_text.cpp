/* Copyright (c) LibreTiny contributors. */

// Host-side test for the helpers that prepare a MAC override for the Realtek
// driver. Build and run:
//   g++ -std=c++11 -Wall -Wextra -I cores/realtek-amb/arduino/libraries/WiFi
//       tests/realtek_wifi_mac/test_mac_text.cpp -o test_mac_text && ./test_mac_text

#include "WiFiMacText.h"

#include <cstdio>
#include <cstdlib>
#include <cstring>

static int failures = 0;

#define CHECK(condition)                                                        \
	do {                                                                        \
		if (!(condition)) {                                                     \
			std::fprintf(stderr, "FAILED line %d: %s\n", __LINE__, #condition); \
			failures++;                                                         \
		}                                                                       \
	} while (0)

static void testAssignable() {
	const uint8_t unicast[6]	 = {0x02, 0x11, 0x22, 0x33, 0x44, 0x55};
	const uint8_t leadingZero[6] = {0x00, 0xe0, 0x4c, 0xb7, 0x23, 0x01};
	const uint8_t multicast[6]	 = {0x01, 0x00, 0x5e, 0x00, 0x00, 0x01};
	const uint8_t zero[6]		 = {0, 0, 0, 0, 0, 0};
	const uint8_t broadcast[6]	 = {0xff, 0xff, 0xff, 0xff, 0xff, 0xff};

	CHECK(wifiMacIsAssignable(unicast));
	CHECK(wifiMacIsAssignable(leadingZero));
	CHECK(!wifiMacIsAssignable(multicast));
	CHECK(!wifiMacIsAssignable(zero));
	CHECK(!wifiMacIsAssignable(broadcast));
	CHECK(!wifiMacIsAssignable(nullptr));
}

static void testFormat() {
	char text[WIFI_MAC_TEXT_SIZE];

	// A leading zero byte must not truncate the text (it did when raw bytes
	// were passed to the SDK).
	const uint8_t withZero[6] = {0x00, 0xe0, 0x4c, 0xb7, 0x23, 0x00};
	wifiMacToText(withZero, text);
	CHECK(std::strcmp(text, "00:e0:4c:b7:23:00") == 0);

	const uint8_t mixed[6] = {0x02, 0xAB, 0x0f, 0xf0, 0x9c, 0x5d};
	wifiMacToText(mixed, text);
	CHECK(std::strcmp(text, "02:ab:0f:f0:9c:5d") == 0);
	CHECK(std::strlen(text) == WIFI_MAC_TEXT_SIZE - 1);
}

// Mirrors how the driver reads the text: pairs at offsets 0, 3, 6, ... 15.
static void testRoundTrip() {
	auto hex				  = [](char c) { return c <= '9' ? c - '0' : c - 'a' + 10; };
	const uint8_t original[6] = {0x06, 0x7a, 0x00, 0xff, 0x10, 0x01};
	char text[WIFI_MAC_TEXT_SIZE];
	wifiMacToText(original, text);

	uint8_t parsed[6];
	for (unsigned i = 0; i < 6; i++) {
		parsed[i] = (hex(text[3 * i]) << 4) | hex(text[3 * i + 1]);
		if (i < 5)
			CHECK(text[3 * i + 2] == ':');
	}
	CHECK(std::memcmp(original, parsed, 6) == 0);
}

int main() {
	testAssignable();
	testFormat();
	testRoundTrip();
	if (failures) {
		std::fprintf(stderr, "%d check(s) failed\n", failures);
		return EXIT_FAILURE;
	}
	std::puts("all MAC text checks passed");
	return EXIT_SUCCESS;
}
