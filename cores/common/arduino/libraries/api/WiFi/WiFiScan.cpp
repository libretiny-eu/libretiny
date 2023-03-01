/* Copyright (c) Kuba Szczodrzyński 2022-04-25. */

#include "WiFi.h"

bool WiFiClass::getNetworkInfo(
	uint8_t networkItem, String &ssid, WiFiAuthMode &encType, int32_t &rssi, uint8_t *&bssid, int32_t &channel
) {
	ssid	= SSID(networkItem);
	encType = encryptionType(networkItem);
	rssi	= RSSI(networkItem);
	bssid	= BSSID(networkItem);
	channel = this->channel(networkItem);
	return true;
}

int16_t WiFiClass::scanComplete() {
	if (!scan)
		return 0;
	if (scan->running)
		return WIFI_SCAN_RUNNING;
	return scan->count;
}

void WiFiClass::scanInit() {
	if (scan)
		return;
	scan = (WiFiScanData *)calloc(1, sizeof(WiFiScanData));
}

void WiFiClass::scanDelete() {
	if (!scan)
		return;
	for (uint8_t i = 0; i < scan->count; i++) {
		free(scan->ap[i].ssid);
	}
	free(scan->ap);
	free(scan);
	scan = NULL;
}

uint8_t WiFiClass::scanAlloc(uint8_t count) {
	uint8_t last = scan->count;
	scan->count	 = count;
	scan->ap	 = (WiFiScanAP *)realloc(scan->ap, count * sizeof(WiFiScanAP));
	if (!scan->ap)
		return 255;
	memset(scan->ap + last, 0, sizeof(WiFiScanAP));
	return last;
}

String WiFiClass::SSID(uint8_t networkItem) {
	if (!scan || networkItem >= scan->count)
		return "";
	return scan->ap[networkItem].ssid;
}

WiFiAuthMode WiFiClass::encryptionType(uint8_t networkItem) {
	if (!scan || networkItem >= scan->count)
		return WIFI_AUTH_INVALID;
	return scan->ap[networkItem].auth;
}

int32_t WiFiClass::RSSI(uint8_t networkItem) {
	if (!scan || networkItem >= scan->count)
		return 0;
	return scan->ap[networkItem].rssi;
}

uint8_t *WiFiClass::BSSID(uint8_t networkItem) {
	if (!scan || networkItem >= scan->count)
		return NULL;
	return scan->ap[networkItem].bssid.addr;
}

String WiFiClass::BSSIDstr(uint8_t networkItem) {
	return macToString(BSSID(networkItem));
}

int32_t WiFiClass::channel(uint8_t networkItem) {
	if (!scan || networkItem >= scan->count)
		return 0;
	return scan->ap[networkItem].channel;
}
