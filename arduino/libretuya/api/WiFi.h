/*
 WiFi.h - esp32 Wifi support.
 Based on WiFi.h from Arduino WiFi shield library.
 Copyright (c) 2011-2014 Arduino.  All right reserved.
 Modified by Ivan Grokhotkov, December 2014

 This library is free software; you can redistribute it and/or
 modify it under the terms of the GNU Lesser General Public
 License as published by the Free Software Foundation; either
 version 2.1 of the License, or (at your option) any later version.

 This library is distributed in the hope that it will be useful,
 but WITHOUT ANY WARRANTY; without even the implied warranty of
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 Lesser General Public License for more details.

 You should have received a copy of the GNU Lesser General Public
 License along with this library; if not, write to the Free Software
 Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
 */

#pragma once

#include <stdint.h>

#include <IPv6Address.h>
#include <api/IPAddress.h>
#include <api/Print.h>

#include "WiFiClient.h"
#include "WiFiServer.h"
#include "WiFiType.h"
#include "WiFiUdp.h"

// TODO wifi events
// TODO WiFiMulti library

class IWiFi {
  public:
	void printDiag(Print &dest);

	// WiFiGenericClass
	int32_t channel(void);
	static bool mode(WiFiMode mode);
	static WiFiMode getMode();

	bool enableSTA(bool enable);
	bool enableAP(bool enable);

	bool setSleep(bool enable);
	bool getSleep();

	bool setTxPower(int power);
	int getTxPower();

	static int hostByName(const char *aHostname, IPAddress &aResult);

	static IPAddress calculateNetworkID(IPAddress ip, IPAddress subnet);
	static IPAddress calculateBroadcast(IPAddress ip, IPAddress subnet);
	static uint8_t calculateSubnetCIDR(IPAddress subnetMask);

	// WiFiSTAClass
	WiFiStatus begin(
		const char *ssid,
		const char *passphrase = NULL,
		int32_t channel		   = 0,
		const uint8_t *bssid   = NULL,
		bool connect		   = true,
	);
	WiFiStatus
	begin(char *ssid, char *passphrase = NULL, int32_t channel = 0, const uint8_t *bssid = NULL, bool connect = true, );
	WiFiStatus begin();

	bool config(
		IPAddress local_ip,
		IPAddress gateway,
		IPAddress subnet,
		IPAddress dns1 = (uint32_t)0x00000000,
		IPAddress dns2 = (uint32_t)0x00000000,
	);

	bool reconnect();
	bool disconnect(bool wifioff = false, bool eraseap = false);

	bool isConnected();

	bool setAutoConnect(bool autoConnect);
	bool getAutoConnect();

	bool setAutoReconnect(bool autoReconnect);
	bool getAutoReconnect();

	uint8_t waitForConnectResult();

	IPAddress localIP();
	uint8_t *macAddress(uint8_t *mac);
	String macAddress();
	IPAddress subnetMask();
	IPAddress gatewayIP();
	IPAddress dnsIP(uint8_t dns_no = 0);
	IPAddress broadcastIP();
	IPAddress networkID();
	uint8_t subnetCIDR();
	bool enableIpV6();
	IPv6Address localIPv6();
	const char *getHostname();
	bool setHostname(const char *hostname);

	bool hostname(const String &aHostname) {
		return setHostname(aHostname.c_str());
	}

	static WiFiStatus status();
	String SSID() const;
	String psk() const;
	uint8_t *BSSID();
	String BSSIDstr();
	int8_t RSSI();

	// WiFiScanClass
	int16_t scanNetworks(
		bool async				 = false,
		bool show_hidden		 = false,
		bool passive			 = false,
		uint32_t max_ms_per_chan = 300,
		uint8_t channel			 = 0,
	);
	bool getNetworkInfo(
		uint8_t networkItem,
		String &ssid,
		WiFiAuthMode &encryptionType,
		int32_t &RSSI,
		uint8_t *&BSSID,
		int32_t &channel,
	);

	int16_t scanComplete();
	void scanDelete();

	String SSID(uint8_t networkItem);
	WiFiAuthMode encryptionType(uint8_t networkItem);
	int32_t RSSI(uint8_t networkItem);
	uint8_t *BSSID(uint8_t networkItem);
	String BSSIDstr(uint8_t networkItem);
	int32_t channel(uint8_t networkItem);

	static void *getScanInfoByIndex(int i) {
		return _getScanInfoByIndex(i);
	};

	// WiFiAPClass
	bool softAP(
		const char *ssid, const char *passphrase = NULL, int channel = 1, int ssid_hidden = 0, int max_connection = 4
	);
	bool softAPConfig(IPAddress local_ip, IPAddress gateway, IPAddress subnet);
	bool softAPdisconnect(bool wifioff = false);

	uint8_t softAPgetStationNum();

	IPAddress softAPIP();
	IPAddress softAPBroadcastIP();
	IPAddress softAPNetworkID();
	uint8_t softAPSubnetCIDR();
	bool softAPenableIpV6();
	IPv6Address softAPIPv6();
	const char *softAPgetHostname();
	bool softAPsetHostname(const char *hostname);
	uint8_t *softAPmacAddress(uint8_t *mac);
	String softAPmacAddress(void);
	String softAPSSID(void) const;
};

extern WiFiClass WiFi;
