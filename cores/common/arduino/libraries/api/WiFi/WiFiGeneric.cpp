/* Copyright (c) Kuba Szczodrzyński 2022-04-25. */

#include "WiFi.h"

bool WiFiClass::mode(WiFiMode mode) {
	// store a pointer to WiFi for WiFiEvents.cpp
	pWiFi = this;

	WiFiMode currentMode = getMode();
	LT_DM(WIFI, "Mode changing %u -> %u", currentMode, mode);
	if (mode == currentMode)
		return true;

	// get mode changes as 0/1
	WiFiModeAction sta = WiFiModeAction((mode & WIFI_MODE_STA) != (currentMode & WIFI_MODE_STA));
	WiFiModeAction ap  = WiFiModeAction((mode & WIFI_MODE_AP) != (currentMode & WIFI_MODE_AP));
	// change 0/1 to 1/2
	sta = WiFiModeAction(sta + sta * !!(mode & WIFI_MODE_STA));
	ap	= WiFiModeAction(ap + ap * !!(mode & WIFI_MODE_AP));
	// initialize data structures if wifi is enabled
	if (mode)
		dataInitialize();
	// actually change the mode
	LT_HEAP_I();
	if (!modePriv(mode, sta, ap))
		return false;
	if (getMode() != mode) {
		LT_WM(WIFI, "Mode changed to %d (requested %d)", getMode(), mode);
	}
	return true;
}

bool WiFiClass::enableSTA(bool enable) {
	WiFiMode currentMode = getMode();
	if (((currentMode & WIFI_MODE_STA) != 0) != enable) {
		return mode((WiFiMode)(currentMode ^ WIFI_MODE_STA));
	}
	return true;
}

bool WiFiClass::enableAP(bool enable) {
	WiFiMode currentMode = getMode();
	if (((currentMode & WIFI_MODE_AP) != 0) != enable) {
		return mode((WiFiMode)(currentMode ^ WIFI_MODE_AP));
	}
	return true;
}

__attribute__((weak)) bool WiFiClass::setSleep(bool enable) {
	return false;
}

__attribute__((weak)) bool WiFiClass::getSleep() {
	return false;
}

__attribute__((weak)) bool WiFiClass::setTxPower(int power) {
	return false;
}

__attribute__((weak)) int WiFiClass::getTxPower() {
	return 0;
}

int WiFiClass::hostByName(const char *hostname, IPAddress &aResult) {
	aResult = hostByName(hostname);
	return true;
}

IPAddress WiFiClass::calculateNetworkID(IPAddress ip, IPAddress subnet) {
	IPAddress networkID;

	for (size_t i = 0; i < 4; i++)
		networkID[i] = subnet[i] & ip[i];

	return networkID;
}

IPAddress WiFiClass::calculateBroadcast(IPAddress ip, IPAddress subnet) {
	IPAddress broadcastIp;

	for (int i = 0; i < 4; i++)
		broadcastIp[i] = ~subnet[i] | ip[i];

	return broadcastIp;
}

uint8_t WiFiClass::calculateSubnetCIDR(IPAddress subnetMask) {
	uint8_t CIDR = 0;

	for (uint8_t i = 0; i < 4; i++) {
		if (subnetMask[i] == 0x80) // 128
			CIDR += 1;
		else if (subnetMask[i] == 0xC0) // 192
			CIDR += 2;
		else if (subnetMask[i] == 0xE0) // 224
			CIDR += 3;
		else if (subnetMask[i] == 0xF0) // 242
			CIDR += 4;
		else if (subnetMask[i] == 0xF8) // 248
			CIDR += 5;
		else if (subnetMask[i] == 0xFC) // 252
			CIDR += 6;
		else if (subnetMask[i] == 0xFE) // 254
			CIDR += 7;
		else if (subnetMask[i] == 0xFF) // 255
			CIDR += 8;
	}

	return CIDR;
}

String WiFiClass::macToString(uint8_t *mac) {
	char macStr[18]; // 6*2 + 5*':' + '\0'
	sprintf(macStr, "%02X:%02X:%02X:%02X:%02X:%02X", mac[0], mac[1], mac[2], mac[3], mac[4], mac[5]);
	return macStr;
}
