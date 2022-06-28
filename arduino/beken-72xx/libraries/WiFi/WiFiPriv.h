/* Copyright (c) Kuba Szczodrzyński 2022-06-26. */

#pragma once

#include <api/WiFi/WiFi.h>

extern "C" {

#include <lwip/api.h>
#include <lwip/ip_addr.h>
#include <lwip/netif.h>

#include <FreeRTOS.h>
#include <semphr.h>

#include <common.h>
#include <config.h>
#include <main_none.h>
#include <param_config.h>
#include <rw_msg_rx.h>
#include <wlan_ui_pub.h>
#include <wpa_supplicant_i.h>

extern void func_init_extended();
extern void app_pre_start();
extern void bk_wlan_ap_init(network_InitTypeDef_st *inNetworkInitPara);

// func/hostapd-2.5/wpa_supplicant/main_supplicant.c
extern struct wpa_ssid_value *wpas_connect_ssid;

// func/lwip_intf/lwip-2.0.2/port/net.c
extern struct netif *net_get_sta_handle(void);
extern struct netif *net_get_uap_handle(void);

// app/param_config.c
extern general_param_t *g_wlan_general_param;
extern ap_param_t *g_ap_param_ptr;
extern sta_param_t *g_sta_param_ptr;
extern uint8_t system_mac[6];

// WiFi.cpp
WiFiStatus eventTypeToStatus(uint8_t type);
WiFiAuthMode securityTypeToAuthMode(uint8_t type);

#define ADDR_STA_IP	  data.ipSta[0]
#define ADDR_STA_MASK data.ipSta[1]
#define ADDR_STA_GW	  data.ipSta[2]
#define ADDR_STA_DNS  data.ipSta[3]
#define ADDR_AP_IP	  data.ipAp[0]
#define ADDR_AP_MASK  data.ipAp[1]
#define ADDR_AP_GW	  data.ipAp[2]
#define ADDR_AP_DNS	  data.ipAp[3]

#define IP_STATUS	((IPStatusTypedef *)data.statusIp)
#define LINK_STATUS ((LinkStatusTypeDef *)data.statusLink)

} // extern "C"
