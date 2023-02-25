/*
 ESP8266WiFiType.h - esp8266 Wifi support.
 Copyright (c) 2011-2014 Arduino.  All right reserved.
 Modified by Ivan Grokhotkov, December 2014
 Reworked by Markus Sattler, December 2015

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

#define WIFI_SCAN_RUNNING (-1)
#define WIFI_SCAN_FAILED  (-2)

#define WiFiMode_t	 wifi_mode_t
#define WiFiMode	 wifi_mode_t
#define WiFiStatus	 wl_status_t
#define WiFiAuthMode wifi_auth_mode_t

#define WIFI_OFF	WIFI_MODE_NULL
#define WIFI_STA	WIFI_MODE_STA
#define WIFI_AP		WIFI_MODE_AP
#define WIFI_AP_STA WIFI_MODE_APSTA

#define WiFiEvent_t		arduino_event_id_t
#define WiFiEventInfo_t arduino_event_info_t
#define WiFiEventId_t	uint16_t

typedef struct {
	uint8_t addr[6];
} WiFiMacAddr;

struct esp_ip6_addr {
	uint32_t addr[4];
	uint8_t zone;
};

struct esp_ip4_addr {
	uint32_t addr;
};

typedef struct esp_ip4_addr esp_ip4_addr_t;
typedef struct esp_ip6_addr esp_ip6_addr_t;

typedef enum {
	WIFI_MODE_NULL = 0, /**< null mode */
	WIFI_MODE_STA,		/**< WiFi station mode */
	WIFI_MODE_AP,		/**< WiFi soft-AP mode */
	WIFI_MODE_APSTA,	/**< WiFi station + soft-AP mode */
	WIFI_MODE_MAX
} wifi_mode_t;

typedef enum {
	WL_NO_SHIELD	   = 255, // for compatibility with WiFi Shield library
	WL_IDLE_STATUS	   = 0,
	WL_NO_SSID_AVAIL   = 1,
	WL_SCAN_COMPLETED  = 2,
	WL_CONNECTED	   = 3,
	WL_CONNECT_FAILED  = 4,
	WL_CONNECTION_LOST = 5,
	WL_DISCONNECTED	   = 6,
} wl_status_t;

typedef enum {
	WIFI_AUTH_OPEN = 0,		   /**< authenticate mode : open */
	WIFI_AUTH_WEP,			   /**< authenticate mode : WEP */
	WIFI_AUTH_WPA_PSK,		   /**< authenticate mode : WPA_PSK */
	WIFI_AUTH_WPA2_PSK,		   /**< authenticate mode : WPA2_PSK */
	WIFI_AUTH_WPA_WPA2_PSK,	   /**< authenticate mode : WPA_WPA2_PSK */
	WIFI_AUTH_WPA2_ENTERPRISE, /**< authenticate mode : WPA2_ENTERPRISE */
	WIFI_AUTH_WPA3_PSK,		   /**< authenticate mode : WPA3_PSK */
	WIFI_AUTH_WPA2_WPA3_PSK,   /**< authenticate mode : WPA2_WPA3_PSK */
	WIFI_AUTH_WAPI_PSK,		   /**< authenticate mode : WAPI_PSK */
	WIFI_AUTH_WPA,
	WIFI_AUTH_WPA2,
	WIFI_AUTH_AUTO	  = 200,
	WIFI_AUTH_INVALID = 255,
	WIFI_AUTH_MAX
} wifi_auth_mode_t;

typedef enum {
	WIFI_REASON_UNSPECIFIED				 = 1,
	WIFI_REASON_AUTH_EXPIRE				 = 2,
	WIFI_REASON_AUTH_LEAVE				 = 3,
	WIFI_REASON_ASSOC_EXPIRE			 = 4,
	WIFI_REASON_ASSOC_TOOMANY			 = 5,
	WIFI_REASON_NOT_AUTHED				 = 6,
	WIFI_REASON_NOT_ASSOCED				 = 7,
	WIFI_REASON_ASSOC_LEAVE				 = 8,
	WIFI_REASON_ASSOC_NOT_AUTHED		 = 9,
	WIFI_REASON_DISASSOC_PWRCAP_BAD		 = 10,
	WIFI_REASON_DISASSOC_SUPCHAN_BAD	 = 11,
	WIFI_REASON_BSS_TRANSITION_DISASSOC	 = 12,
	WIFI_REASON_IE_INVALID				 = 13,
	WIFI_REASON_MIC_FAILURE				 = 14,
	WIFI_REASON_4WAY_HANDSHAKE_TIMEOUT	 = 15,
	WIFI_REASON_GROUP_KEY_UPDATE_TIMEOUT = 16,
	WIFI_REASON_IE_IN_4WAY_DIFFERS		 = 17,
	WIFI_REASON_GROUP_CIPHER_INVALID	 = 18,
	WIFI_REASON_PAIRWISE_CIPHER_INVALID	 = 19,
	WIFI_REASON_AKMP_INVALID			 = 20,
	WIFI_REASON_UNSUPP_RSN_IE_VERSION	 = 21,
	WIFI_REASON_INVALID_RSN_IE_CAP		 = 22,
	WIFI_REASON_802_1X_AUTH_FAILED		 = 23,
	WIFI_REASON_CIPHER_SUITE_REJECTED	 = 24,
	WIFI_REASON_INVALID_PMKID			 = 53,
	WIFI_REASON_BEACON_TIMEOUT			 = 200,
	WIFI_REASON_NO_AP_FOUND				 = 201,
	WIFI_REASON_AUTH_FAIL				 = 202,
	WIFI_REASON_ASSOC_FAIL				 = 203,
	WIFI_REASON_HANDSHAKE_TIMEOUT		 = 204,
	WIFI_REASON_CONNECTION_FAIL			 = 205,
	WIFI_REASON_AP_TSF_RESET			 = 206,
	WIFI_REASON_ROAMING					 = 207,
} wifi_err_reason_t;

typedef struct {
	char *ssid;
	WiFiAuthMode auth;
	int32_t rssi;
	WiFiMacAddr bssid;
	int32_t channel;
} WiFiScanAP;

typedef struct {
	bool running		  = false;
	unsigned long timeout = 0;
	uint8_t count		  = 0;
	WiFiScanAP *ap		  = NULL;
} WiFiScanData;

typedef enum {
	WLMODE_NONE	   = 0,
	WLMODE_DISABLE = 1,
	WLMODE_ENABLE  = 2,
} WiFiModeAction;
