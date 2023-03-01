/* Copyright (c) Kuba Szczodrzyński 2022-06-26. */

#include "WiFiPrivate.h"

WiFiClass::WiFiClass() {
	memset(&data, 0x00, sizeof(WiFiData));
	data.scanSem = xSemaphoreCreateBinary();
}

WiFiClass::~WiFiClass() {
	vSemaphoreDelete(data.scanSem);
}

void WiFiClass::dataInitialize() {
	if (data.statusIp)
		return;
	LT_DM(WIFI, "Data init");
	data.configSta	   = calloc(1, sizeof(network_InitTypeDef_st));
	data.configAp	   = calloc(1, sizeof(network_InitTypeDef_ap_st));
	data.statusIp	   = malloc(sizeof(IPStatusTypedef));
	data.statusLink	   = malloc(sizeof(LinkStatusTypeDef));
	STA_CFG->dhcp_mode = DHCP_CLIENT;
	LT_DM(WIFI, "Data = %p", data.configSta);
}

void WiFiClass::dataFree() {
	LT_DM(WIFI, "Data free");
	free(data.configSta);
	free(data.configAp);
	free(data.statusIp);
	free(data.statusLink);
	data.configSta	= NULL;
	data.configAp	= NULL;
	data.statusIp	= NULL;
	data.statusLink = NULL;
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
