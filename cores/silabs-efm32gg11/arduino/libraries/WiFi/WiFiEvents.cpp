/* Copyright (c) WGM160P-LibreTiny port 2026-06-10. */

#include "WiFiPrivate.h"

static TaskHandle_t s_eventTask;

// --- STA auto-reconnect supervisor -------------------------------------------
// Rejoins a link that was established (reached GOT_IP) and then dropped, so the
// app needn't run its own WiFi.status() supervisor. Acts ONLY when s_armed —
// set on GOT_IP, cleared by a fresh begin()/intentional disconnect() — so a
// never-succeeded join (e.g. wrong password) is never looped; that stays the
// app's call via begin()'s return. Driven from the event task's idle tick, so
// the rejoin runs off the app thread and is non-blocking: it fires one join
// command and lets the normal CONNECT->GOT_IP ladder update staStatus.
//
// All the policy (when to join, back off, demote a dead link) lives in the pure
// lt_wifi_rc_decide() (base/wifi/lt_wifi_reconnect.c), which is host-unit-tested.
// This file is the thin wrapper: it keeps the timers, runs the link-health
// probe, and performs the hardware action the decision returns.
static volatile bool s_autoReconnect = true; // ESP32 parity: default ON
static volatile bool s_armed		 = false;
static uint32_t s_lastJoinMs		 = 0; // 0 = no attempt since arm/disarm
static uint32_t s_backoffMs			 = 0; // ramps 2s -> 30s so a vanished AP isn't hammered
static uint32_t s_lastProbeMs		 = 0; // last link-health probe (CONNECTED only)
static uint32_t s_healthBadMs		 = 0; // running time the probe has reported bad while CONNECTED

#define LT_WIFI_RECONNECT_POLL_MS 1000u // event-task idle wake to run the tick
#define LT_WIFI_HEALTH_PROBE_MS	  5000u // link-health probe cadence (gentler than the tick)

void ltWifiSetAutoReconnect(bool enable) {
	s_autoReconnect = enable;
}

bool ltWifiGetAutoReconnect() {
	return s_autoReconnect;
}

void ltWifiReconnectArm() {
	s_armed		  = true;
	s_backoffMs	  = 0;
	s_lastJoinMs  = 0;
	s_healthBadMs = 0;
	s_lastProbeMs = 0;
}

void ltWifiReconnectDisarm() {
	s_armed		  = false;
	s_backoffMs	  = 0;
	s_lastJoinMs  = 0;
	s_healthBadMs = 0;
	s_lastProbeMs = 0;
}

// Cheap firmware liveness query. NOTE (bench-validate): this treats a failed
// signal-strength command as a dead link. If the WF200 keeps returning OK with
// a stale RCPI after the AP vanishes, swap this probe for an active reachability
// check (e.g. a gateway ARP) — lt_wifi_rc_decide() is probe-agnostic and the
// host tests stay valid, only this function changes.
static bool ltWifiLinkHealthy() {
	uint32_t rcpi = 0;
	WIFI_CMD_TAKE(pDATA);
	sl_status_t s = sl_wfx_get_signal_strength(&rcpi);
	WIFI_CMD_GIVE(pDATA);
	return s == SL_STATUS_OK;
}

// Maintain s_healthBadMs: while CONNECTED, probe at LT_WIFI_HEALTH_PROBE_MS
// cadence and accrue the elapsed interval for each consecutive bad result;
// reset on a good result or whenever we are not CONNECTED.
static void ltWifiUpdateHealth(uint32_t now) {
	if (pDATA->staStatus != WL_CONNECTED) {
		s_healthBadMs = 0;
		s_lastProbeMs = now ? now : 1;
		return;
	}
	if (s_lastProbeMs != 0 && (now - s_lastProbeMs) < LT_WIFI_HEALTH_PROBE_MS)
		return;
	uint32_t interval = s_lastProbeMs ? (now - s_lastProbeMs) : 0;
	s_lastProbeMs	  = now ? now : 1;
	if (ltWifiLinkHealthy())
		s_healthBadMs = 0;
	else
		s_healthBadMs += interval;
}

