/* Copyright (c) Kuba Szczodrzyński 2022-06-26. */

#include "WiFiPriv.h"

WiFiClass::WiFiClass() {
	data.scanSem = xSemaphoreCreateBinary();
}

WiFiClass::~WiFiClass() {
	vSemaphoreDelete(data.scanSem);
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
