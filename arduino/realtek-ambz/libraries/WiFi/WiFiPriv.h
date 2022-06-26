/* Copyright (c) Kuba Szczodrzyński 2022-04-25. */

#pragma once

#include "WiFi.h"

extern "C" {

// these are defined in PIO builder (for IDE to understand)
#define LWIP_TIMEVAL_PRIVATE 0
#define LWIP_NETIF_HOSTNAME	 1
#define LWIP_SO_RCVBUF		 1

#define MAX_SOCK_NUM				4
#define WIFI_CLIENT_CONNECT_TIMEOUT 3000
#define WIFI_CLIENT_READ_TIMEOUT	3000
#define WIFI_CLIENT_WRITE_RETRY		10
#define WIFI_CLIENT_SELECT_TIMEOUT	1000
#define WIFI_CLIENT_FLUSH_BUF_SIZE	1024

// disable #defines removing lwip_ prefix
#undef LWIP_COMPAT_SOCKETS
#define LWIP_COMPAT_SOCKETS 0
#include <lwip/sockets.h>

#include <FreeRTOS.h>
#include <autoconf.h>
#include <dhcp/dhcps.h>
#include <lwip/api.h>
#include <lwip/dns.h>
#include <lwip/err.h>
#include <lwip_netconf.h>
#include <semphr.h>
#include <sys/time.h>
#include <wifi_conf.h>
#include <wifi_constants.h>
#include <wifi_structures.h>

extern struct netif xnetif[NET_IF_NUM];

} // extern "C"

// WiFi.cpp
extern rtw_network_info_t wifi;
extern rtw_ap_info_t ap;
extern rtw_wifi_setting_t wifi_setting;
extern unsigned char sta_password[65];
extern unsigned char ap_password[65];
extern void reset_wifi_struct(void);
extern rtw_mode_t wifi_mode;
extern WiFiAuthMode securityTypeToAuthMode(uint8_t type);
// WiFiEvents.cpp
extern void startWifiTask();
extern void handleRtwEvent(uint16_t event, char *data, int len, int flags);

#define NETIF_RTW_STA &xnetif[RTW_STA_INTERFACE]
#define NETIF_RTW_AP  (wifi_mode == WIFI_MODE_APSTA ? &xnetif[RTW_AP_INTERFACE] : NETIF_RTW_STA)

#define NETNAME_STA WLAN0_NAME
#define NETNAME_AP	(wifi_mode == WIFI_MODE_APSTA ? WLAN1_NAME : WLAN0_NAME)
