/* Copyright (c) Kuba Szczodrzyński 2022-04-25. */

#pragma once

#include <WiFi.h>
#include <sdk_private.h>

extern "C" {

// copy defines from PIO builder (for IDE to understand)
#define LWIP_TIMEVAL_PRIVATE 0
#define LWIP_NETIF_HOSTNAME	 1
#define LWIP_SO_RCVBUF		 1

#include <FreeRTOS.h>
#include <autoconf.h>
#include <dhcp/dhcps.h>
#include <lwip/api.h>
#include <lwip/dns.h>
#include <lwip/err.h>
#include <lwip_netconf.h>
#include <semphr.h>
#include <wifi_conf.h>
#include <wifi_constants.h>
#include <wifi_structures.h>

extern struct netif xnetif[NET_IF_NUM];

// dhcps.c
extern void dns_server_init(struct netif *pnetif);
extern void dns_server_deinit(void);

// wifi_util.c
extern void rltk_stop_softap(const char *ifname);
extern void rltk_suspend_softap(const char *ifname);
extern void rltk_suspend_softap_beacon(const char *ifname);

} // extern "C"

// WiFi.cpp
extern rtw_wifi_setting_t wifi_setting;
extern wifi_mode_t wifi_mode;
extern WiFiAuthMode securityTypeToAuthMode(uint8_t type);
// WiFiEvents.cpp
extern void startWifiTask();
extern void handleRtwEvent(uint16_t event, char *data, int len, int flags);

#define WLAN0_NETIF &xnetif[RTW_STA_INTERFACE]
#define WLAN1_NETIF &xnetif[RTW_AP_INTERFACE]

#define NETIF_RTW_STA WLAN0_NETIF
#define NETIF_RTW_AP  (wifi_mode & WIFI_MODE_STA ? WLAN1_NETIF : WLAN0_NETIF)

#define NETNAME_STA WLAN0_NAME
#define NETNAME_AP	(wifi_mode & WIFI_MODE_STA ? WLAN1_NAME : WLAN0_NAME)

typedef struct {
	bool initialized;
	bool sleep;
	SemaphoreHandle_t scanSem;
	WiFiNetworkInfo sta;
	WiFiNetworkInfo ap;
} WiFiData;

#define DATA  ((WiFiData *)data)
#define pDATA ((WiFiData *)pWiFi->data)
#define cDATA ((WiFiData *)cls->data)
