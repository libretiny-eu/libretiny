/* Copyright (c) Kuba Szczodrzyński 2022-06-26. */

#include "WiFiPrivate.h"

WiFiClass::WiFiClass() {
	data = (WiFiData *)calloc(1, sizeof(WiFiData));

	DATA->scanSem		  = xSemaphoreCreateBinary();
	STA_ADV_CFG.dhcp_mode = DHCP_CLIENT;
}

WiFiClass::~WiFiClass() {
	vSemaphoreDelete(DATA->scanSem);
	free(data);
	data = NULL;
}

WiFiStatus eventTypeToStatus(uint8_t type) {
	// rw_msg_pub.h:9
	switch (type) {
		case RW_EVT_STA_IDLE:
			return WL_IDLE_STATUS;
		case RW_EVT_STA_NO_AP_FOUND:
			return WL_NO_SSID_AVAIL;
		case RW_EVT_STA_CONNECTING:
		case RW_EVT_STA_CONNECTED:
			return WL_SCAN_COMPLETED;
		case RW_EVT_STA_GOT_IP:
			return WL_CONNECTED;
		case RW_EVT_STA_PASSWORD_WRONG:
		case RW_EVT_STA_ASSOC_FULL:
		case RW_EVT_STA_CONNECT_FAILED:
			return WL_CONNECT_FAILED;
		case RW_EVT_STA_BEACON_LOSE:
			return WL_CONNECTION_LOST;
		case RW_EVT_STA_DISCONNECTED:
			return WL_DISCONNECTED;
	}
	return WL_IDLE_STATUS;
}

WiFiAuthMode securityTypeToAuthMode(uint8_t type) {
	// wlan_ui_pub.h:62
	switch (type) {
		case BK_SECURITY_TYPE_NONE:
			return WIFI_AUTH_OPEN;
		case BK_SECURITY_TYPE_WEP:
			return WIFI_AUTH_WEP;
		case BK_SECURITY_TYPE_WPA_TKIP:
		case BK_SECURITY_TYPE_WPA_AES:
			return WIFI_AUTH_WPA_PSK;
		case BK_SECURITY_TYPE_WPA2_TKIP:
		case BK_SECURITY_TYPE_WPA2_AES:
		case BK_SECURITY_TYPE_WPA2_MIXED:
			return WIFI_AUTH_WPA2_PSK;
		case BK_SECURITY_TYPE_WPA3_SAE:
			return WIFI_AUTH_WPA3_PSK;
		case BK_SECURITY_TYPE_WPA3_WPA2_MIXED:
			return WIFI_AUTH_WPA2_WPA3_PSK;
	}
	return WIFI_AUTH_INVALID;
}
