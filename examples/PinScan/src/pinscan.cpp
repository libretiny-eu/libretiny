/* Copyright (c) Kuba Szczodrzyński 2022-07-31. */

#include "pinscan.h"

#if USE_WIFI
#include <WiFi.h>
#include <WiFiMulti.h>

static WiFiMulti wm;
static WiFiServer server(23);
static WiFiClient client;
#endif

Stream *stream		  = NULL;
uint8_t mode[]		  = {'?', '\0', '\0', '\0'};
pin_size_t pinSkip[2] = {255, 255};
int output			  = -1;

static void parseMode();

void setup() {
	Serial.begin(115200);
#if USE_WIFI
	wm.addAP(WIFI_SSID, WIFI_PASS);
	while (wm.run() != WL_CONNECTED) {
		Serial.println("WiFi connection failed, retrying in 5s");
		delay(5000);
	}
	server.begin();
	server.setNoDelay(true);
#else
	stream	   = &Serial;
#if LT_UART_DEFAULT_SERIAL == 0
	output	   = 0;
	pinSkip[0] = PIN_SERIAL0_TX;
	pinSkip[1] = PIN_SERIAL0_RX;
#elif LT_UART_DEFAULT_SERIAL == 1
	output	   = 1;
	pinSkip[0] = PIN_SERIAL1_TX;
	pinSkip[1] = PIN_SERIAL1_RX;
#elif LT_UART_DEFAULT_SERIAL == 2
	output	   = 2;
	pinSkip[0] = PIN_SERIAL2_TX;
	pinSkip[1] = PIN_SERIAL2_RX;
#endif
#endif
}

void loop() {
#if USE_WIFI
	while (wm.run() != WL_CONNECTED) {
		Serial.println("WiFi connection dropped, retrying in 5s");
		delay(5000);
	}
	if (!stream) {
		client = server.accept();
		if (client) {
			stream = &client;
			printHelp();
		}
	} else if (!client.connected()) {
		stream = NULL;
		client.stop();
		client = WiFiClient();
	}
#endif

	if (stream && stream->available()) {
		// put the char in first free mode item
		bool modeSet = false;
		for (uint8_t i = 0; i < sizeof(mode) / sizeof(uint8_t); i++) {
			if (mode[i] == '\0') {
				mode[i] = stream->read();
				// make the next mode show help screen
				if (i < sizeof(mode) - 1) {
					mode[++i] = '?';
				}
				// clear the following items, if any
				while (i < sizeof(mode) - 1) {
					mode[++i] = '\0';
				}
				modeSet = true;
				break;
			}
		}
		if (!modeSet) {
			// no free mode items, reset everything
			memset(mode, '\0', sizeof(mode));
			while (stream->available()) {
				stream->read();
			}
		}
		// LT_I("New mode: %s", mode);
	}

	// check for any 'q' keypresses
	for (uint8_t i = 0; i < sizeof(mode) / sizeof(uint8_t); i++) {
		if (mode[i] == 'q') {
			if (mode[0] == 'd')
				digitalDetach();
			memset(mode, '\0', sizeof(mode));
			mode[0] = '?';
			break;
		}
	}

	if (stream)
		parseMode();
}

static void parseMode() {
	if (mode[0] == '\0')
		return;
	int serial = 0;
	switch (mode[0]) {
		case '?':
			printHelp();
			break;
		case 'd':
			runDigital();
			return;
		case 'a':
			runAnalog();
			return;
		case 'r':
			LT.restart();
			while (1) {}
			return;

#if !USE_WIFI
		case 's':
			stream->print("Choose output UART: ");
			serial = stream->parseInt();
			stream->println();
			// clang-format off
#ifdef PIN_SERIAL0_TX
			if (serial == 0) {
				stream = &Serial0;
				pinSkip[0] = PIN_SERIAL0_TX;
				pinSkip[1] = PIN_SERIAL0_RX;
			} else
#endif
#ifdef PIN_SERIAL1_TX
			if (serial == 1) {
				stream = &Serial1;
				pinSkip[0] = PIN_SERIAL1_TX;
				pinSkip[1] = PIN_SERIAL1_RX;
			} else
#endif
#ifdef PIN_SERIAL2_TX
			if (serial == 2) {
				stream = &Serial2;
				pinSkip[0] = PIN_SERIAL2_TX;
				pinSkip[1] = PIN_SERIAL2_RX;
			} else
#endif
			// clang-format on
			{
				stream->print("Port unavailable");
				break;
			}
			output = serial;
			((SerialClass *)stream)->begin(115200);
			stream->println();
			mode[0] = '?';
			return;
#endif

		case 't':
			ansi = !ansi;
			stream->print("ANSI control codes ");
			if (ansi) {
				stream->println("enabled");
				mode[0]		  = '?';
				ansiSkipErase = true;
				return;
			} else {
				stream->println("disabled");
			}
			break;
		default:
			break;
	}
	mode[0] = '\0';
}
