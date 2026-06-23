/* Copyright (c) WGM160P-LibreTiny port 2026-06-11. */

#include "WiFiStatusLed.h"

#include <Arduino.h>

#if defined(LED_R) && defined(LED_G) && defined(LED_B)

// Runtime gate. Written from the app thread (ltWifiStatusLedEnable), read from
// every ltWifiStatusLed() call site. Single-byte access is atomic on Cortex-M4,
// so no lock is needed; volatile keeps the read at each call site.
// Compile-time default is enabled (upstream behavior). An application that owns the RGB LED
// itself (e.g. the OpenEVSE-lite EVSE-state indicator) can default this OFF with
// -D LT_WIFI_STATUS_LED_DEFAULT=0, so the WiFi ladder never touches the LED at all — not even
// during boot/WiFi-connect, before the app's runtime ltWifiStatusLedEnable(false) would run.
#ifndef LT_WIFI_STATUS_LED_DEFAULT
#define LT_WIFI_STATUS_LED_DEFAULT 1
#endif
static volatile bool s_enabled = LT_WIFI_STATUS_LED_DEFAULT;
static bool inited			   = false;

void ltWifiStatusLedEnable(bool enabled) {
	s_enabled = enabled;
	// Force pin re-init on the next enabled call so handing the LEDs back from
	// the application (which may have changed their mode) restores outputs.
	if (!enabled)
		inited = false;
}

void ltWifiStatusLed(LtWifiLedStage stage) {
	// Disabled: the application owns the LEDs — touch no GPIO.
	if (!s_enabled)
		return;

	// First call (always from the app thread via modePriv) configures the
	// channels; later calls only flip the atomic GPIO set/clear registers.
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

void ltWifiStatusLedEnable(bool enabled) {
	(void)enabled;
}

#endif
