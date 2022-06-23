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
	scan = (WiFiScanData *)zalloc(sizeof(WiFiScanData));
}

void WiFiClass::scanDelete() {
	if (!scan)
		return;
	for (uint8_t i = 0; i < scan->count; i++) {
		free(scan->ssid[i]);
	}
	free(scan->ssid);
	free(scan->auth);
	free(scan->rssi);
	free(scan->bssid);
	free(scan->channel);
	free(scan);
	scan = NULL;
}

String WiFiClass::SSID(uint8_t networkItem) {
	if (!scan || networkItem >= scan->count)
		return "";
	return scan->ssid[networkItem];
}

WiFiAuthMode WiFiClass::encryptionType(uint8_t networkItem) {
	if (!scan || networkItem >= scan->count)
		return WIFI_AUTH_INVALID;
	return scan->auth[networkItem];
}

int32_t WiFiClass::RSSI(uint8_t networkItem) {
	if (!scan || networkItem >= scan->count)
		return 0;
	return scan->rssi[networkItem];
}

uint8_t *WiFiClass::BSSID(uint8_t networkItem) {
	if (!scan || networkItem >= scan->count)
		return NULL;
	return scan->bssid[networkItem].addr;
}

String WiFiClass::BSSIDstr(uint8_t networkItem) {
	return macToString(BSSID(networkItem));
}

int32_t WiFiClass::channel(uint8_t networkItem) {
	if (!scan || networkItem >= scan->count)
		return 0;
	return scan->channel[networkItem];
}
