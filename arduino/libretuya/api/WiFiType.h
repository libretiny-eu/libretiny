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

#define WIFI_SCAN_RUNNING (-1)
#define WIFI_SCAN_FAILED  (-2)

#define WiFiMode_t	 wl_mode_t
#define WiFiMode	 wl_mode_t
#define WiFiStatus	 wl_status_t
#define WiFiAuthMode wl_auth_mode_t

#define WIFI_OFF	WIFI_MODE_NULL
#define WIFI_STA	WIFI_MODE_STA
#define WIFI_AP		WIFI_MODE_AP
#define WIFI_AP_STA WIFI_MODE_APSTA

typedef enum {
	WIFI_MODE_NULL = 0,
	WIFI_MODE_STA,
	WIFI_MODE_AP,
	WIFI_MODE_APSTA,
	WIFI_MODE_MAX,
} wl_mode_t;

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
	WIFI_AUTH_INVALID	   = 255,
	WIFI_AUTH_AUTO		   = 200,
	WIFI_AUTH_OPEN_SYSTEM  = 0,
	WIFI_AUTH_SHARED_KEY   = 1,
	WIFI_AUTH_WPA		   = 10,
	WIFI_AUTH_WPA2		   = 11,
	WIFI_AUTH_WPA_PSK	   = 2,
	WIFI_AUTH_WPA2_PSK	   = 3,
	WIFI_AUTH_WPA_WPA2_PSK = 4,
} wl_auth_mode_t;
