/*
 * SPDX-FileCopyrightText: 2015-2021 Espressif Systems (Shanghai) CO LTD
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once

#include "WiFiType.h"

/** Argument structure for WIFI_EVENT_SCAN_DONE event */
typedef struct {
	uint32_t status; /**< status of scanning APs: 0 - success, 1 - failure */
	uint8_t number;	 /**< number of scan results */
	uint8_t scan_id; /**< scan sequence number, used for block scan */
} wifi_event_sta_scan_done_t;

/** Argument structure for WIFI_EVENT_STA_CONNECTED event */
typedef struct {
	uint8_t ssid[32];		   /**< SSID of connected AP */
	uint8_t ssid_len;		   /**< SSID length of connected AP */
	uint8_t bssid[6];		   /**< BSSID of connected AP*/
	uint8_t channel;		   /**< channel of connected AP*/
	wifi_auth_mode_t authmode; /**< authentication mode used by AP*/
} wifi_event_sta_connected_t;

/** Argument structure for WIFI_EVENT_STA_DISCONNECTED event */
typedef struct {
	uint8_t ssid[32]; /**< SSID of disconnected AP */
	uint8_t ssid_len; /**< SSID length of disconnected AP */
	uint8_t bssid[6]; /**< BSSID of disconnected AP */
	uint8_t reason;	  /**< reason of disconnection */
} wifi_event_sta_disconnected_t;

/** Argument structure for WIFI_EVENT_STA_AUTHMODE_CHANGE event */
typedef struct {
	wifi_auth_mode_t old_mode; /**< the old auth mode of AP */
	wifi_auth_mode_t new_mode; /**< the new auth mode of AP */
} wifi_event_sta_authmode_change_t;

/** Argument structure for WIFI_EVENT_STA_WPS_ER_PIN event */
typedef struct {
	uint8_t pin_code[8]; /**< PIN code of station in enrollee mode */
} wifi_event_sta_wps_er_pin_t;

/** Argument structure for WIFI_EVENT_STA_WPS_ER_FAILED event */
typedef enum {
	WPS_FAIL_REASON_NORMAL = 0, /**< ESP32 WPS normal fail reason */
	WPS_FAIL_REASON_RECV_M2D,	/**< ESP32 WPS receive M2D frame */
	WPS_FAIL_REASON_MAX
} wifi_event_sta_wps_fail_reason_t;

#define MAX_SSID_LEN	   32
#define MAX_PASSPHRASE_LEN 64
#define MAX_WPS_AP_CRED	   3

/** Argument structure for WIFI_EVENT_STA_WPS_ER_SUCCESS event */
typedef struct {
	uint8_t ap_cred_cnt; /**< Number of AP credentials received */

	struct {
		uint8_t ssid[MAX_SSID_LEN];				/**< SSID of AP */
		uint8_t passphrase[MAX_PASSPHRASE_LEN]; /**< Passphrase for the AP */
	} ap_cred[MAX_WPS_AP_CRED];					/**< All AP credentials received from WPS handshake */
} wifi_event_sta_wps_er_success_t;

/** Argument structure for WIFI_EVENT_AP_STACONNECTED event */
typedef struct {
	uint8_t mac[6];		/**< MAC address of the station connected to ESP32 soft-AP */
	uint8_t aid;		/**< the aid that ESP32 soft-AP gives to the station connected to  */
	bool is_mesh_child; /**< flag to identify mesh child */
} wifi_event_ap_staconnected_t;

/** Argument structure for WIFI_EVENT_AP_STADISCONNECTED event */
typedef struct {
	uint8_t mac[6];		/**< MAC address of the station disconnects to ESP32 soft-AP */
	uint8_t aid;		/**< the aid that ESP32 soft-AP gave to the station disconnects to  */
	bool is_mesh_child; /**< flag to identify mesh child */
} wifi_event_ap_stadisconnected_t;

/** Argument structure for WIFI_EVENT_AP_PROBEREQRECVED event */
typedef struct {
	int rssi;		/**< Received probe request signal strength */
	uint8_t mac[6]; /**< MAC address of the station which send probe request */
} wifi_event_ap_probe_req_rx_t;

/**
 * @brief FTM operation status types
 *
 */
