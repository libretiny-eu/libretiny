/* Copyright (c) Kuba Szczodrzyński 2022-06-26. */

#include "WiFiPrivate.h"

bool WiFiClass::modePriv(WiFiMode mode, WiFiModeAction sta, WiFiModeAction ap) {
	__wrap_bk_printf_disable();
	startWifiTask();

	if (!__bk_rf_is_init) {
		LT_DM(WIFI, "Initializing func&app");
		func_init_extended();
		app_pre_start();
		// wait for the init_thread to finish its job
		while (xTaskGetHandle("init_thread")) {
			LT_VM(WIFI, "Waiting for init_thread");
			delay(10);
		}
		LT_DM(WIFI, "Init OK");
		__bk_rf_is_init = true;
	}

	LT_HEAP_I();

	if (mode) {
		LT_DM(WIFI, "Wakeup RF");
		uint32_t reg = 1; // this is only checked for being true-ish
		sddev_control(SCTRL_DEV_NAME, CMD_RF_HOLD_BIT_SET, &reg);
	}

	if (sta == WLMODE_ENABLE) {
		LT_DM(WIFI, "Enabling STA");
		bk_wlan_sta_init(NULL);
#if CFG_WPA_CTRL_IFACE
		wlan_sta_enable();
#endif
		wifiEventSendArduino(ARDUINO_EVENT_WIFI_STA_START);
	} else if (sta == WLMODE_DISABLE) {
		LT_DM(WIFI, "Disabling STA");
		bk_wlan_stop(BK_STATION);
		wifiEventSendArduino(ARDUINO_EVENT_WIFI_STA_STOP);
	}

	LT_HEAP_I();

	if (ap == WLMODE_ENABLE) {
		LT_DM(WIFI, "Enabling AP");
		// fake it - on BK7231, enabling the AP without starting it breaks all connection attempts
		data.apEnabled = true;
		wifiEventSendArduino(ARDUINO_EVENT_WIFI_AP_START);
	} else if (ap == WLMODE_DISABLE) {
		LT_DM(WIFI, "Disabling AP");
		bk_wlan_stop(BK_SOFT_AP);
		data.apEnabled = false;
		wifiEventSendArduino(ARDUINO_EVENT_WIFI_AP_STOP);
	}

	// force checking actual mode again
	mode = getMode();

	if (!mode)
		dataFree();

	LT_HEAP_I();

	__wrap_bk_printf_enable();
	return true;
}

WiFiMode WiFiClass::getMode() {
	uint8_t sta = !!bk_wlan_has_role(VIF_STA) * WIFI_MODE_STA;
	uint8_t ap	= data.apEnabled * WIFI_MODE_AP; // report the faked value
	return (WiFiMode)(sta | ap);
}

WiFiStatus WiFiClass::status() {
	// TODO remove the cast
	rw_evt_type status = (rw_evt_type)data.lastStaEvent;
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
