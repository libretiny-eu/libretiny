/* Copyright (c) Kuba Szczodrzyński 2022-04-25. */

#include "WiFi.h"
#include "WiFiPriv.h"

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
	_scanSem = xSemaphoreCreateBinary();
}

WiFiClass::~WiFiClass() {
	vSemaphoreDelete(_scanSem);
}

void WiFiClass::printDiag(Print &dest) {
	const char *modes[] = {"NULL", "STA", "AP", "STA+AP"};
	const char *enc[]	= {"Open", "WEP", "WPA PSK", "WPA2 PSK", "WPA/WPA2", "WPA", "WPA2"};

	dest.print("Mode: ");
	dest.println(modes[getMode()]);

	if (wifi_mode & WIFI_MODE_STA) {
		dest.println("-- Station --");
		dest.print("SSID: ");
		dest.println(SSID());
		if (isConnected()) {
			dest.print("BSSID: ");
			dest.println(BSSIDstr());
			dest.print("RSSI: ");
			dest.println(RSSI());
			dest.print("Encryption: ");
			dest.println(enc[getEncryption()]);
			dest.print("IP: ");
			dest.println(localIP());
			dest.print("MAC: ");
			dest.println(macAddress());
			dest.print("Hostname: ");
			dest.println(getHostname());
		}
	}

	if (wifi_mode & WIFI_MODE_AP) {
		dest.println("-- Access Point --");
		dest.print("SSID: ");
		dest.println(softAPSSID());
		dest.print("IP: ");
		dest.println(softAPIP());
		dest.print("MAC: ");
		dest.println(softAPmacAddress());
		dest.print("Hostname: ");
		dest.println(softAPgetHostname());
	}
}

WiFiAuthMode WiFiClass::securityTypeToAuthMode(uint8_t type) {
	switch (wifi_setting.security_type) {
		case RTW_SECURITY_OPEN:
			return WIFI_AUTH_OPEN;
		case RTW_SECURITY_WEP_SHARED:
			return WIFI_AUTH_WEP;
		case RTW_SECURITY_WPA_TKIP_PSK:
			return WIFI_AUTH_WPA_PSK;
		case RTW_SECURITY_WPA_AES_PSK:
			return WIFI_AUTH_WPA;
		case RTW_SECURITY_WPA2_TKIP_PSK:
			return WIFI_AUTH_WPA2_PSK;
		case RTW_SECURITY_WPA2_AES_PSK:
			return WIFI_AUTH_WPA2;
		case RTW_SECURITY_WPA_WPA2_MIXED:
			return WIFI_AUTH_WPA_WPA2_PSK;
	}
	return WIFI_AUTH_INVALID;
}

WiFiClass WiFi;
