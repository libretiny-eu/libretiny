/**
 *
 * @file WiFiMulti.cpp
 * @date 16.05.2015
 * @author Markus Sattler
 *
 * Copyright (c) 2015 Markus Sattler. All rights reserved.
 * This file is part of the esp8266 core for Arduino environment.
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
 *
 */

#if LT_ARD_HAS_WIFI

#include "WiFiMulti.h"
#include <Arduino.h>
#include <limits.h>
#include <string.h>

WiFiMulti::WiFiMulti() {}

WiFiMulti::~WiFiMulti() {
	for (uint32_t i = 0; i < APlist.size(); i++) {
		WifiAPlist_t entry = APlist[i];
		if (entry.ssid) {
			free(entry.ssid);
		}
		if (entry.passphrase) {
			free(entry.passphrase);
		}
	}
	APlist.clear();
}

bool WiFiMulti::addAP(const char *ssid, const char *passphrase) {
	WifiAPlist_t newAP;

	if (!ssid || *ssid == 0x00 || strlen(ssid) > 31) {
		// fail SSID too long or missing!
		LT_EM(WIFI, "SSID missing or too long");
		return false;
	}

	if (passphrase && strlen(passphrase) > 64) {
		// fail passphrase too long!
		LT_EM(WIFI, "Passphrase too long");
		return false;
	}

	newAP.ssid = strdup(ssid);

	if (!newAP.ssid) {
		LT_EM(WIFI, "Fail newAP.ssid == 0");
		return false;
	}

	if (passphrase && *passphrase != 0x00) {
		newAP.passphrase = strdup(passphrase);
		if (!newAP.passphrase) {
			LT_EM(WIFI, "Fail newAP.passphrase == 0");
			free(newAP.ssid);
			return false;
		}
	} else {
		newAP.passphrase = NULL;
	}

	APlist.push_back(newAP);
	LT_VM(WIFI, "Add SSID: %s", newAP.ssid);
	return true;
}

uint8_t WiFiMulti::run(uint32_t connectTimeout) {
	int8_t scanResult;
	uint8_t status = WiFi.status();
	if (status == WL_CONNECTED) {
		for (uint32_t x = 0; x < APlist.size(); x++) {
			if (WiFi.SSID() == APlist[x].ssid) {
				return status;
			}
		}
		WiFi.disconnect(false);
		delay(10);
		status = WiFi.status();
	}

	scanResult = WiFi.scanNetworks();
	if (scanResult == WIFI_SCAN_RUNNING) {
		// scan is running
		return WL_NO_SSID_AVAIL;
	} else if (scanResult >= 0) {
		// scan done analyze
		WifiAPlist_t bestNetwork{NULL, NULL};
		int bestNetworkDb = INT_MIN;
		uint8_t bestBSSID[6];
		int32_t bestChannel = 0;

		LT_IM(WIFI, "Scan finished");

		if (scanResult == 0) {
			LT_IM(WIFI, "No networks found");
		} else {
			LT_IM(WIFI, "%d networks found", scanResult);
			for (int8_t i = 0; i < scanResult; ++i) {

				String ssid_scan;
				int32_t rssi_scan;
				WiFiAuthMode sec_scan;
				uint8_t *BSSID_scan;
				int32_t chan_scan;

				WiFi.getNetworkInfo(i, ssid_scan, sec_scan, rssi_scan, BSSID_scan, chan_scan);

				bool known = false;
				for (uint32_t x = APlist.size(); x > 0; x--) {
					WifiAPlist_t entry = APlist[x - 1];

					if (ssid_scan == entry.ssid) { // SSID match
						known = true;
						if (rssi_scan > bestNetworkDb) { // best network
							if (sec_scan == WIFI_AUTH_OPEN ||
								entry.passphrase) { // check for passphrase if not open wlan
								bestNetworkDb = rssi_scan;
								bestChannel	  = chan_scan;
								memcpy((void *)&bestNetwork, (void *)&entry, sizeof(bestNetwork));
								memcpy((void *)&bestBSSID, (void *)BSSID_scan, sizeof(bestBSSID));
							}
						}
						break;
					}
				}

				if (known) {
					LT_DM(
						WIFI,
						" --->   %d: [%d][%02X:%02X:%02X:%02X:%02X:%02X] %s (%d) %c",
						i,
						chan_scan,
						BSSID_scan[0],
						BSSID_scan[1],
						BSSID_scan[2],
						BSSID_scan[3],
						BSSID_scan[4],
						BSSID_scan[5],
						ssid_scan.c_str(),
						rssi_scan,
						(sec_scan == WIFI_AUTH_OPEN) ? ' ' : '*'
					);
				} else {
					LT_DM(
						WIFI,
						"       %d: [%d][%02X:%02X:%02X:%02X:%02X:%02X] %s (%d) %c",
						i,
						chan_scan,
						BSSID_scan[0],
						BSSID_scan[1],
						BSSID_scan[2],
						BSSID_scan[3],
						BSSID_scan[4],
						BSSID_scan[5],
						ssid_scan.c_str(),
						rssi_scan,
						(sec_scan == WIFI_AUTH_OPEN) ? ' ' : '*'
					);
				}
			}
		}

		// clean up ram
		WiFi.scanDelete();

		if (bestNetwork.ssid) {
			LT_IM(
				WIFI,
				"Connecting to BSSID: %02X:%02X:%02X:%02X:%02X:%02X SSID: %s Channel: %d (%d)",
				bestBSSID[0],
				bestBSSID[1],
				bestBSSID[2],
				bestBSSID[3],
				bestBSSID[4],
				bestBSSID[5],
				bestNetwork.ssid,
				bestChannel,
				bestNetworkDb
			);

			WiFi.begin(bestNetwork.ssid, bestNetwork.passphrase, bestChannel, bestBSSID);
			status = WiFi.status();

			auto startTime = millis();
			// wait for connection, fail, or timeout
			while (status != WL_CONNECTED && status != WL_NO_SSID_AVAIL && status != WL_CONNECT_FAILED &&
				   (millis() - startTime) <= connectTimeout) {
				delay(10);
				status = WiFi.status();
			}

			IPAddress ip;
			switch (status) {
				case WL_CONNECTED:
					LT_IM(WIFI, "Connecting done");
					LT_DM(WIFI, "SSID: %s", WiFi.SSID().c_str());
					// TODO fix this after implementing IP format for printf()
					ip = WiFi.localIP();
					LT_DM(WIFI, "IP: %u.%u.%u.%u", ip[0], ip[1], ip[2], ip[3]);
					LT_DM(WIFI, "MAC: %s", WiFi.BSSIDstr().c_str());
					LT_DM(WIFI, "Channel: %d", WiFi.channel());
					break;
				case WL_NO_SSID_AVAIL:
					LT_EM(WIFI, "Connecting failed; AP not found");
					break;
				case WL_CONNECT_FAILED:
					LT_EM(WIFI, "Connecting failed");
					break;
				default:
					LT_EM(WIFI, "Connecting failed (%d)", status);
					break;
			}
		} else {
			LT_EM(WIFI, "No matching network found!");
		}
	} else {
		// start scan
		LT_VM(WIFI, "Delete old wifi config...");
		WiFi.disconnect();

		LT_DM(WIFI, "Start scan");
		// scan wifi async mode
		WiFi.scanNetworks(true);
	}

	return status;
}

#endif // LT_ARD_HAS_WIFI