// Fire one (re)join command. Non-blocking: leaves staStatus in WL_IDLE_STATUS
// and lets the normal CONNECT->GOT_IP ladder finish it (or the join-timeout in
// lt_wifi_rc_decide() abandon it).
static void ltWifiFireRejoin(uint32_t now) {
	s_lastJoinMs = now ? now : 1; // keep 0 reserved as the "no attempt yet" sentinel
	s_backoffMs	 = lt_wifi_rc_next_backoff(s_backoffMs);

	WiFiNetworkInfo &info			= pDATA->sta;
	sl_wfx_security_mode_t security = info.password ? WFM_SECURITY_MODE_WPA2_PSK : WFM_SECURITY_MODE_OPEN;
	LT_IM(WIFI, "auto-reconnect: rejoining %s", info.ssid);
	pDATA->staStatus = WL_IDLE_STATUS;
	ltWifiStatusLed(LT_WIFI_LED_JOINING);
	WIFI_CMD_TAKE(pDATA);
	sl_status_t s = sl_wfx_send_join_command(
		(const uint8_t *)info.ssid,
		strlen(info.ssid),
		NULL,
		0 /* channel: any */,
		security,
		1 /* prevent_roaming */,
		WFM_MGMT_FRAME_PROTECTION_OPTIONAL,
		(const uint8_t *)(info.password ? info.password : ""),
		info.password ? strlen(info.password) : 0,
		NULL,
		0
	);
	WIFI_CMD_GIVE(pDATA);
	if (s != SL_STATUS_OK) {
		LT_EM(WIFI, "auto-reconnect join cmd failed: 0x%08lX", (unsigned long)s);
		pDATA->staStatus = WL_CONNECTION_LOST; // stay retryable for the next tick
	}
}

// Synthesize a disconnect for a link the firmware never told us was gone (a
// silently-dead CONNECTED link, or a rejoin stuck in WL_IDLE_STATUS). Clears the
// firmware association so the next tick's join starts clean, then drops to the
// retry path. Mirrors reconnect()'s timeout teardown.
static void ltWifiDemoteLink() {
	LT_IM(WIFI, "auto-reconnect: link down with no indication — forcing retry");
	WIFI_CMD_TAKE(pDATA);
	sl_wfx_send_disconnect_command();
	WIFI_CMD_GIVE(pDATA);
	lt_wf200_netif_set_link(false);
	pDATA->staStatus = WL_CONNECTION_LOST;
	s_healthBadMs	 = 0;
	ltWifiStatusLed(LT_WIFI_LED_IDLE);
}

void ltWifiReconnectTick() {
	if (!pWiFi)
		return;
	uint32_t now = millis();
	ltWifiUpdateHealth(now);

	lt_wifi_rc_input_t in = {
		.enabled	   = s_autoReconnect,
		.armed		   = s_armed,
		.have_ssid	   = pDATA->sta.ssid != NULL,
		.status		   = (int)pDATA->staStatus,
		.now_ms		   = now,
		.last_join_ms  = s_lastJoinMs,
		.backoff_ms	   = s_backoffMs,
		.health_bad_ms = s_healthBadMs,
	};
	switch (lt_wifi_rc_decide(&in)) {
		case LT_WIFI_RC_ACTION_JOIN:
			ltWifiFireRejoin(now);
			break;
		case LT_WIFI_RC_ACTION_DEMOTE:
			ltWifiDemoteLink(); // next tick fires the join
			break;
		case LT_WIFI_RC_ACTION_NONE:
		default:
			break;
	}
}

// Map the WF200 scan-result security bitmask to Arduino auth modes.
// The host port (sl_wfx_host.c) memcpy's the raw 1-byte packed bitfield
// (sl_wfx_security_mode_bitmask_t, sl_wfx_cmd_api.h) into the event's
// uint32_t — decode it back through the named bits, not magic hex.
// NOTE: consolidated here (the event task below needs it); Task 13's
// WiFiScan.cpp must NOT duplicate this.
WiFiAuthMode wfxSecurityToAuthMode(uint32_t security) {
	sl_wfx_security_mode_bitmask_t bits;
	uint8_t raw = (uint8_t)security;
	memcpy(&bits, &raw, sizeof(bits));
	if (bits.wpa3)
		return bits.wpa2 ? WIFI_AUTH_WPA2_WPA3_PSK : WIFI_AUTH_WPA3_PSK;
	if (bits.wpa2) {
		if (bits.eap && !bits.psk)
			return WIFI_AUTH_WPA2_ENTERPRISE;
		return bits.wpa ? WIFI_AUTH_WPA_WPA2_PSK : WIFI_AUTH_WPA2_PSK;
	}
	if (bits.wpa)
		return WIFI_AUTH_WPA_PSK;
	if (bits.wep)
		return WIFI_AUTH_WEP;
	return WIFI_AUTH_OPEN;
}

