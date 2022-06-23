/* Copyright (c) Kuba Szczodrzyński 2022-04-25. */

#include "WiFi.h"

WiFiStatus WiFiClass::begin(char *ssid, char *passphrase, int32_t channel, const uint8_t *bssid, bool connect) {
	return begin((const char *)ssid, (const char *)passphrase, channel, bssid, connect);
}

bool WiFiClass::isConnected() {
	return status() == WL_CONNECTED;
}

WiFiStatus WiFiClass::waitForConnectResult(unsigned long timeout) {
	if ((getMode() & WIFI_MODE_STA) == 0) {
		return WL_DISCONNECTED;
	}
	unsigned long start = millis();
	while ((!status() || status() >= WL_DISCONNECTED) && (millis() - start) < timeout) {
		delay(100);
	}
	return status();
}

String WiFiClass::macAddress(void) {
	uint8_t mac[6];
	macAddress(mac);
	return macToString(mac);
}

IPAddress WiFiClass::networkID() {
	return calculateNetworkID(gatewayIP(), subnetMask());
}

uint8_t WiFiClass::subnetCIDR() {
	return calculateSubnetCIDR(subnetMask());
}

String WiFiClass::BSSIDstr() {
	return macToString(BSSID());
}

__attribute__((weak)) bool WiFiClass::enableIpV6() {
	return false;
}

__attribute__((weak)) IPv6Address WiFiClass::localIPv6() {
	return IPv6Address();
}
