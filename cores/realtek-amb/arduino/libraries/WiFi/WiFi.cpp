/* Copyright (c) Kuba Szczodrzyński 2022-04-25. */

#include "WiFiPrivate.h"

// TODO move these to WiFiData
rtw_network_info_t wifi = {0};
rtw_ap_info_t ap		= {0};
rtw_wifi_setting_t wifi_setting;
unsigned char sta_password[65] = {0};
unsigned char ap_password[65]  = {0};

void reset_wifi_struct(void) {
	memset(wifi.ssid.val, 0, sizeof(wifi.ssid.val));
	memset(wifi.bssid.octet, 0, ETH_ALEN);
	memset(sta_password, 0, sizeof(sta_password));
	memset(ap_password, 0, sizeof(ap_password));
	wifi.ssid.len	  = 0;
	wifi.password	  = NULL;
	wifi.password_len = 0;
	wifi.key_id		  = -1;
	memset(ap.ssid.val, 0, sizeof(ap.ssid.val));
	ap.ssid.len		= 0;
	ap.password		= NULL;
	ap.password_len = 0;
	ap.channel		= 1;
}

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