typedef enum {
	FTM_STATUS_SUCCESS = 0,	  /**< FTM exchange is successful */
	FTM_STATUS_UNSUPPORTED,	  /**< Peer does not support FTM */
	FTM_STATUS_CONF_REJECTED, /**< Peer rejected FTM configuration in FTM Request */
	FTM_STATUS_NO_RESPONSE,	  /**< Peer did not respond to FTM Requests */
	FTM_STATUS_FAIL,		  /**< Unknown error during FTM exchange */
} wifi_ftm_status_t;

/** Argument structure for */
typedef struct {
	uint8_t dlog_token; /**< Dialog Token of the FTM frame */
	int8_t rssi;		/**< RSSI of the FTM frame received */
	uint32_t rtt;		/**< Round Trip Time in pSec with a peer */
	uint64_t t1;		/**< Time of departure of FTM frame from FTM Responder in pSec */
	uint64_t t2;		/**< Time of arrival of FTM frame at FTM Initiator in pSec */
	uint64_t t3;		/**< Time of departure of ACK from FTM Initiator in pSec */
	uint64_t t4;		/**< Time of arrival of ACK at FTM Responder in pSec */
} wifi_ftm_report_entry_t;

/** Argument structure for WIFI_EVENT_FTM_REPORT event */
typedef struct {
	uint8_t peer_mac[6];	  /**< MAC address of the FTM Peer */
	wifi_ftm_status_t status; /**< Status of the FTM operation */
	uint32_t rtt_raw;		  /**< Raw average Round-Trip-Time with peer in Nano-Seconds */
	uint32_t rtt_est;		  /**< Estimated Round-Trip-Time with peer in Nano-Seconds */
	uint32_t dist_est;		  /**< Estimated one-way distance in Centi-Meters */
	wifi_ftm_report_entry_t
		*ftm_report_data;			/**< Pointer to FTM Report with multiple entries, should be freed after use */
	uint8_t ftm_report_num_entries; /**< Number of entries in the FTM Report data */
} wifi_event_ftm_report_t;

#define WIFI_STATIS_BUFFER (1 << 0)
#define WIFI_STATIS_RXTX   (1 << 1)
#define WIFI_STATIS_HW	   (1 << 2)
#define WIFI_STATIS_DIAG   (1 << 3)
#define WIFI_STATIS_PS	   (1 << 4)
#define WIFI_STATIS_ALL	   (-1)

/** Argument structure for WIFI_EVENT_ACTION_TX_STATUS event */
typedef struct {
	int ifx;		  /**< WiFi interface to send request to */
	uint32_t context; /**< Context to identify the request */
	uint8_t da[6];	  /**< Destination MAC address */
	uint8_t status;	  /**< Status of the operation */
} wifi_event_action_tx_status_t;

/** Argument structure for WIFI_EVENT_ROC_DONE event */
typedef struct {
	uint32_t context; /**< Context to identify the request */
} wifi_event_roc_done_t;

/** Event structure for IP_EVENT_STA_GOT_IP, IP_EVENT_ETH_GOT_IP events  */
typedef struct {
	esp_ip4_addr_t ip;		/**< Interface IPV4 address */
	esp_ip4_addr_t netmask; /**< Interface IPV4 netmask */
	esp_ip4_addr_t gw;		/**< Interface IPV4 gateway address */
} esp_netif_ip_info_t;

/** @brief IPV6 IP address information
 */
typedef struct {
	esp_ip6_addr_t ip; /**< Interface IPV6 address */
} esp_netif_ip6_info_t;

typedef struct {
	int if_index;				 /*!< Interface index for which the event is received (left for legacy compilation) */
	void *esp_netif;			 /*!< Pointer to corresponding esp-netif object */
	esp_netif_ip_info_t ip_info; /*!< IP address, netmask, gatway IP address */
	bool ip_changed;			 /*!< Whether the assigned IP has changed or not */
} ip_event_got_ip_t;

/** Event structure for IP_EVENT_GOT_IP6 event */
typedef struct {
	int if_index;				   /*!< Interface index for which the event is received (left for legacy compilation) */
	void *esp_netif;			   /*!< Pointer to corresponding esp-netif object */
	esp_netif_ip6_info_t ip6_info; /*!< IPv6 address of the interface */
	int ip_index;				   /*!< IPv6 address index */
} ip_event_got_ip6_t;

/** Event structure for IP_EVENT_AP_STAIPASSIGNED event */
typedef struct {
	esp_ip4_addr_t ip; /*!< IP address which was assigned to the station */
} ip_event_ap_staipassigned_t;
