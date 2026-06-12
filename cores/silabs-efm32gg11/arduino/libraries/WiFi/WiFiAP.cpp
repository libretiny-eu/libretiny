/* Copyright (c) WGM160P-LibreTiny port 2026-06-10. */

#include "WiFiPrivate.h"

// softAP public API. The radio bring-up lives in modePriv() (WiFiGeneric.cpp,
// Task 3); enableAP(true/false) routes mode()->modePriv() to start/stop the
// AP using the DATA->ap* fields these entry points stage.
//
// Byte order: DATA->apLocalIP / apSubnet are stored in network byte order, the
// same form ip4_addr_t.addr wants. (uint32_t)IPAddress already yields that form
// (STA config()/localIP() round-trip through ip4_addr_t.addr identically), so
// softAPConfig stores it raw and softAPIP feeds it straight back to IPAddress().

bool WiFiClass::softAP(const char *ssid, const char *passphrase, int channel, bool ssidHidden, int maxClients) {
	// NOTE: calling softAP() again while the AP is already active re-stages the
	// config fields but does NOT re-apply them to the radio (enableAP(true) is a
	// no-op when the AP bit is already set). To change SSID/passphrase/channel of
	// a running AP, call softAPdisconnect() first. (Live reconfigure: softAP T9.)
	(void)ssidHidden; // Task 3 start_ap hardcodes hidden=0
	(void)maxClients; // and client isolation=0; out of scope here
	if (!ssid || !ssid[0] || strlen(ssid) > 32)
		return false;
	bool open = (passphrase == NULL || passphrase[0] == '\0');
	if (!open && strlen(passphrase) < 8)
		return false; // WPA2-PSK minimum
	// Guard the alias case (caller passing our own apSsid back) before freeing.
	if (DATA->apSsid && DATA->apSsid != ssid)
		free(DATA->apSsid);
	DATA->apSsid = strdup(ssid);
	free(DATA->apPassword);
	DATA->apPassword = open ? NULL : strdup(passphrase);
	DATA->apChannel	 = channel;
	return enableAP(true); // -> modePriv ap-enable
}

bool WiFiClass::softAPConfig(IPAddress localIP, IPAddress gateway, IPAddress subnet) {
	(void)gateway; // gateway is the device itself (ap_create uses ip as gw)
	DATA->apLocalIP = (uint32_t)localIP;
	DATA->apSubnet	= (uint32_t)subnet;
	return true; // applied at softAP() bring-up
}

bool WiFiClass::softAPdisconnect(bool wifiOff) {
	(void)wifiOff;
	return enableAP(false);
}

uint8_t WiFiClass::softAPgetStationNum() {
	return DATA->apClientCount;
}

IPAddress WiFiClass::softAPIP() {
	return IPAddress(DATA->apLocalIP ? DATA->apLocalIP : (uint32_t)IPAddress(192, 168, 4, 1));
}

IPAddress WiFiClass::softAPSubnetMask() {
	return IPAddress(DATA->apSubnet ? DATA->apSubnet : (uint32_t)IPAddress(255, 255, 255, 0));
}

const char *WiFiClass::softAPgetHostname() {
	return "";
}

bool WiFiClass::softAPsetHostname(const char *hostname) {
	(void)hostname;
	return false;
}

uint8_t *WiFiClass::softAPmacAddress(uint8_t *mac) {
	struct netif *nif = lt_wf200_netif_ap();
	if (nif)
		memcpy(mac, nif->hwaddr, 6);
	else
		memset(mac, 0, 6);
	return mac;
}

String WiFiClass::softAPmacAddress(void) {
	uint8_t mac[6];
	softAPmacAddress(mac);
	return macToString(mac);
}

const String WiFiClass::softAPSSID(void) {
	return DATA->apSsid ? String(DATA->apSsid) : String();
}
