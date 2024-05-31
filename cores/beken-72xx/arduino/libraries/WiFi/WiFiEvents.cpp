/* Copyright (c) Kuba Szczodrzyński 2022-07-10. */

#include "WiFiPrivate.h"

#include <vector>

static xQueueHandle wifiEventQueueHandle = NULL;
static xTaskHandle wifiEventTaskHandle	 = NULL;

static const rw_evt_type eventConnected = RW_EVT_STA_CONNECTED;

// callback for bk_wlan_status_register_cb()
static void wifiStatusCallback(rw_evt_type *pEvent) {
	if (wifiEventQueueHandle && wifiEventTaskHandle) {
		xQueueSend(wifiEventQueueHandle, pEvent, portMAX_DELAY);
	} else {
		wifiEventHandler(*pEvent);
	}
}

static void wifiEventTask(void *arg) {
	rw_evt_type event = RW_EVT_MAX;
	for (;;) {
		if (xQueueReceive(wifiEventQueueHandle, &event, portMAX_DELAY) == pdTRUE) {
			wifiEventHandler(event);
		}
	}
}

// There is a race condition, when we have an event about a successful
// connection but no SSID yet returned by BDK. Even a single millisecond
// delay should prevent this from happening. It's better to waste a bit
// of time here than to lose a valid connection down the line.
static String waitForValidSSID(WiFiClass *pWiFi) {
	String result;

	// Read the initial value that might just be available already.
	result = pWiFi->SSID();

	if (!result.length()) {
		std::size_t i = 0;
		for (; i < 10; i++) {
			// Delay and query again.
			delay(1);
			result = pWiFi->SSID();

			if (result.length()) {
				LT_DM(WIFI, "Got valid SSID after %u delays", i + 1);
				break;
			}

			// It's a good idea to yield.
			yield();
		}

		if (!result.length()) {
			LT_WM(WIFI, "Could not obtain a valid SSID after %u delays", i);
		}
	}

	return result;
}

void wifiEventSendArduino(EventId event) {
	event = (EventId)(RW_EVT_ARDUINO | event);
	wifiStatusCallback((rw_evt_type *)&event);
}

void startWifiTask() {
	if (!wifiEventQueueHandle) {
		LT_HEAP_I();
		wifiEventQueueHandle = xQueueCreate(32, sizeof(rw_evt_type));
		LT_HEAP_I();
	}
	if (!wifiEventTaskHandle) {
		LT_HEAP_I();
		xTaskCreate(wifiEventTask, "wifievent", 512, NULL, 4, &wifiEventTaskHandle);
		LT_HEAP_I();
	}
	bk_wlan_status_register_cb((FUNC_1PARAM_PTR)wifiStatusCallback);
}

void wifiEventHandler(rw_evt_type event) {
	if (!pWiFi)
		return; // failsafe

	LT_DM(WIFI, "BK event %u", event);

	EventId eventId;
	EventInfo eventInfo;
	String ssid;

	memset(&eventInfo, 0, sizeof(EventInfo));

	switch (event) {
		case RW_EVT_STA_IDLE:
			eventId = ARDUINO_EVENT_WIFI_READY;
			break;

		case RW_EVT_STA_BEACON_LOSE:
		case RW_EVT_STA_PASSWORD_WRONG:
		case RW_EVT_STA_NO_AP_FOUND:
		case RW_EVT_STA_ASSOC_FULL:
		case RW_EVT_STA_DISCONNECTED:
		case RW_EVT_STA_CONNECT_FAILED:
			eventId									 = ARDUINO_EVENT_WIFI_STA_DISCONNECTED;
			eventInfo.wifi_sta_disconnected.ssid_len = 0;
			switch (event) {
				case RW_EVT_STA_BEACON_LOSE:
					eventInfo.wifi_sta_disconnected.reason = WIFI_REASON_BEACON_TIMEOUT;
					break;
				case RW_EVT_STA_PASSWORD_WRONG:
					eventInfo.wifi_sta_disconnected.reason = WIFI_REASON_AUTH_FAIL;
					break;
				case RW_EVT_STA_NO_AP_FOUND:
					eventInfo.wifi_sta_disconnected.reason = WIFI_REASON_NO_AP_FOUND;
					break;
				case RW_EVT_STA_ASSOC_FULL:
					eventInfo.wifi_sta_disconnected.reason = WIFI_REASON_ASSOC_TOOMANY;
					break;
				case RW_EVT_STA_DISCONNECTED:
					eventInfo.wifi_sta_disconnected.reason = WIFI_REASON_ASSOC_LEAVE;
					break;
				case RW_EVT_STA_CONNECT_FAILED:
					eventInfo.wifi_sta_disconnected.reason = WIFI_REASON_CONNECTION_FAIL;
					break;
				default:
					eventInfo.wifi_sta_disconnected.reason = WIFI_REASON_UNSPECIFIED;
					break;
			}
			break;

		case RW_EVT_STA_CONNECTED:
			eventId								  = ARDUINO_EVENT_WIFI_STA_CONNECTED;
			ssid								  = waitForValidSSID(pWiFi);
			eventInfo.wifi_sta_connected.ssid_len = ssid.length();
			eventInfo.wifi_sta_connected.channel  = pWiFi->channel();
			eventInfo.wifi_sta_connected.authmode = pWiFi->getEncryption();
			memcpy(eventInfo.wifi_sta_connected.ssid, ssid.c_str(), eventInfo.wifi_sta_connected.ssid_len + 1);
			memcpy(eventInfo.wifi_sta_connected.bssid, pWiFi->BSSID(), 6);
			break;

		case RW_EVT_STA_GOT_IP:
			eventId								  = ARDUINO_EVENT_WIFI_STA_GOT_IP;
			eventInfo.got_ip.if_index			  = 0;
			eventInfo.got_ip.ip_changed			  = true;
			eventInfo.got_ip.ip_info.ip.addr	  = WiFi.localIP();
			eventInfo.got_ip.ip_info.netmask.addr = WiFi.subnetMask();
			eventInfo.got_ip.ip_info.gw.addr	  = WiFi.gatewayIP();
			break;

		case RW_EVT_AP_CONNECTED:
			eventId = ARDUINO_EVENT_WIFI_AP_STACONNECTED;
			// TODO station MAC
			break;

		case RW_EVT_AP_DISCONNECTED:
			eventId = ARDUINO_EVENT_WIFI_AP_STADISCONNECTED;
			// TODO station MAC
			break;

		case RW_EVT_ARDUINO | ARDUINO_EVENT_WIFI_SCAN_DONE:
			eventId							= ARDUINO_EVENT_WIFI_SCAN_DONE;
			eventInfo.wifi_scan_done.status = 0;
			if (pWiFi->scan)
				eventInfo.wifi_scan_done.number = pWiFi->scan->count;
			eventInfo.wifi_scan_done.scan_id = 0;
			break;

		default:
			if (event < RW_EVT_ARDUINO)
				return;
			eventId = (EventId)(event - RW_EVT_ARDUINO);
			break;
	}

	// Publish state update only after the event data is retrieved.
	// This relates to the race condition with RW_EVT_STA_CONNECTED.
	if (event <= RW_EVT_STA_GOT_IP) {
		pDATA->lastStaEvent = event;
	} else {
		pDATA->lastApEvent = event;
	}

	pWiFi->postEvent(eventId, eventInfo);
}
