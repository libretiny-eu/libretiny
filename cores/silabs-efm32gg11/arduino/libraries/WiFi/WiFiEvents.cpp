/* Copyright (c) WGM160P-LibreTiny port 2026-06-10. */

#include "WiFiPrivate.h"

static TaskHandle_t s_eventTask;

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
		if (xQueueReceive(lt_wfx_host.event_queue, &ev, portMAX_DELAY) != pdTRUE)
			continue;
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
