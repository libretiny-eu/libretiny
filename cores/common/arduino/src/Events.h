/*
 ESP8266WiFiGeneric.h - esp8266 Wifi support.
 Based on WiFi.h from Ardiono WiFi shield library.
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
#include <WiFiEvents.h>
#include <functional>

typedef enum {
	ARDUINO_EVENT_WIFI_READY = 0,			/**< ESP32 WiFi ready */
	ARDUINO_EVENT_WIFI_SCAN_DONE,			/**< ESP32 finish scanning AP */
	ARDUINO_EVENT_WIFI_STA_START,			/**< ESP32 station start */
	ARDUINO_EVENT_WIFI_STA_STOP,			/**< ESP32 station stop */
	ARDUINO_EVENT_WIFI_STA_CONNECTED,		/**< ESP32 station connected to AP */
	ARDUINO_EVENT_WIFI_STA_DISCONNECTED,	/**< ESP32 station disconnected from AP */
	ARDUINO_EVENT_WIFI_STA_AUTHMODE_CHANGE, /**< the auth mode of AP connected by ESP32 station changed */
	ARDUINO_EVENT_WIFI_STA_GOT_IP,
	ARDUINO_EVENT_WIFI_STA_GOT_IP6,
	ARDUINO_EVENT_WIFI_STA_LOST_IP,
	ARDUINO_EVENT_WIFI_AP_START,		   /**< ESP32 soft-AP start */
	ARDUINO_EVENT_WIFI_AP_STOP,			   /**< ESP32 soft-AP stop */
	ARDUINO_EVENT_WIFI_AP_STACONNECTED,	   /**< a station connected to ESP32 soft-AP */
	ARDUINO_EVENT_WIFI_AP_STADISCONNECTED, /**< a station disconnected from ESP32 soft-AP */
	ARDUINO_EVENT_WIFI_AP_STAIPASSIGNED,
	ARDUINO_EVENT_WIFI_AP_PROBEREQRECVED, /**< Receive probe request packet in soft-AP interface */
	ARDUINO_EVENT_WIFI_AP_GOT_IP6,
	ARDUINO_EVENT_WIFI_FTM_REPORT, /**< Receive report of FTM procedure */
	ARDUINO_EVENT_ETH_START,
	ARDUINO_EVENT_ETH_STOP,
	ARDUINO_EVENT_ETH_CONNECTED,
	ARDUINO_EVENT_ETH_DISCONNECTED,
	ARDUINO_EVENT_ETH_GOT_IP,
	ARDUINO_EVENT_ETH_GOT_IP6,
	ARDUINO_EVENT_WPS_ER_SUCCESS,	  /**< ESP32 station wps succeeds in enrollee mode */
	ARDUINO_EVENT_WPS_ER_FAILED,	  /**< ESP32 station wps fails in enrollee mode */
	ARDUINO_EVENT_WPS_ER_TIMEOUT,	  /**< ESP32 station wps timeout in enrollee mode */
	ARDUINO_EVENT_WPS_ER_PIN,		  /**< ESP32 station wps pin code in enrollee mode */
	ARDUINO_EVENT_WPS_ER_PBC_OVERLAP, /**< ESP32 station wps overlap in enrollee mode */
	ARDUINO_EVENT_SC_SCAN_DONE,
	ARDUINO_EVENT_SC_FOUND_CHANNEL,
	ARDUINO_EVENT_SC_GOT_SSID_PSWD,
	ARDUINO_EVENT_SC_SEND_ACK_DONE,
	ARDUINO_EVENT_PROV_INIT,
	ARDUINO_EVENT_PROV_DEINIT,
	ARDUINO_EVENT_PROV_START,
	ARDUINO_EVENT_PROV_END,
	ARDUINO_EVENT_PROV_CRED_RECV,
	ARDUINO_EVENT_PROV_CRED_FAIL,
	ARDUINO_EVENT_PROV_CRED_SUCCESS,
	ARDUINO_EVENT_MAX
} arduino_event_id_t;

typedef union {
	wifi_event_sta_scan_done_t wifi_scan_done;
	wifi_event_sta_authmode_change_t wifi_sta_authmode_change;
	wifi_event_sta_connected_t wifi_sta_connected;
	wifi_event_sta_disconnected_t wifi_sta_disconnected;
	wifi_event_sta_wps_er_pin_t wps_er_pin;
	wifi_event_sta_wps_fail_reason_t wps_fail_reason;
	wifi_event_ap_probe_req_rx_t wifi_ap_probereqrecved;
	wifi_event_ap_staconnected_t wifi_ap_staconnected;
	wifi_event_ap_stadisconnected_t wifi_ap_stadisconnected;
	wifi_event_ftm_report_t wifi_ftm_report;
	ip_event_ap_staipassigned_t wifi_ap_staipassigned;
	ip_event_got_ip_t got_ip;
	ip_event_got_ip6_t got_ip6;
	// smartconfig_event_got_ssid_pswd_t sc_got_ssid_pswd;
	// esp_eth_handle_t eth_connected;
	// wifi_sta_config_t prov_cred_recv;
	// wifi_prov_sta_fail_reason_t prov_fail_reason;
} arduino_event_info_t;

typedef struct {
	arduino_event_id_t event_id;
	arduino_event_info_t event_info;
} arduino_event_t;

#define EventId		arduino_event_id_t
#define EventId_t	arduino_event_id_t
#define EventInfo	arduino_event_info_t
#define EventInfo_t arduino_event_info_t
#define Event_t		arduino_event_t

typedef void (*EventCb)(EventId event);
typedef std::function<void(EventId event, EventInfo info)> EventFuncCb;
typedef void (*EventSysCb)(Event_t *event);

typedef struct EventHandler_s {
	static uint16_t lastId;
	uint16_t id;
	EventCb cb;
	EventFuncCb fcb;
	EventSysCb scb;
	EventId eventId;

	EventHandler_s() : id(lastId++), cb(NULL), fcb(NULL), scb(NULL) {}
} EventHandler;
