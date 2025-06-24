/* Copyright (c) Kuba Szczodrzyński 2022-06-26. */

#pragma once

#include <WiFi.h>
#include <sdk_private.h>

extern "C" {

#include <lwip/api.h>
#include <lwip/ip_addr.h>
#include <lwip/netif.h>
// port/net.h
#include <net.h>

#include <FreeRTOS.h>
#include <semphr.h>

#include <common.h>
#include <config.h>
#include <main_none.h>
#include <param_config.h>
#include <rw_msg_pub.h>
#include <rw_msg_rx.h>
#include <sa_ap.h>
#include <sys_ctrl_pub.h>
#include <vif_mgmt.h>
#include <wlan_ui_pub.h>
#include <wpa_supplicant_i.h>

extern void func_init_extended();
extern void app_pre_start();
extern void bk_wlan_ap_init(network_InitTypeDef_st *inNetworkInitPara);
extern int bk_wlan_power_save_set_level(BK_PS_LEVEL level);

// func/hostapd-2.5/wpa_supplicant/main_supplicant.c
extern struct wpa_ssid_value *wpas_connect_ssid;

// app/param_config.c
extern general_param_t *g_wlan_general_param;
extern ap_param_t *g_ap_param_ptr;
extern sta_param_t *g_sta_param_ptr;
extern uint8_t system_mac[6];

// WiFi.cpp
WiFiStatus eventTypeToStatus(uint8_t type);
WiFiAuthMode securityTypeToAuthMode(uint8_t type);

// WiFiEvents.cpp
extern void wifiEventSendArduino(EventId event);
extern void startWifiTask();
extern void wifiEventHandler(rw_evt_type event);

#define RW_EVT_ARDUINO (1 << 7)

#define IP_FMT "%u.%u.%u.%u"

typedef struct {
	network_InitTypeDef_st configSta;
	network_InitTypeDef_adv_st configStaAdv;
	network_InitTypeDef_ap_st configAp;
	unsigned long scannedAt;
	SemaphoreHandle_t scanSem;
	IPStatusTypedef statusIp;
	LinkStatusTypeDef statusLink;
	rw_evt_type lastStaEvent;
	rw_evt_type lastApEvent;
	bool apEnabled;
	bool sleep;
} WiFiData;

#define DATA  ((WiFiData *)data)
#define pDATA ((WiFiData *)pWiFi->data)
#define cDATA ((WiFiData *)cls->data)

#define STA_CFG		(DATA->configSta)
#define STA_ADV_CFG (DATA->configStaAdv)
#define AP_CFG		(DATA->configAp)
#define IP_STATUS	(DATA->statusIp)
#define LINK_STATUS (DATA->statusLink)

#define STA_GET_LINK_STATUS_RETURN(ret)                        \
	{                                                          \
		if (!sta_ip_is_start())                                \
			return ret;                                        \
		memset(&LINK_STATUS, 0x00, sizeof(LinkStatusTypeDef)); \
		bk_wlan_get_link_status(&LINK_STATUS);                 \
	}

#define STA_GET_IP_STATUS_RETURN(ret)                      \
	{                                                      \
		if (!sta_ip_is_start())                            \
			return ret;                                    \
		memset(&IP_STATUS, 0x00, sizeof(IPStatusTypedef)); \
		bk_wlan_get_ip_status(&IP_STATUS, BK_STATION);     \
	}

#define AP_GET_LINK_STATUS_RETURN(ret) \
	{                                  \
		if (!uap_ip_is_start())        \
			return ret;                \
	}

#define AP_GET_IP_STATUS_RETURN(ret)                       \
	{                                                      \
		if (!uap_ip_is_start())                            \
			return ret;                                    \
		memset(&IP_STATUS, 0x00, sizeof(IPStatusTypedef)); \
		bk_wlan_get_ip_status(&IP_STATUS, BK_SOFT_AP);     \
	}

} // extern "C"
