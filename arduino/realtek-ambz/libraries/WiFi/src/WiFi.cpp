/*
  WiFi.cpp - Library for Arduino Wifi shield.
  Copyright (c) 2011-2014 Arduino LLC.  All right reserved.

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

#include "WiFi.h"

#include <inttypes.h>

#include "wifi_drv.h"

WiFiClass::WiFiClass() {
}

void WiFiClass::init() {
    WiFiDrv::wifiDriverInit();
}

char* WiFiClass::firmwareVersion() {
    return WiFiDrv::getFwVersion();
}

int WiFiClass::begin(char* ssid) {
    uint8_t status = WL_IDLE_STATUS;

    WiFiDrv::wifiDriverInit();

    if (WiFiDrv::wifiSetNetwork(ssid, strlen(ssid)) != WL_FAILURE) {
        status = WiFiDrv::getConnectionStatus();
    } else {
        status = WL_CONNECT_FAILED;
    }
    return status;
}

int WiFiClass::begin(char* ssid, uint8_t key_idx, const char *key) {
    uint8_t status = WL_IDLE_STATUS;

    WiFiDrv::wifiDriverInit();

    // set encryption key
    if (WiFiDrv::wifiSetKey(ssid, strlen(ssid), key_idx, key, strlen(key)) != WL_FAILURE) {
        status = WiFiDrv::getConnectionStatus();
    } else {
        status = WL_CONNECT_FAILED;
    }
    return status;
}

int WiFiClass::begin(char* ssid, const char *passphrase) {
	uint8_t status = WL_IDLE_STATUS;

    WiFiDrv::wifiDriverInit();

    // set passphrase
    if (WiFiDrv::wifiSetPassphrase(ssid, strlen(ssid), passphrase, strlen(passphrase))!= WL_FAILURE) {
        status = WiFiDrv::getConnectionStatus();
    } else {
        status = WL_CONNECT_FAILED;
    }
    return status;
}

int WiFiClass::disconnect() {
    return WiFiDrv::disconnect();
}

uint8_t* WiFiClass::macAddress(uint8_t* mac) {
    uint8_t* _mac = WiFiDrv::getMacAddress();
    memcpy(mac, _mac, WL_MAC_ADDR_LENGTH);
    return mac;
}

IPAddress WiFiClass::localIP() {
    IPAddress ret;
    WiFiDrv::getIpAddress(ret);
    return ret;
}

IPAddress WiFiClass::subnetMask() {
    IPAddress ret;
    WiFiDrv::getSubnetMask(ret);
    return ret;
}

IPAddress WiFiClass::gatewayIP() {
    IPAddress ret;
    WiFiDrv::getGatewayIP(ret);
    return ret;
}

char* WiFiClass::SSID() {
    return WiFiDrv::getCurrentSSID();
}

uint8_t* WiFiClass::BSSID(uint8_t* bssid) {
    uint8_t* _bssid = WiFiDrv::getCurrentBSSID();
    memcpy(bssid, _bssid, WL_MAC_ADDR_LENGTH);
    return bssid;
}

int32_t WiFiClass::RSSI() {
    return WiFiDrv::getCurrentRSSI();
}

uint8_t WiFiClass::encryptionType() {
    return WiFiDrv::getCurrentEncryptionType();
}


int8_t WiFiClass::scanNetworks() {
    uint8_t attempts = 10;
    uint8_t numOfNetworks = 0;

    if (WiFiDrv::startScanNetworks() == WL_FAILURE) {
        return WL_FAILURE;
    }
    do {
        delay(2000);
        numOfNetworks = WiFiDrv::getScanNetworks();
    } while (( numOfNetworks == 0)&&(--attempts>0));
    return numOfNetworks;
}

char* WiFiClass::SSID(uint8_t networkItem) {
    return WiFiDrv::getSSIDNetoworks(networkItem);
}

int32_t WiFiClass::RSSI(uint8_t networkItem) {
    return WiFiDrv::getRSSINetoworks(networkItem);
}

uint8_t WiFiClass::encryptionType(uint8_t networkItem) {
    return WiFiDrv::getEncTypeNetowrks(networkItem);
}

uint32_t WiFiClass::encryptionTypeEx(uint8_t networkItem) {
    return WiFiDrv::getEncTypeNetowrksEx(networkItem);
}

uint8_t WiFiClass::status() {
    return WiFiDrv::getConnectionStatus();
}

int WiFiClass::hostByName(const char* aHostname, IPAddress& aResult) {
    return WiFiDrv::getHostByName(aHostname, aResult);
}

int WiFiClass::apbegin(char* ssid, char* channel) {
    uint8_t status = WL_IDLE_STATUS;

    if (WiFiDrv::apSetNetwork(ssid, strlen(ssid)) != WL_FAILURE) {
        WiFiDrv::apSetChannel(channel);

        if (WiFiDrv::apActivate() != WL_FAILURE) {
            status = WL_CONNECTED;
        } else {
            status = WL_CONNECT_FAILED;
        }
    } else {
        status = WL_CONNECT_FAILED;
    }
    return status;
}

int WiFiClass::apbegin(char* ssid, char* password, char* channel) {
    uint8_t status = WL_IDLE_STATUS;

    if (WiFiDrv::apSetNetwork(ssid, strlen(ssid)) != WL_FAILURE) {
        WiFiDrv::apSetPassphrase(password, strlen(password));
        WiFiDrv::apSetChannel(channel);

        if(WiFiDrv::apActivate() != WL_FAILURE) {
            status = WL_CONNECTED;
        } else {
            status = WL_CONNECT_FAILED;
        }
    } else {
        status = WL_CONNECT_FAILED;
    }

    return status;
}

int WiFiClass::disablePowerSave() {
    return WiFiDrv::disablePowerSave();
}

WiFiClass WiFi;
