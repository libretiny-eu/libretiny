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

#include <Arduino.h>
#include <Events.h>
#include <api/IPAddress.h>
#include <api/IPv6Address.h>
#include <vector>

#include "WiFiType.h"

#include <WiFiClient.h>
#include <WiFiClientSecure.h>
#include <WiFiServer.h>
#include <WiFiUdp.h>

class WiFiClass {
  public:
	// must be public for WiFiEvents & WiFiScan static handlers
	void *data;
	WiFiScanData *scan = NULL;

  public: /* WiFi.cpp */
	WiFiClass();
	~WiFiClass();
	void printDiag(Print &dest);
	bool validate(const char *ssid, const char *passphrase);
	void dataInitialize();
	void dataFree();

  public: /* WiFiGeneric.cpp */
	bool mode(WiFiMode mode);
	bool modePriv(WiFiMode mode, WiFiModeAction sta, WiFiModeAction ap);
	WiFiMode getMode();
	WiFiStatus status();

	bool enableSTA(bool enable);
	bool enableAP(bool enable);

	bool setSleep(bool enable);
	bool getSleep();

	bool setTxPower(int power);
	int getTxPower();

	int hostByName(const char *hostname, IPAddress &aResult);
	IPAddress hostByName(const char *hostname);

	static IPAddress calculateNetworkID(IPAddress ip, IPAddress subnet);
	static IPAddress calculateBroadcast(IPAddress ip, IPAddress subnet);
	static uint8_t calculateSubnetCIDR(IPAddress subnetMask);
	static String macToString(uint8_t *mac);

  protected: /* WiFiEvents.cpp */
	static std::vector<EventHandler> handlers;

  public: /* WiFiEvents.cpp */
	uint16_t onEvent(EventCb callback, EventId eventId = ARDUINO_EVENT_MAX);
	uint16_t onEvent(EventFuncCb callback, EventId eventId = ARDUINO_EVENT_MAX);
	uint16_t onEvent(EventSysCb callback, EventId eventId = ARDUINO_EVENT_MAX);
	void removeEvent(EventCb callback, EventId eventId);
	void removeEvent(EventSysCb callback, EventId eventId);
	void removeEvent(uint16_t id);
	static void postEvent(EventId eventId, EventInfo eventInfo);

  public: /* WiFiSTA.cpp */
	WiFiStatus begin(
		const char *ssid,
		const char *passphrase = NULL,
		int32_t channel		   = 0,
		const uint8_t *bssid   = NULL,
		bool connect		   = true
	);
	WiFiStatus
	begin(char *ssid, char *passphrase = NULL, int32_t channel = 0, const uint8_t *bssid = NULL, bool connect = true);

	bool config(
		IPAddress localIP,
		IPAddress gateway,
		IPAddress subnet,
		IPAddress dns1 = (uint32_t)0x00000000,
		IPAddress dns2 = (uint32_t)0x00000000
	);

	bool reconnect(const uint8_t *bssid = NULL);
	bool disconnect(bool wifiOff = false);

	bool isConnected();

	bool setAutoReconnect(bool autoReconnect);
	bool getAutoReconnect();

	WiFiStatus waitForConnectResult(unsigned long timeout);

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
	bool setMacAddress(const uint8_t *mac);

	inline bool hostname(const String &aHostname) {
		return setHostname(aHostname.c_str());
	}

	const String SSID();
	const String psk();
	uint8_t *BSSID();
	String BSSIDstr();
	int32_t channel();
	int8_t RSSI();
	WiFiAuthMode getEncryption();

  public: /* WiFiScan.cpp */
	int16_t scanNetworks(
		bool async				 = false,
		bool showHidden			 = false,
		bool passive			 = false,
		uint32_t maxMsPerChannel = 300,
		uint8_t channel			 = 0
	);
	bool getNetworkInfo(
		uint8_t networkItem,
		String &ssid,
		WiFiAuthMode &encryptionType,
		int32_t &RSSI,
		uint8_t *&BSSID,
		int32_t &channel
	);

	int16_t scanComplete();
	uint8_t scanAlloc(uint8_t count);
	void scanInit();
	void scanDelete();

	String SSID(uint8_t networkItem);
	WiFiAuthMode encryptionType(uint8_t networkItem);
	int32_t RSSI(uint8_t networkItem);
	uint8_t *BSSID(uint8_t networkItem);
	String BSSIDstr(uint8_t networkItem);
	int32_t channel(uint8_t networkItem);

  public: /* WiFiAP.cpp */
	bool softAP(
		const char *ssid, const char *passphrase = NULL, int channel = 1, bool ssidHidden = false, int maxClients = 4
	);
	bool softAPConfig(IPAddress localIP, IPAddress gateway, IPAddress subnet);
	bool softAPdisconnect(bool wifiOff = false);

	uint8_t softAPgetStationNum();

	IPAddress softAPIP();
	IPAddress softAPBroadcastIP();
	IPAddress softAPNetworkID();
	uint8_t softAPSubnetCIDR();
	IPAddress softAPSubnetMask();
	bool softAPenableIpV6();
	IPv6Address softAPIPv6();
	const char *softAPgetHostname();
	bool softAPsetHostname(const char *hostname);
	uint8_t *softAPmacAddress(uint8_t *mac);
	String softAPmacAddress(void);
	const String softAPSSID(void);
};

extern WiFiClass WiFi;
extern WiFiClass *pWiFi;
