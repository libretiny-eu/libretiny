/* Copyright (c) LibreTiny contributors. */

// Host-side test for reading the MAC from the vendor's "CALD" flash sector.
// Build and run:
//   g++ -std=c++11 -Wall -Wextra -I cores/realtek-ambz2/base
//       tests/realtek_wifi_mac/test_cald.cpp -o test_cald && ./test_cald

#include "lt_cald.h"

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

// First bytes of the sector as found on a Broadlink RM4 Pro.
static const uint8_t broadlinkHeader[LT_CALD_HEADER_SIZE] =
	{'C', 'A', 'L', 'D', 0xff, 0xff, 0xff, 0xff, 0xe8, 0x70, 0x72, 0xba, 0x6b, 0x16};

static void testValidSector() {
	uint8_t mac[6]			  = {};
	const uint8_t expected[6] = {0xe8, 0x70, 0x72, 0xba, 0x6b, 0x16};
	CHECK(lt_cald_parse_mac(broadlinkHeader, mac));
	CHECK(std::memcmp(mac, expected, 6) == 0);
}

static void testNoMagic() {
	uint8_t header[LT_CALD_HEADER_SIZE];
	uint8_t mac[6] = {};

	// erased flash
	std::memset(header, 0xff, sizeof(header));
	CHECK(!lt_cald_parse_mac(header, mac));

	// any other content, e.g. a bootloader that replaced the sector
	std::memcpy(header, broadlinkHeader, sizeof(header));
	header[0] = 0x99;
	CHECK(!lt_cald_parse_mac(header, mac));
}

static void testUnusableMac() {
	uint8_t header[LT_CALD_HEADER_SIZE];
	uint8_t mac[6] = {};

	std::memcpy(header, broadlinkHeader, sizeof(header));
	std::memset(header + LT_CALD_MAC_OFFSET, 0x00, 6);
	CHECK(!lt_cald_parse_mac(header, mac));

	std::memset(header + LT_CALD_MAC_OFFSET, 0xff, 6);
	CHECK(!lt_cald_parse_mac(header, mac));

	std::memcpy(header, broadlinkHeader, sizeof(header));
	header[LT_CALD_MAC_OFFSET] = 0x01; // multicast
	CHECK(!lt_cald_parse_mac(header, mac));
}

static void testOutputUntouchedOnFailure() {
	uint8_t header[LT_CALD_HEADER_SIZE];
	uint8_t mac[6] = {1, 2, 3, 4, 5, 6};
	std::memset(header, 0xff, sizeof(header));
	CHECK(!lt_cald_parse_mac(header, mac));
	const uint8_t untouched[6] = {1, 2, 3, 4, 5, 6};
	CHECK(std::memcmp(mac, untouched, 6) == 0);
}

int main() {
	testValidSector();
	testNoMagic();
	testUnusableMac();
	testOutputUntouchedOnFailure();
	if (failures) {
		std::fprintf(stderr, "%d check(s) failed\n", failures);
		return EXIT_FAILURE;
	}
	std::puts("all CALD checks passed");
	return EXIT_SUCCESS;
}
