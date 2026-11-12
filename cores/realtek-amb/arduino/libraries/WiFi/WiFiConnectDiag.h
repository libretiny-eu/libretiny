/* Copyright (c) Kuba Szczodrzyński 2022-04-25. */

#pragma once

#include <stdbool.h>

/*
 * Ameba-1 station join policy.
 *
 * The known-working vendor firmware joins with the plain call:
 *
 *   wifi_connect(ssid, security, password,
 *                strlen(ssid), strlen(password), 0, NULL)
 *
 * It does not send wext_set_ssid("-") first, does not pass a BSSID and does
 * not configure a partial scan. Mirror that exactly: a BSSID argument from
 * WiFi.begin() is accepted for API compatibility but ignored on this family.
 *
 * Pure constants/inline logic so the host regression test can compile this
 * header directly.
 */

#define LT_AMB1_JOIN_KEY_ID 0

enum {
	LT_AMB1_JOIN_USES_BSSID = 0, /* never call wifi_connect_bssid() */
	LT_AMB1_JOIN_USES_PSCAN = 0, /* never call wifi_set_pscan_chan() */
};

static inline const char *lt_amb1_path_name(void) {
	return "plain";
}