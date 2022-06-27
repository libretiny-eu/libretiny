/* Copyright (c) Kuba Szczodrzyński 2022-06-26. */

#include "WiFiPriv.h"

bool WiFiClass::modePriv(WiFiMode mode, WiFiModeAction sta, WiFiModeAction ap) {
	__wrap_bk_printf_disable();

	if (!__bk_rf_is_init) {
		LT_D_WG("Initializing func&app");
		func_init_extended();
		app_pre_start();
		__bk_rf_is_init = true;
	}

	LT_HEAP_I();

	if (sta == WLMODE_ENABLE) {
		LT_D_WG("Enabling STA");
		bk_wlan_sta_init(NULL);
	} else if (sta == WLMODE_DISABLE) {
		LT_D_WG("Disabling STA");
		bk_wlan_stop(BK_STATION);
	}

	LT_HEAP_I();

	if (ap == WLMODE_ENABLE) {
		LT_D_WG("Enabling AP");
		bk_wlan_ap_init(NULL);
	} else if (ap == WLMODE_DISABLE) {
		LT_D_WG("Disabling AP");
		bk_wlan_stop(BK_SOFT_AP);
	}

	LT_HEAP_I();

	__wrap_bk_printf_enable();
}

WiFiMode WiFiClass::getMode() {
	if (!g_wlan_general_param)
		return WIFI_MODE_NULL;
	uint8_t role = g_wlan_general_param->role;
	// change 1->2, 2->1
	return (WiFiMode)(role + (role == 1) - (role == 2));
}

WiFiStatus WiFiClass::status() {
	// wpa_suppliant_ctrl_get_wpas()->disconnected;
	if (wpas_connect_ssid && wpas_connect_ssid->ssid_len) {
		return WL_CONNECTED;
	} else {
		return WL_DISCONNECTED;
	}
}
