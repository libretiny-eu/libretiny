/* Copyright (c) WGM160P-LibreTiny port 2026-06-10. */

#include "WiFiPrivate.h"

// softAP is a Phase 2 follow-on; modePriv rejects AP mode. These stubs exist
// only because common code (printDiag, WiFiGeneric) references the symbols —
// every entry point reports "off/empty" without touching the radio.

bool WiFiClass::softAP(const char *ssid, const char *passphrase, int channel, bool ssidHidden, int maxClients) {
	(void)ssid;
	(void)passphrase;
	(void)channel;
	(void)ssidHidden;
	(void)maxClients;
	LT_EM(WIFI, "softAP not implemented on WF200 yet (Phase 2 follow-on)");
	return false;
}

bool WiFiClass::softAPConfig(IPAddress localIP, IPAddress gateway, IPAddress subnet) {
	(void)localIP;
	(void)gateway;
	(void)subnet;
	return false;
}

bool WiFiClass::softAPdisconnect(bool wifiOff) {
	(void)wifiOff;
	return false;
}

uint8_t WiFiClass::softAPgetStationNum() {
	return 0;
}

IPAddress WiFiClass::softAPIP() {
	return IPAddress();
}

IPAddress WiFiClass::softAPSubnetMask() {
	return IPAddress();
}

const char *WiFiClass::softAPgetHostname() {
	return "";
}

bool WiFiClass::softAPsetHostname(const char *hostname) {
	(void)hostname;
	return false;
}

uint8_t *WiFiClass::softAPmacAddress(uint8_t *mac) {
	memset(mac, 0, 6);
	return mac;
}

String WiFiClass::softAPmacAddress(void) {
	return String();
}

const String WiFiClass::softAPSSID(void) {
	return String();
}