// DEADLOCK CONSTRAINT: onEvent callbacks run SYNCHRONOUSLY in this task (via
// postEvent). Calling blocking WiFi APIs (scanNetworks, begin, disconnect)
// from a callback deadlocks — those APIs wait on state changes that only this
// task can deliver. Not supported.
static void wifiEventTask(void *arg) {
	(void)arg;
	lt_wfx_event_t ev;
	for (;;) {
		if (xQueueReceive(lt_wfx_host.event_queue, &ev, pdMS_TO_TICKS(LT_WIFI_RECONNECT_POLL_MS)) != pdTRUE) {
			ltWifiReconnectTick(); // idle wake: supervise STA auto-reconnect
			continue;
		}
		if (!pWiFi)
			continue;
		EventInfo info;
		memset(&info, 0, sizeof(info));

		switch (ev.id) {
			case SL_WFX_CONNECT_IND_ID:
				if (ev.u.connect.status == 0) {
					lt_wf200_netif_set_link(true);
					if (pDATA->sta.localIP == 0)
						lt_wf200_netif_dhcp_start();
					pDATA->staStatus = WL_IDLE_STATUS; // CONNECTED only on IP
					// stash for the BSSID()/channel() getters (WiFiSTA.cpp)
					memcpy(pDATA->staBssid, ev.u.connect.mac, 6);
					pDATA->staChannel = ev.u.connect.channel;
					memcpy(info.wifi_sta_connected.bssid, ev.u.connect.mac, 6);
					info.wifi_sta_connected.channel = (uint8_t)ev.u.connect.channel;
					if (pDATA->sta.ssid) {
						uint8_t len = strlen(pDATA->sta.ssid);
						if (len > 32)
							len = 32;
						memcpy(info.wifi_sta_connected.ssid, pDATA->sta.ssid, len);
						info.wifi_sta_connected.ssid_len = len;
					}
					pWiFi->postEvent(ARDUINO_EVENT_WIFI_STA_CONNECTED, info);
				} else {
					pDATA->staStatus = WL_CONNECT_FAILED;
					ltWifiStatusLed(LT_WIFI_LED_JOIN_FAIL);
					info.wifi_sta_disconnected.reason = (uint8_t)ev.u.connect.status;
					pWiFi->postEvent(ARDUINO_EVENT_WIFI_STA_DISCONNECTED, info);
				}
				break;

			case SL_WFX_DISCONNECT_IND_ID:
				lt_wf200_netif_set_link(false);
				if (pDATA->staStatus == WL_CONNECTED || pDATA->staStatus == WL_IDLE_STATUS)
					pDATA->staStatus = WL_CONNECTION_LOST;
				// Radio still up; drop back to the idle (not-joined) color.
				ltWifiStatusLed(LT_WIFI_LED_IDLE);
				info.wifi_sta_disconnected.reason = (uint8_t)ev.u.disconnect.reason;
				pWiFi->postEvent(ARDUINO_EVENT_WIFI_STA_DISCONNECTED, info);
				break;

			case SL_WFX_SCAN_RESULT_IND_ID: {
				WiFiScanData *scan = pWiFi->scan;
				if (!scan || !scan->running)
					break;
				uint8_t last = scan->count + 1;
				if (pWiFi->scanAlloc(last) < last)
					break;
				last--;
				scan->ap[last].ssid	   = strdup((char *)ev.u.scan_result.ssid);
				scan->ap[last].rssi	   = ev.u.scan_result.rssi_dbm;
				scan->ap[last].channel = (int32_t)ev.u.scan_result.channel;
				scan->ap[last].auth	   = wfxSecurityToAuthMode(ev.u.scan_result.security);
				memcpy(scan->ap[last].bssid.addr, ev.u.scan_result.mac, 6);
				break;
			}

			case SL_WFX_SCAN_COMPLETE_IND_ID:
				info.wifi_scan_done.status = ev.u.scan_complete.status == 0 ? 0 : 1;
				if (pWiFi->scan) {
					pWiFi->scan->running	   = false;
					info.wifi_scan_done.number = pWiFi->scan->count;
				}
				xSemaphoreGive(pDATA->scanSem);
				pWiFi->postEvent(ARDUINO_EVENT_WIFI_SCAN_DONE, info);
				break;

			case SL_WFX_START_AP_IND_ID:
				pDATA->apActive = true;
				pWiFi->postEvent(ARDUINO_EVENT_WIFI_AP_START, info);
				break;

			case SL_WFX_STOP_AP_IND_ID:
				pDATA->apActive		 = false;
				pDATA->apClientCount = 0;
				pWiFi->postEvent(ARDUINO_EVENT_WIFI_AP_STOP, info);
				break;

			case SL_WFX_AP_CLIENT_CONNECTED_IND_ID:
				if (pDATA->apClientCount < 255)
					pDATA->apClientCount++;
				memcpy(info.wifi_ap_staconnected.mac, ev.u.ap_client.mac, 6);
				pWiFi->postEvent(ARDUINO_EVENT_WIFI_AP_STACONNECTED, info);
				break;

			case SL_WFX_AP_CLIENT_DISCONNECTED_IND_ID:
				if (pDATA->apClientCount > 0)
					pDATA->apClientCount--;
				memcpy(info.wifi_ap_stadisconnected.mac, ev.u.ap_client.mac, 6);
				pWiFi->postEvent(ARDUINO_EVENT_WIFI_AP_STADISCONNECTED, info);
				break;
		}
	}
}

