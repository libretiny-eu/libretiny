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
void ltWifiStatusLed(LtWifiLedStage stage);
