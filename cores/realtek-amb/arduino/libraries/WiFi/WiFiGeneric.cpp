/* Copyright (c) Kuba Szczodrzyński 2022-04-25. */

#include "WiFiPriv.h"

int32_t WiFiClass::channel() {
	int channel = 0;
	wifi_get_channel(&channel);
	return channel;
}

bool WiFiClass::modePriv(WiFiMode mode, WiFiModeAction sta, WiFiModeAction ap) {
	__wrap_rtl_printf_disable();
	__wrap_DiagPrintf_disable();
	startWifiTask();

	if (!data.initialized) {
		// initialize wifi first
		LT_IM(WIFI, "Initializing LwIP");
		LwIP_Init();
		reset_wifi_struct();
		data.initialized = true;
	}
	LT_HEAP_I();
	if (getMode()) {
		// stop wifi to change mode
		LT_DM(WIFI, "Stopping WiFi to change mode");
		if (wifi_off() != RTW_SUCCESS)
			goto error;
		vTaskDelay(20);
		if (mode == WIFI_MODE_NULL)
			goto error;
	}

	if (wifi_on((rtw_mode_t)mode) != RTW_SUCCESS) {
		LT_EM(WIFI, "Error while changing mode(%u)", mode);
		goto error;
	}

	// send STA start/stop events and AP stop event (start is handled in softAP())
	if (sta == WLMODE_ENABLE) {
		wifi_indication(WIFI_EVENT_CONNECT, NULL, ARDUINO_EVENT_WIFI_STA_START, -2);
	} else if (sta == WLMODE_DISABLE) {
		wifi_indication(WIFI_EVENT_CONNECT, NULL, ARDUINO_EVENT_WIFI_STA_STOP, -2);
	}
	if (ap == WLMODE_DISABLE) {
		wifi_indication(WIFI_EVENT_CONNECT, NULL, ARDUINO_EVENT_WIFI_AP_STOP, -2);
	}

	LT_HEAP_I();
	__wrap_rtl_printf_enable();
	__wrap_DiagPrintf_enable();
	return true;

error:
	__wrap_rtl_printf_enable();
	__wrap_DiagPrintf_enable();
	return false;
}

WiFiMode WiFiClass::getMode() {
	if (!data.initialized)
		return WIFI_MODE_NULL;
	return (WiFiMode)wifi_mode;
}

WiFiStatus WiFiClass::status() {
	if (wifi_is_connected_to_ap() == 0) {
		return WL_CONNECTED;
	} else {
		return WL_DISCONNECTED;
	}
}

bool WiFiClass::setSleep(bool enable) {
	LT_DM(WIFI, "WiFi sleep mode %u", enable);
	if (enable) {
		if (wifi_enable_powersave() != RTW_SUCCESS)
			return false;
	} else {
		if (wifi_disable_powersave() != RTW_SUCCESS)
			return false;
	}
	data.sleep = enable;
	return true;
}

bool WiFiClass::getSleep() {
	return data.sleep;
}

bool WiFiClass::setTxPower(int power) {
	return false; // wifi_set_txpower(power) == RTW_SUCCESS;
}

int WiFiClass::getTxPower() {
	return 0;
	int power = 0;
	wifi_get_txpower(&power);
	return power;
}

IPAddress WiFiClass::hostByName(const char *hostname) {
	ip_addr_t ip;
	int ret = netconn_gethostbyname(hostname, &ip);
	if (ret == ERR_OK) {
		return ip.addr;
	}
	return IPAddress();
}
