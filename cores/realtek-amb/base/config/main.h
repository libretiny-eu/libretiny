#pragma once

#include <autoconf.h>

void wlan_network();

// clang-format off
#define WEP40_KEY		 { 0x12, 0x34, 0x56, 0x78, 0x90 }
// clang-format on

#define CONFIG_WLAN		 1
#define SERIAL_DEBUG_RX	 1
#define STA_MODE_SSID	 "ap"
#define AP_MODE_SSID	 "wlan_ap_ssid"
#define AP_DEFAULT_CH	 6
#define WLAN0_NAME		 "wlan0"
#define WLAN1_NAME		 "wlan1"
#define WPA_PASSPHRASE	 "1234567890"
#define ATVER_1			 1
#define ATVER_2			 2
#define ATCMD_VER		 ATVER_1
#define IP_ADDR0		 192
#define IP_ADDR1		 168
#define IP_ADDR2		 1
#define IP_ADDR3		 80
#define NETMASK_ADDR0	 255
#define NETMASK_ADDR1	 255
#define NETMASK_ADDR2	 255
#define NETMASK_ADDR3	 0
#define GW_ADDR0		 192
#define GW_ADDR1		 168
#define GW_ADDR2		 1
#define GW_ADDR3		 1
#define AP_IP_ADDR0		 192
#define AP_IP_ADDR1		 168
#define AP_IP_ADDR2		 43
#define AP_IP_ADDR3		 1
#define AP_NETMASK_ADDR0 255
#define AP_NETMASK_ADDR1 255
#define AP_NETMASK_ADDR2 255
#define AP_NETMASK_ADDR3 0
#define AP_GW_ADDR0		 192
#define AP_GW_ADDR1		 168
#define AP_GW_ADDR2		 43
#define AP_GW_ADDR3		 1
