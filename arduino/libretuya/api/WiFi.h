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

#include "WiFiType.h"

// TODO wifi events
// TODO WiFiMulti library

class IWiFiClass {
  public:
	virtual void printDiag(Print &dest) = 0;
};

class IWiFiGenericClass {
  public:
	virtual int32_t channel(void)	 = 0;
	virtual bool mode(WiFiMode mode) = 0;
	virtual WiFiMode getMode()		 = 0;
	virtual WiFiStatus status()		 = 0;

	virtual bool enableSTA(bool enable) = 0;
	virtual bool enableAP(bool enable)	= 0;

	virtual bool setSleep(bool enable) = 0;
	virtual bool getSleep()			   = 0;

	virtual bool setTxPower(int power) = 0;
	virtual int getTxPower()		   = 0;

	virtual int hostByName(const char *hostname, IPAddress &aResult) {
		aResult = hostByName(hostname);
		return true;
	}

	virtual IPAddress hostByName(const char *hostname) = 0;

	static IPAddress calculateNetworkID(IPAddress ip, IPAddress subnet);
	static IPAddress calculateBroadcast(IPAddress ip, IPAddress subnet);
	static uint8_t calculateSubnetCIDR(IPAddress subnetMask);
	static String macToString(uint8_t *mac);
};

class IWiFiSTAClass {
  public:
	virtual WiFiStatus begin(
		const char *ssid,
		const char *passphrase = NULL,
		int32_t channel		   = 0,
		const uint8_t *bssid   = NULL,
		bool connect		   = true
	) = 0;
	virtual WiFiStatus begin(
		char *ssid, char *passphrase = NULL, int32_t channel = 0, const uint8_t *bssid = NULL, bool connect = true
	) = 0;

	virtual bool config(
		IPAddress localIP,
		IPAddress gateway,
		IPAddress subnet,
		IPAddress dns1 = (uint32_t)0x00000000,
		IPAddress dns2 = (uint32_t)0x00000000
	) = 0;

	virtual bool reconnect()					  = 0;
	virtual bool disconnect(bool wifiOff = false) = 0;

	virtual bool isConnected();

	virtual bool setAutoReconnect(bool autoReconnect) = 0;
	virtual bool getAutoReconnect()					  = 0;

	virtual WiFiStatus waitForConnectResult(unsigned long timeout) = 0;

	virtual IPAddress localIP()					   = 0;
	virtual uint8_t *macAddress(uint8_t *mac)	   = 0;
	virtual String macAddress()					   = 0;
	virtual IPAddress subnetMask()				   = 0;
	virtual IPAddress gatewayIP()				   = 0;
	virtual IPAddress dnsIP(uint8_t dns_no = 0)	   = 0;
	virtual IPAddress broadcastIP()				   = 0;
	virtual IPAddress networkID()				   = 0;
	virtual uint8_t subnetCIDR()				   = 0;
	virtual bool enableIpV6()					   = 0;
	virtual IPv6Address localIPv6()				   = 0;
	virtual const char *getHostname()			   = 0;
	virtual bool setHostname(const char *hostname) = 0;
	virtual bool setMacAddress(const uint8_t *mac) = 0;

	inline bool hostname(const String &aHostname) {
		return setHostname(aHostname.c_str());
	}

	virtual const String SSID()			 = 0;
	virtual const String psk()			 = 0;
	virtual uint8_t *BSSID()			 = 0;
	virtual String BSSIDstr()			 = 0;
	virtual int8_t RSSI()				 = 0;
	virtual WiFiAuthMode getEncryption() = 0;
};

class IWiFiScanClass {
  public:
	virtual int16_t scanNetworks(
		bool async				 = false,
		bool showHidden			 = false,
		bool passive			 = false,
		uint32_t maxMsPerChannel = 300,
		uint8_t channel			 = 0
	) = 0;
	virtual bool getNetworkInfo(
		uint8_t networkItem,
		String &ssid,
		WiFiAuthMode &encryptionType,
		int32_t &RSSI,
		uint8_t *&BSSID,
		int32_t &channel
	) = 0;

	virtual int16_t scanComplete() = 0;
	virtual void scanDelete()	   = 0;

	virtual String SSID(uint8_t networkItem)				 = 0;
	virtual WiFiAuthMode encryptionType(uint8_t networkItem) = 0;
	virtual int32_t RSSI(uint8_t networkItem)				 = 0;
	virtual uint8_t *BSSID(uint8_t networkItem)				 = 0;
	virtual String BSSIDstr(uint8_t networkItem)			 = 0;
	virtual int32_t channel(uint8_t networkItem)			 = 0;
};

class IWiFiAPClass {
  public:
	virtual bool softAP(
		const char *ssid, const char *passphrase = NULL, int channel = 1, bool ssidHidden = false, int maxClients = 4
	)																				  = 0;
	virtual bool softAPConfig(IPAddress localIP, IPAddress gateway, IPAddress subnet) = 0;
	virtual bool softAPdisconnect(bool wifiOff = false)								  = 0;

	virtual uint8_t softAPgetStationNum() = 0;

	virtual IPAddress softAPIP()						 = 0;
	virtual IPAddress softAPBroadcastIP()				 = 0;
	virtual IPAddress softAPNetworkID()					 = 0;
	virtual uint8_t softAPSubnetCIDR()					 = 0;
	virtual bool softAPenableIpV6()						 = 0;
	virtual IPv6Address softAPIPv6()					 = 0;
	virtual const char *softAPgetHostname()				 = 0;
	virtual bool softAPsetHostname(const char *hostname) = 0;
	virtual uint8_t *softAPmacAddress(uint8_t *mac)		 = 0;
	virtual String softAPmacAddress(void)				 = 0;
	virtual const String softAPSSID(void)				 = 0;
};