// lwIP netif status callback fires on IP acquisition/loss (tcpip thread
// context). DEADLOCK CONSTRAINT: onEvent callbacks run synchronously here;
// calling blocking WiFi APIs (scanNetworks, begin, ...) from a callback would
// block the tcpip thread and deadlock — not supported.
static void netifStatusCb(struct netif *nif) {
	if (!pWiFi)
		return;
	EventInfo info;
	memset(&info, 0, sizeof(info));
	if (!ip4_addr_isany_val(*netif_ip4_addr(nif))) {
		pDATA->staStatus = WL_CONNECTED;
		// Link is up end-to-end (L2 + IP): eligible for backend auto-rejoin if it
		// later drops. Also resets the retry backoff after a clean connect.
		ltWifiReconnectArm();
		// End-to-end up (L2 + IP): green.
		ltWifiStatusLed(LT_WIFI_LED_JOINED);
		info.got_ip.ip_info.ip.addr		 = netif_ip4_addr(nif)->addr;
		info.got_ip.ip_info.netmask.addr = netif_ip4_netmask(nif)->addr;
		info.got_ip.ip_info.gw.addr		 = netif_ip4_gw(nif)->addr;
		pWiFi->postEvent(ARDUINO_EVENT_WIFI_STA_GOT_IP, info);
	} else if (pDATA->staStatus == WL_CONNECTED) {
		// Address became zero while we believed we had one — IP lost
		// (e.g. DHCP released/expired without renewal).
		pDATA->staStatus = WL_DISCONNECTED;
		// Lost our lease but the link may still be up: idle blue.
		ltWifiStatusLed(LT_WIFI_LED_IDLE);
		pWiFi->postEvent(ARDUINO_EVENT_WIFI_STA_LOST_IP, info);
	}
}

void ltWifiAttachNetifCallback() {
	struct netif *nif = lt_wf200_netif_sta();
	if (!nif)
		return;
	// Direct call, not tcpip_callback(): modePriv calls this immediately
	// after lt_wf200_netif_create() succeeds, while the link is still down
	// and no traffic can race the tcpip thread. Anyone moving this call
	// later in the bring-up sequence must switch to tcpip_callback().
	netif_set_status_callback(nif, netifStatusCb);
}

bool startWifiEventTask() {
	if (lt_wfx_host_setup() != SL_STATUS_OK) {
		// don't spawn the event task on a NULL queue
		LT_EM(WIFI, "WF200 host setup failed");
		return false;
	}
	if (!s_eventTask) {
		// PA8 = WF200 WIRQ -> EXTI line 8 via the family dispatcher (wiring_irq).
		// Inside the guard: re-calls must not re-attach the interrupt.
		attachInterrupt(0x08 /* PA8: (port A=0)<<4 | 8 */, []() { lt_wfx_host_wirq_signal(); }, RISING);
		if (xTaskCreate(wifiEventTask, "wifievent", 1024, NULL, 4, &s_eventTask) != pdPASS) {
			LT_EM(WIFI, "WiFi event task create failed");
			detachInterrupt(0x08);
			s_eventTask = NULL;
			return false;
		}
	}
	return true;
}
