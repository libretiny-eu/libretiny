/* Copyright (c) Kuba Szczodrzyński 2022-06-26. */

#include "WiFiPriv.h"

bool WiFiClass::modePriv(WiFiMode mode, WiFiModeAction sta, WiFiModeAction ap) {
	__wrap_bk_printf_disable();
	startWifiTask();

	if (mode && !data.statusIp) {
		data.configSta	   = zalloc(sizeof(network_InitTypeDef_st));
		data.configAp	   = zalloc(sizeof(network_InitTypeDef_ap_st));
		data.statusIp	   = malloc(sizeof(IPStatusTypedef));
		data.statusLink	   = malloc(sizeof(LinkStatusTypeDef));
		STA_CFG->dhcp_mode = DHCP_CLIENT;
	}

	if (!__bk_rf_is_init) {
		LT_D_WG("Initializing func&app");
		func_init_extended();
		app_pre_start();
		// wait for the init_thread to finish its job
		while (xTaskGetHandle("init_thread")) {
			LT_V_WG("Waiting for init_thread");
			delay(10);
		}
		LT_D_WG("Success");
		__bk_rf_is_init = true;
	}

	LT_HEAP_I();

	if (sta == WLMODE_ENABLE) {
		LT_D_WG("Enabling STA");
		bk_wlan_sta_init(NULL);
		wifiEventSendArduino(ARDUINO_EVENT_WIFI_STA_START);
	} else if (sta == WLMODE_DISABLE) {
		LT_D_WG("Disabling STA");
		bk_wlan_stop(BK_STATION);
		wifiEventSendArduino(ARDUINO_EVENT_WIFI_STA_STOP);
	}

	LT_HEAP_I();

	if (ap == WLMODE_ENABLE) {
		LT_D_WG("Enabling AP");
		bk_wlan_ap_init(NULL);
		wifiEventSendArduino(ARDUINO_EVENT_WIFI_AP_START);
	} else if (ap == WLMODE_DISABLE) {
		LT_D_WG("Disabling AP");
		bk_wlan_stop(BK_SOFT_AP);
		wifiEventSendArduino(ARDUINO_EVENT_WIFI_AP_STOP);
	}

	if (!mode) {
		free(data.configSta);
		free(data.configAp);
		free(data.statusIp);
		free(data.statusLink);
		data.configSta	= NULL;
		data.configAp	= NULL;
		data.statusIp	= NULL;
		data.statusLink = NULL;
	}

	LT_HEAP_I();

	__wrap_bk_printf_enable();
	return true;
}

WiFiMode WiFiClass::getMode() {
	if (!g_wlan_general_param)
		return WIFI_MODE_NULL;
	uint8_t role = g_wlan_general_param->role;
	// change 1->2, 2->1
	return (WiFiMode)(role + (role == 1) - (role == 2));
}

WiFiStatus WiFiClass::status() {
	rw_evt_type status = mhdr_get_station_status();
	if (status == RW_EVT_STA_CONNECTED && STA_CFG->dhcp_mode == DHCP_DISABLE)
		status = RW_EVT_STA_GOT_IP;
	return eventTypeToStatus(status);
}

IPAddress WiFiClass::hostByName(const char *hostname) {
	ip_addr_t ip;
	int ret = netconn_gethostbyname(hostname, &ip);
	if (ret == ERR_OK) {
		return ip.addr;
	}
	return IPAddress();
}
