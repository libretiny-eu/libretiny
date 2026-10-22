/* Copyright (c) Kuba Szczodrzyński 2022-04-25. */

#include "WiFi.h"

IPAddress WiFiClass::softAPBroadcastIP() {
	return calculateBroadcast(softAPIP(), softAPSubnetMask());
}

IPAddress WiFiClass::softAPNetworkID() {
	return calculateNetworkID(softAPIP(), softAPSubnetMask());
}

uint8_t WiFiClass::softAPSubnetCIDR() {
	return calculateSubnetCIDR(softAPSubnetMask());
}

__attribute__((weak)) bool WiFiClass::softAPenableIpV6() {
	return false;
}

__attribute__((weak)) IPv6Address WiFiClass::softAPIPv6() {
	return IPv6Address();
}
