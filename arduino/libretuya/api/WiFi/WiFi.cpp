/* Copyright (c) Kuba Szczodrzyński 2022-04-25. */

#include "WiFi.h"

void WiFiClass::printDiag(Print &dest) {
	const char *modes[] = {"NULL", "STA", "AP", "STA+AP"};
	const char *enc[]	= {"Open", "WEP", "WPA PSK", "WPA2 PSK", "WPA/WPA2", "WPA", "WPA2"};

	dest.print("Mode: ");
	dest.println(modes[getMode()]);

	if (getMode() & WIFI_MODE_STA) {
		dest.println("-- Station --");
		dest.print("SSID: ");
		if (isConnected()) {
			dest.println(SSID());
			dest.print("Channel: ");
			dest.println(channel());
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
		} else {
			dest.println("disconnected");
		}
	}

	if (getMode() & WIFI_MODE_AP) {
		dest.println("-- Access Point --");
		dest.print("SSID: ");
		if (softAPSSID().length()) {
			dest.println(softAPSSID());
			dest.print("IP: ");
			dest.println(softAPIP());
			dest.print("MAC: ");
			dest.println(softAPmacAddress());
			dest.print("Hostname: ");
			dest.println(softAPgetHostname());
		} else {
			dest.println("disconnected");
		}
	}
}

bool WiFiClass::validate(const char *ssid, const char *passphrase) {
	if (!ssid || *ssid == 0x00 || strlen(ssid) > 32) {
		LT_WM(WIFI, "SSID not specified or too long");
		return false;
	}
	if (passphrase) {
		uint16_t length = strlen(passphrase);
		if (length < 8) {
			LT_WM(WIFI, "Passphrase too short (%u)", length);
			return false;
		}
		if (length > 63) {
			LT_WM(WIFI, "Passphrase too long (%u)", length);
			return false;
		}
	}
	return true;
}

__attribute__((weak)) void WiFiClass::dataInitialize() {}

__attribute__((weak)) void WiFiClass::dataFree() {}

WiFiClass WiFi;
WiFiClass *pWiFi = NULL;
