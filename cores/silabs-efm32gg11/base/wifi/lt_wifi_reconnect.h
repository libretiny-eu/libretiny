/* Copyright (c) WGM160P-LibreTiny port 2026-06-17. */
/* lt_wifi_reconnect.h — pure decision logic for the STA auto-reconnect
 * supervisor.
 *
 * No hardware / FreeRTOS / lwIP dependencies, so the decision is
 * host-unit-testable (see cores/silabs-efm32gg11/test/wifi). The C++ wrapper
 * (arduino/libraries/WiFi/WiFiEvents.cpp:ltWifiReconnectTick) supplies the
 * inputs each tick — status, timers, and a link-health signal — and carries
 * out the returned action (fire a join command / mark the link lost).
 *
 * Why a link-health input at all: the WF200 FMAC has no autonomous rejoin, so
 * the backend supervises it. The event-driven path (an unsolicited
 * SL_WFX_DISCONNECT_IND_ID, or DHCP zeroing the IP) is primary. But if the AP
 * vanishes and the firmware never emits a disconnect indication, staStatus
 * stays WL_CONNECTED forever and nothing ever retries. The health-bad timer is
 * the fallback that demotes such a silently-dead link into the retry path.
 */
#pragma once

#include <stdbool.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

// Mirrors the WiFiStatus values from
// cores/common/arduino/libraries/api/WiFi/WiFiType.h. Kept as local constants
// so the host tests need not pull in the whole Arduino WiFi API. MUST stay in
// sync with that enum.
#define LT_WIFI_RC_IDLE         0 // WL_IDLE_STATUS
#define LT_WIFI_RC_CONNECTED    3 // WL_CONNECTED
#define LT_WIFI_RC_FAILED       4 // WL_CONNECT_FAILED
#define LT_WIFI_RC_LOST         5 // WL_CONNECTION_LOST
#define LT_WIFI_RC_DISCONNECTED 6 // WL_DISCONNECTED

// Backoff window for repeated rejoin attempts: ramps so a vanished AP isn't
// hammered.
#define LT_WIFI_RC_MIN_BACKOFF_MS 2000u
#define LT_WIFI_RC_MAX_BACKOFF_MS 30000u

// A supervisor-fired join that never reaches CONNECT_IND/GOT_IP within this
// window is abandoned and retried (the async rejoin has no other timeout; the
// blocking app-side begin() has its own 20 s bound).
#define LT_WIFI_RC_JOIN_TIMEOUT_MS 20000u

// While WL_CONNECTED, a link-health probe that reports bad continuously for
// this long demotes the link to the retry path. Generous, so a single
// transient probe failure on a healthy link can't trip a spurious reconnect.
#define LT_WIFI_RC_HEALTH_TIMEOUT_MS 15000u

typedef enum {
	LT_WIFI_RC_ACTION_NONE = 0, // nothing to do this tick
	LT_WIFI_RC_ACTION_JOIN,		// fire a (re)join command now
	LT_WIFI_RC_ACTION_DEMOTE,	// believe the link is gone: mark it LOST (a later tick joins)
} lt_wifi_rc_action_t;

typedef struct {
	bool enabled;		   // auto-reconnect enabled (WiFi.setAutoReconnect)
	bool armed;			   // a connection reached GOT_IP (or static-IP link-up) at least once
	bool have_ssid;		   // a target SSID is stored
	int status;			   // current WiFiStatus
	uint32_t now_ms;	   // millis()
	uint32_t last_join_ms; // when the supervisor last fired a join; 0 = none since arm
	uint32_t backoff_ms;   // current backoff window
	uint32_t health_bad_ms; // how long the link-health probe has reported bad while CONNECTED (0 = healthy / n/a)
} lt_wifi_rc_input_t;

// Decide what the supervisor should do this tick. Pure: no side effects.
lt_wifi_rc_action_t lt_wifi_rc_decide(const lt_wifi_rc_input_t *in);

// Advance the backoff window after a join fires: 0 -> MIN, then doubles, capped
// at MAX. Exported for the wrapper and the tests.
uint32_t lt_wifi_rc_next_backoff(uint32_t cur);

#ifdef __cplusplus
}
#endif
