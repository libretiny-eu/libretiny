/* Copyright (c) Etienne Le Cousin 2024-02-18. */

#include "WiFiPrivate.h"

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
	switch (type) {
		case LN_WIFI_AUTH_OPEN:
			return WIFI_AUTH_OPEN;
		case LN_WIFI_AUTH_WEP:
			return WIFI_AUTH_WEP;
		case LN_WIFI_AUTH_WPA_PSK:
			return WIFI_AUTH_WPA_PSK;
		case LN_WIFI_AUTH_WPA2_PSK:
			return WIFI_AUTH_WPA2_PSK;
		case LN_WIFI_AUTH_WPA_WPA2_PSK:
			return WIFI_AUTH_WPA_WPA2_PSK;
		case LN_WIFI_AUTH_WPA2_ENTERPRISE:
			return WIFI_AUTH_WPA2_ENTERPRISE;
		case LN_WIFI_AUTH_WPA3_SAE:
			return WIFI_AUTH_WPA3_PSK;
		case LN_WIFI_AUTH_WPA2_PSK_WPA3_SAE:
			return WIFI_AUTH_WPA2_WPA3_PSK;
	}
	return WIFI_AUTH_INVALID;
}

uint8_t authModeToSecurityType(WiFiAuthMode auth) {
	switch (auth) {
		case WIFI_AUTH_OPEN:
			return LN_WIFI_AUTH_OPEN;
		case WIFI_AUTH_WEP:
			return LN_WIFI_AUTH_WEP;
		case WIFI_AUTH_WPA_PSK:
			return LN_WIFI_AUTH_WPA_PSK;
		case WIFI_AUTH_WPA2_PSK:
			return LN_WIFI_AUTH_WPA2_PSK;
		case WIFI_AUTH_WPA_WPA2_PSK:
			return LN_WIFI_AUTH_WPA_WPA2_PSK;
		case WIFI_AUTH_WPA2_ENTERPRISE:
			return LN_WIFI_AUTH_WPA2_ENTERPRISE;
		case WIFI_AUTH_WPA3_PSK:
			return LN_WIFI_AUTH_WPA3_SAE;
		case WIFI_AUTH_WPA2_WPA3_PSK:
			return LN_WIFI_AUTH_WPA2_PSK_WPA3_SAE;
	}
	return WIFI_AUTH_INVALID;
}