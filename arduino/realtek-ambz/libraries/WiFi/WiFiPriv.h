#pragma once

#ifdef __cplusplus
extern "C" {
#endif // __cplusplus

#define LWIP_NETIF_HOSTNAME 1 // this is defined in PIO builder

#include <autoconf.h>
#include <dhcp/dhcps.h>
#include <lwip/api.h>
#include <lwip/dns.h>
#include <lwip/err.h>
#include <lwip_netconf.h>
#include <wifi_conf.h>
#include <wifi_constants.h>
#include <wifi_structures.h>

extern struct netif xnetif[NET_IF_NUM];

#ifdef __cplusplus
} // extern "C"
#endif

#include <Arduino.h>

#include "WiFi.h"

extern rtw_network_info_t wifi;
extern rtw_ap_info_t ap;
extern rtw_wifi_setting_t wifi_setting;
extern unsigned char sta_password[65];
extern unsigned char ap_password[65];
extern void reset_wifi_struct(void);
extern rtw_mode_t wifi_mode;

#define NETIF_RTW_STA &xnetif[RTW_STA_INTERFACE]
#define NETIF_RTW_AP  (wifi_mode == WIFI_MODE_APSTA ? &xnetif[RTW_AP_INTERFACE] : NETIF_RTW_STA)

#define NETNAME_STA WLAN0_NAME
#define NETNAME_AP	(wifi_mode == WIFI_MODE_APSTA ? WLAN1_NAME : WLAN0_NAME)
