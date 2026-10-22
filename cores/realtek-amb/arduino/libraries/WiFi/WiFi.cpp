/* Copyright (c) Kuba Szczodrzyński 2022-04-25. */

#include "WiFiPrivate.h"

rtw_wifi_setting_t wifi_setting;

WiFiClass::WiFiClass() {
	data = (WiFiData *)calloc(1, sizeof(WiFiData));

	DATA->scanSem = xSemaphoreCreateBinary();
}

WiFiClass::~WiFiClass() {
	vSemaphoreDelete(DATA->scanSem);
	free(data);
	data = NULL;
}

WiFiAuthMode securityTypeToAuthMode(uint8_t type) {
	// the value reported in rtw_scan_result is rtw_encryption_t, even though it's rtw_security_t in the header file
	switch (type) {
		case RTW_ENCRYPTION_OPEN:
			return WIFI_AUTH_OPEN;
		case RTW_ENCRYPTION_WEP40:
		case RTW_ENCRYPTION_WEP104:
			return WIFI_AUTH_WEP;
		case RTW_ENCRYPTION_WPA_TKIP:
		case RTW_ENCRYPTION_WPA_AES:
			return WIFI_AUTH_WPA_PSK;
		case RTW_ENCRYPTION_WPA2_TKIP:
		case RTW_ENCRYPTION_WPA2_AES:
		case RTW_ENCRYPTION_WPA2_MIXED:
			return WIFI_AUTH_WPA2_PSK;
	}
	return WIFI_AUTH_INVALID;
}
