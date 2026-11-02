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

static void testFromText() {
	const uint8_t expected[6] = {0xec, 0x0b, 0xae, 0x9e, 0x21, 0xc1};
	uint8_t mac[6]			  = {};

	// the form used by the LT_WIFI_STA_MAC build option
	CHECK(wifiMacFromText("ec0bae9e21c1", mac));
	CHECK(std::memcmp(mac, expected, 6) == 0);

	std::memset(mac, 0, 6);
	CHECK(wifiMacFromText("EC:0B:AE:9E:21:C1", mac));
	CHECK(std::memcmp(mac, expected, 6) == 0);

	// a stringified macro may arrive with quotes
	std::memset(mac, 0, 6);
	CHECK(wifiMacFromText("\"ec-0b-ae-9e-21-c1\"", mac));
	CHECK(std::memcmp(mac, expected, 6) == 0);

	// a leading zero byte must survive
	CHECK(wifiMacFromText("00e04cb72300", mac));
	CHECK(mac[0] == 0x00 && mac[1] == 0xe0 && mac[5] == 0x00);
}

static void testFromTextRejects() {
	uint8_t mac[6]			   = {1, 2, 3, 4, 5, 6};
	const uint8_t untouched[6] = {1, 2, 3, 4, 5, 6};

	CHECK(!wifiMacFromText(nullptr, mac));
	CHECK(!wifiMacFromText("", mac));
	CHECK(!wifiMacFromText("ec0bae9e21c", mac));   // 11 digits
	CHECK(!wifiMacFromText("ec0bae9e21c1f", mac)); // 13 digits
	CHECK(!wifiMacFromText("ec0bae9e21cg", mac));  // not hex
	CHECK(!wifiMacFromText("ec0b ae9e21c1", mac)); // space
	CHECK(std::memcmp(mac, untouched, 6) == 0);
}

static void testFromBuildText() {
	uint8_t mac[6] = {};

	// same as wifiMacFromText() for ordinary values
	const uint8_t expected[6] = {0xec, 0x0b, 0xae, 0x9e, 0x21, 0xc1};
	CHECK(wifiMacFromBuildText("ec0bae9e21c1", mac));
	CHECK(std::memcmp(mac, expected, 6) == 0);
	CHECK(wifiMacFromBuildText("ec:0b:ae:9e:21:c1", mac));
	CHECK(std::memcmp(mac, expected, 6) == 0);

	// a build tool turned 020000000001 into the number 20000000001
	const uint8_t digitsOnly[6] = {0x02, 0x00, 0x00, 0x00, 0x00, 0x01};
	CHECK(wifiMacFromBuildText("20000000001", mac));
	CHECK(std::memcmp(mac, digitsOnly, 6) == 0);
	CHECK(wifiMacFromBuildText("020000000001", mac));
	CHECK(std::memcmp(mac, digitsOnly, 6) == 0);

	// all twelve digits present, nothing to restore
	const uint8_t full[6] = {0x12, 0x34, 0x56, 0x78, 0x90, 0x12};
	CHECK(wifiMacFromBuildText("123456789012", mac));
	CHECK(std::memcmp(mac, full, 6) == 0);

	// still rejected
	CHECK(!wifiMacFromBuildText(nullptr, mac));
	CHECK(!wifiMacFromBuildText("", mac));
	CHECK(!wifiMacFromBuildText("1234567890123", mac));
	CHECK(!wifiMacFromBuildText("ec0bae9e21c", mac)); // short, but not only digits
}

int main() {
	testAssignable();
	testFromBuildText();
	testFromText();
	testFromTextRejects();
	testFormat();
	testRoundTrip();
	if (failures) {
		std::fprintf(stderr, "%d check(s) failed\n", failures);
		return EXIT_FAILURE;
	}
	std::puts("all MAC text checks passed");
	return EXIT_SUCCESS;
}
