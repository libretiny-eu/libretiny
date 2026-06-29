/* Copyright (c) WGM160P-LibreTiny port 2026-06-11. */
#pragma once

// Visual bring-up indicator for boards that expose an RGB status LED
// (LED_R/LED_G/LED_B from the board variant, e.g. wgm160p-juicebox-40).
// On boards that don't define those macros every call compiles to a no-op,
// so the WiFi backend can signal stages unconditionally.
//
// Color ladder (active-high RGB):
//   RADIO_INIT  red      sl_wfx_init running   (sticky red = radio hung)
//   RADIO_FAIL  magenta  radio / SDIO / netif fault — bring-up bailed
//   IDLE        blue     radio + IP stack up, not joined
//   JOINING     yellow   join command sent, awaiting CONNECT ind + IP
//   JOINED      green    WL_CONNECTED (got IP)
//   JOIN_FAIL   white    join refused / DHCP timeout / link lost during join
//
// It exists for bench observability without SWD: on the silent-radio bench
// unit the board shows red -> magenta, telling us the radio never answered
// the moment we power it.
typedef enum {
	LT_WIFI_LED_OFF = 0,
	LT_WIFI_LED_RADIO_INIT,
	LT_WIFI_LED_RADIO_FAIL,
	LT_WIFI_LED_IDLE,
	LT_WIFI_LED_JOINING,
	LT_WIFI_LED_JOINED,
	LT_WIFI_LED_JOIN_FAIL,
} LtWifiLedStage;

// Drive the RGB status LED to the color for `stage`. Idempotent; the first
// call configures the pins as outputs. Safe to call from the app thread, the
// WiFi event task, and the lwIP tcpip thread: each channel is written with
// digitalWrite(), which lowers to an atomic GPIO set/clear, and the one-time
// pin init always runs first from the app thread (modePriv) before any other
// thread can reach a call site.
//
// When disabled (see ltWifiStatusLedEnable) every call is a no-op.
void ltWifiStatusLed(LtWifiLedStage stage);

// Enable or disable the WiFi backend's use of the RGB status LED at runtime.
// Enabled by default. Pass false to release LED_R/LED_G/LED_B so the
// application can own them (e.g. an EVSE charge-state indicator): once
// disabled, ltWifiStatusLed() touches no GPIO until re-enabled, leaving the
// pins for the app to drive with pinMode()/digitalWrite(). Re-enabling forces
// the next ltWifiStatusLed() call to reconfigure the channels as outputs.
//
// Call from the app thread before handing the LEDs off. WiFi LED writes only
// happen on connection-state changes (infrequent), so a disable issued before
// the app first drives the LEDs reliably wins; there is no need to coordinate
// with the WiFi event task. On boards without LED_R/LED_G/LED_B this is a
// no-op.
void ltWifiStatusLedEnable(bool enabled);
