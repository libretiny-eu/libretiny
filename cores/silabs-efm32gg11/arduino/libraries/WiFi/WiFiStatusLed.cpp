/* Copyright (c) WGM160P-LibreTiny port 2026-06-11. */

#include "WiFiStatusLed.h"

#include <Arduino.h>

#if defined(LED_R) && defined(LED_G) && defined(LED_B)

void ltWifiStatusLed(LtWifiLedStage stage) {
	// First call (always from the app thread via modePriv) configures the
	// channels; later calls only flip the atomic GPIO set/clear registers.
	static bool inited = false;
	if (!inited) {
		pinMode(LED_R, OUTPUT);
		pinMode(LED_G, OUTPUT);
		pinMode(LED_B, OUTPUT);
		inited = true;
	}

	bool r = false, g = false, b = false;
	switch (stage) {
		case LT_WIFI_LED_RADIO_INIT:
			r = true;
			break; // red
		case LT_WIFI_LED_RADIO_FAIL:
			r = true;
			b = true;
			break; // magenta
		case LT_WIFI_LED_IDLE:
			b = true;
			break; // blue
		case LT_WIFI_LED_JOINING:
			r = true;
			g = true;
			break; // yellow
		case LT_WIFI_LED_JOINED:
			g = true;
			break; // green
		case LT_WIFI_LED_JOIN_FAIL:
			r = true;
			g = true;
			b = true;
			break; // white
		case LT_WIFI_LED_OFF:
		default:
			break; // off
	}

	digitalWrite(LED_R, r ? HIGH : LOW);
	digitalWrite(LED_G, g ? HIGH : LOW);
	digitalWrite(LED_B, b ? HIGH : LOW);
}

#else // board has no RGB status LED — every call is a no-op

void ltWifiStatusLed(LtWifiLedStage stage) {
	(void)stage;
}

#endif
