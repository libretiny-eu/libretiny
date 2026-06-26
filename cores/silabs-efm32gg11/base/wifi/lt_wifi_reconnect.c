/* Copyright (c) WGM160P-LibreTiny port 2026-06-17. */
#include "lt_wifi_reconnect.h"

lt_wifi_rc_action_t lt_wifi_rc_decide(const lt_wifi_rc_input_t *in) {
	// Act only on a connection that was established at least once (armed) and is
	// still wanted (enabled, with a target SSID). A never-succeeded join stays
	// the app's call via begin()'s return — we never loop it.
	if (!in->enabled || !in->armed || !in->have_ssid)
		return LT_WIFI_RC_ACTION_NONE;

	switch (in->status) {
		case LT_WIFI_RC_CONNECTED:
			// Primary loss detection is event-driven (DISCONNECT_IND / DHCP loss).
			// This is the fallback: if the link went silently dead and the probe
			// has reported bad for long enough, demote into the retry path.
			if (in->health_bad_ms >= LT_WIFI_RC_HEALTH_TIMEOUT_MS)
				return LT_WIFI_RC_ACTION_DEMOTE;
			return LT_WIFI_RC_ACTION_NONE;

		case LT_WIFI_RC_IDLE:
			// A supervisor-fired join is in flight. If CONNECT_IND/GOT_IP never
			// arrives within the window, abandon it so the next tick retries.
			// (last_join_ms == 0 means no join recorded yet — leave it alone.)
			if (in->last_join_ms != 0 && (in->now_ms - in->last_join_ms) >= LT_WIFI_RC_JOIN_TIMEOUT_MS)
				return LT_WIFI_RC_ACTION_DEMOTE;
			return LT_WIFI_RC_ACTION_NONE;

		case LT_WIFI_RC_LOST:
		case LT_WIFI_RC_FAILED:
		case LT_WIFI_RC_DISCONNECTED:
			// Retryable. First attempt (last_join_ms == 0) fires immediately;
			// later attempts wait out the backoff window.
			if (in->last_join_ms != 0 && (in->now_ms - in->last_join_ms) < in->backoff_ms)
				return LT_WIFI_RC_ACTION_NONE;
			return LT_WIFI_RC_ACTION_JOIN;

		default:
			return LT_WIFI_RC_ACTION_NONE;
	}
}

uint32_t lt_wifi_rc_next_backoff(uint32_t cur) {
	if (cur == 0)
		return LT_WIFI_RC_MIN_BACKOFF_MS;
	uint32_t next = cur * 2;
	return next < LT_WIFI_RC_MAX_BACKOFF_MS ? next : LT_WIFI_RC_MAX_BACKOFF_MS;
}
