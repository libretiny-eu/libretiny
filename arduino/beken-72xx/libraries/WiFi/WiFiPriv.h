/* Copyright (c) Kuba Szczodrzyński 2022-06-26. */

#pragma once

#include "WiFi.h"

extern "C" {

#include <FreeRTOS.h>
#include <semphr.h>

#include <common.h>
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

// app/param_config.c
extern general_param_t *g_wlan_general_param;
extern ap_param_t *g_ap_param_ptr;
extern sta_param_t *g_sta_param_ptr;
extern uint8_t system_mac[6];

// WiFi.cpp
WiFiAuthMode securityTypeToAuthMode(uint8_t type);

} // extern "C"
