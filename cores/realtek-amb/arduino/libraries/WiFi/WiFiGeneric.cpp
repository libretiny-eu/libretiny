/* Copyright (c) Kuba Szczodrzyński 2022-04-25. */

#include "WiFiPrivate.h"

int32_t WiFiClass::channel() {
	int channel = 0;
	wifi_get_channel(&channel);
	return channel;
}

bool WiFiClass::modePriv(WiFiMode mode, WiFiModeAction sta, WiFiModeAction ap) {
	DIAG_PRINTF_DISABLE();
	startWifiTask();

	if (!DATA->initialized) {
		// initialize wifi first
		LT_IM(WIFI, "Initializing LwIP");
		LwIP_Init();
		DATA->initialized = true;
	}
	LT_HEAP_I();
	WiFiMode currentMode	 = getMode();
	WiFiNetworkInfo &staInfo = DATA->sta;
	WiFiNetworkInfo &apInfo	 = DATA->ap;
	bool reenableSTA		 = false;
	bool reenableAP			 = false;

	if (mode == WIFI_MODE_APSTA) {
		if (currentMode == WIFI_MODE_STA) {
			// adding AP mode doesn't seem to work fine:
			// - STA -> AP+STA
			LT_DM(WIFI, "STA was enabled: %s", staInfo.ssid);
			reenableSTA = true;
		}
		if (currentMode == WIFI_MODE_AP) {
			// restart AP mode later, as wifi has to be stopped first:
			// - AP -> AP+STA
			LT_DM(WIFI, "AP was enabled: %s", apInfo.ssid);
			reenableAP = true;
		}
	}

	if (reenableSTA || reenableAP || mode == WIFI_MODE_NULL || currentMode == WIFI_MODE_AP ||
		currentMode && mode == WIFI_MODE_AP) {
		// must stop wifi first:
		// - STA -> NULL
		// - STA -> AP
		// - STA -> AP+STA
		// - AP -> NULL
		// - AP -> STA
		// - AP -> AP+STA
		// - AP+STA -> NULL
		// - AP+STA -> AP
		LT_DM(WIFI, "Stopping WiFi to change mode");
		if (wifi_off() != RTW_SUCCESS) {
			LT_EM(WIFI, "Error while changing mode(%u)", mode);
			goto error;
		}
		rltk_wlan_deinit_fastly();
		rltk_wlan_rf_off();
		init_event_callback_list();
		vTaskDelay(20);
		currentMode = getMode();
	}

	if (currentMode == WIFI_MODE_NULL && mode != WIFI_MODE_NULL) {
		// wifi is not running, enable it the usual way:
		// - NULL -> STA
		// - NULL -> AP
		// - NULL -> AP+STA
		if (wifi_on((rtw_mode_t)mode) != RTW_SUCCESS) {
			LT_EM(WIFI, "Error while changing mode(%u)", mode);
			goto error;
		}
	} else {
		// just enable/disable wlan1:
		// - STA -> AP+STA - unused (wifi reset required)
		// - AP+STA -> STA
		wifi_mode = mode;
		/* if (ap == WLMODE_ENABLE) {
			LT_DM(WIFI, "Mode: %s ENABLE", WLAN1_NAME);
			rltk_wlan_init(WLAN1_IDX, RTW_MODE_AP);
			rltk_wlan_start(WLAN1_IDX);
			uint32_t timeout = 20;
			while (1) {
				if (rltk_wlan_running(WLAN1_IDX)) {
					wifi_set_country_code();
					break;
				}
				if (timeout == 0) {
					LT_EM(WIFI, "Error while changing mode(%u)", mode);
					goto error;
				}
				vTaskDelay(1 * configTICK_RATE_HZ);
				timeout--;
			}
			netif_set_up(WLAN1_NETIF);
			netif_set_link_up(WLAN1_NETIF);
		} */
		if (ap == WLMODE_DISABLE) {
			LT_DM(WIFI, "Mode: %s DISABLE", WLAN1_NAME);
			netif_set_link_down(WLAN1_NETIF);
			netif_set_down(WLAN1_NETIF);
#if !LT_RTL8720C
			rltk_stop_softap(WLAN1_NAME);
#else
			rltk_suspend_softap(WLAN1_NAME);
#endif
			rltk_wlan_init(WLAN1_IDX, RTW_MODE_NONE);
			wext_set_mode(WLAN1_NAME, IW_MODE_INFRA);
		}
		vTaskDelay(50);
	}

	if (mode & WIFI_MODE_AP) {
		// indicate that the interface is an AP
		// use NETNAME_AP to retrieve the actual iface name (wlan0/wlan1)
		// (this is determined by STA bit being set in wifi_mode)
		wext_set_mode(NETNAME_AP, IW_MODE_MASTER);
	}

	if (sta == WLMODE_DISABLE) {
		// mark that STA mode has been disabled manually
		free(staInfo.ssid);
		staInfo.ssid = NULL;
	}
	if (ap == WLMODE_DISABLE) {
		// mark that AP mode has been disabled manually
		free(apInfo.ssid);
		apInfo.ssid = NULL;
	}

	// force checking WiFi mode again (which will update wifi_mode)
	getMode();

	if (reenableSTA) {
		// restart STA mode from previously used config (if set)
		if (!restoreSTAConfig(staInfo))
			LT_EM(WIFI, "Couldn't restore STA mode: %s", staInfo.ssid);
	}
	if (reenableAP) {
		// restart AP mode from previously used config (if set)
		if (!restoreAPConfig(apInfo))
			LT_EM(WIFI, "Couldn't restore AP mode: %s", apInfo.ssid);
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
	DIAG_PRINTF_ENABLE();
	return true;

error:
	DIAG_PRINTF_ENABLE();
	return false;
}

WiFiMode WiFiClass::getMode() {
	if (!DATA->initialized)
		return WIFI_MODE_NULL;
	uint8_t wlan0_state = rltk_wlan_running(WLAN0_IDX);
	uint8_t wlan1_state = rltk_wlan_running(WLAN1_IDX);
	wifi_mode			= (wifi_mode_t)wlan0_state;

	LT_DM(WIFI, "WLAN: %s=%u, %s=%u", WLAN0_NAME, wlan0_state, WLAN1_NAME, wlan1_state);

	if (wlan1_state) {
		if (netif_is_up(WLAN1_NETIF))
			wifi_mode = (wifi_mode_t)(WIFI_MODE_AP | wlan0_state);
	} else {
		wifi_mode = (wifi_mode_t)(wlan0_state);
		int mode  = 0;
		// check wlan0 mode to determine if it's an AP
		if (wlan0_state)
			wext_get_mode(WLAN0_NAME, &mode);
		if (mode == IW_MODE_MASTER)
			wifi_mode = (wifi_mode_t)(wifi_mode << 1);
	}

	return wifi_mode;
}

WiFiStatus WiFiClass::status() {
	if (rltk_wlan_is_connected_to_ap() == 0) {
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
	DATA->sleep = enable;
	return true;
}

bool WiFiClass::getSleep() {
	return DATA->sleep;
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
