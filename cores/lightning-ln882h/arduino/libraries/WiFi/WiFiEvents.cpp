/* Copyright (c) Etienne Le Cousin 2024-03-10. */

#include "WiFiPrivate.h"

void wifiEventSendArduino(EventId event) {
	EventInfo eventInfo;
	memset(&eventInfo, 0, sizeof(EventInfo));
	if (!pWiFi)
		return; // failsafe
	pWiFi->postEvent(event, eventInfo);
}

static void wifiEventStaStartup(void *arg) {
	wifiEventSendArduino(ARDUINO_EVENT_WIFI_STA_START);
}

static void wifiEventStaConnected(void *arg) {
	EventInfo eventInfo;
	memset(&eventInfo, 0, sizeof(EventInfo));
	if (!pWiFi)
		return; // failsafe

	String ssid							  = pWiFi->SSID();
	eventInfo.wifi_sta_connected.ssid_len = ssid.length();
	eventInfo.wifi_sta_connected.channel  = pWiFi->channel();
	eventInfo.wifi_sta_connected.authmode = pWiFi->getEncryption();
	memcpy(eventInfo.wifi_sta_connected.ssid, ssid.c_str(), eventInfo.wifi_sta_connected.ssid_len + 1);
	memcpy(eventInfo.wifi_sta_connected.bssid, pWiFi->BSSID(), 6);

	pWiFi->postEvent(ARDUINO_EVENT_WIFI_STA_CONNECTED, eventInfo);
}

static void wifiEventStaDisconnected(void *arg) {
	EventInfo eventInfo;
	memset(&eventInfo, 0, sizeof(EventInfo));
	if (!pWiFi)
		return; // failsafe

	eventInfo.wifi_sta_disconnected.ssid_len = 0;
	eventInfo.wifi_sta_disconnected.reason	 = WIFI_REASON_ASSOC_LEAVE;
	pWiFi->postEvent(ARDUINO_EVENT_WIFI_STA_DISCONNECTED, eventInfo);
}

static void wifiEventStaConnectFailed(void *arg) {
	wifi_sta_connect_failed_reason_t reason = *(wifi_sta_connect_failed_reason_t *)arg;
	EventInfo eventInfo;
	memset(&eventInfo, 0, sizeof(EventInfo));
	if (!pWiFi)
		return; // failsafe

	eventInfo.wifi_sta_disconnected.ssid_len = 0;
	eventInfo.wifi_sta_disconnected.reason	 = WIFI_REASON_UNSPECIFIED;
	switch (reason) {
		case WIFI_STA_CONN_WRONG_PWD:
			eventInfo.wifi_sta_disconnected.reason = WIFI_REASON_AUTH_FAIL;
			break;
		case WIFI_STA_CONN_TARGET_AP_NOT_FOUND:
			eventInfo.wifi_sta_disconnected.reason = WIFI_REASON_NO_AP_FOUND;
			break;
		case WIFI_STA_CONN_TIMEOUT:
			eventInfo.wifi_sta_disconnected.reason = WIFI_REASON_BEACON_TIMEOUT;
			break;
		case WIFI_STA_CONN_REFUSED:
			eventInfo.wifi_sta_disconnected.reason = WIFI_REASON_CONNECTION_FAIL;
			break;
	}
	pWiFi->postEvent(ARDUINO_EVENT_WIFI_STA_DISCONNECTED, eventInfo);
}

static void wifiEventSoftAPStartup(void *arg) {
	netdev_set_state(NETIF_IDX_AP, NETDEV_UP);
	wifiEventSendArduino(ARDUINO_EVENT_WIFI_AP_START);
}

static void wifiEventSoftAPAssociated(void *arg) {
	const uint8_t *mac_addr = (const uint8_t *)arg;
	EventInfo eventInfo;
	memset(&eventInfo, 0, sizeof(EventInfo));
	if (!pWiFi)
		return; // failsafe

	memcpy(eventInfo.wifi_ap_staconnected.mac, mac_addr, 6);
	pWiFi->postEvent(ARDUINO_EVENT_WIFI_AP_STACONNECTED, eventInfo);
}

static void wifiEventSoftAPDisassociated(void *arg) {
	const uint8_t *mac_addr = (const uint8_t *)arg;
	EventInfo eventInfo;
	memset(&eventInfo, 0, sizeof(EventInfo));
	if (!pWiFi)
		return; // failsafe

	memcpy(eventInfo.wifi_ap_staconnected.mac, mac_addr, 6);
	pWiFi->postEvent(ARDUINO_EVENT_WIFI_AP_STADISCONNECTED, eventInfo);
}

static void wifiEventIpReceived(struct netif *nif) {
	EventInfo eventInfo;
	memset(&eventInfo, 0, sizeof(EventInfo));
	if (!pWiFi || !nif)
		return; // failsafe

	eventInfo.got_ip.if_index			  = 0;
	eventInfo.got_ip.ip_changed			  = true;
	eventInfo.got_ip.ip_info.ip.addr	  = nif->ip_addr.addr;
	eventInfo.got_ip.ip_info.gw.addr	  = nif->gw.addr;
	eventInfo.got_ip.ip_info.netmask.addr = nif->netmask.addr;
	pWiFi->postEvent(ARDUINO_EVENT_WIFI_STA_GOT_IP, eventInfo);
}

void registerWifiHandlers() {
	wifi_manager_reg_event_callback(WIFI_MGR_EVENT_STA_STARTUP, &wifiEventStaStartup);
	wifi_manager_reg_event_callback(WIFI_MGR_EVENT_STA_CONNECTED, &wifiEventStaConnected);
	wifi_manager_reg_event_callback(WIFI_MGR_EVENT_STA_DISCONNECTED, &wifiEventStaDisconnected);
	wifi_manager_reg_event_callback(WIFI_MGR_EVENT_STA_CONNECT_FAILED, &wifiEventStaConnectFailed);

	netdev_get_ip_cb_set(&wifiEventIpReceived);

	wifi_manager_reg_event_callback(WIFI_MGR_EVENT_SOFTAP_STARTUP, &wifiEventSoftAPStartup);
	wifi_manager_reg_event_callback(WIFI_MGR_EVENT_SOFTAP_ASSOCIATED, &wifiEventSoftAPAssociated);
	wifi_manager_reg_event_callback(WIFI_MGR_EVENT_SOFTAP_DISASSOCIATED, &wifiEventSoftAPDisassociated);
}
