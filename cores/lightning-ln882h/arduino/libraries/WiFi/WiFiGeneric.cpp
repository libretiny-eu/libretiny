/* Copyright (c) Etienne Le Cousin 2024-03-10. */

#include "WiFiPrivate.h"

static uint8_t psk_value[40] = {0x0};

bool WiFiClass::modePriv(WiFiMode mode, WiFiModeAction sta, WiFiModeAction ap) {
	__wrap_ln_printf_disable();

	if (!DATA->initialized) {
		// rf preprocess,img cal
		wifi_rf_calibration();
		// Init wifi stack.
		wifi_init();
		// Init lwip stack.
		LT_IM(WIFI, "Initializing LwIP");
		lwip_tcpip_init();
		// Init wifi manager
		wifi_manager_init();
		// Register event handlers
		registerWifiHandlers();
		DATA->mode		  = WIFI_MODE_NULL;
		DATA->initialized = true;
	}

	LT_HEAP_I();
	WiFiMode currentMode	 = DATA->mode;
	WiFiNetworkInfo &staInfo = DATA->sta;
	WiFiNetworkInfo &apInfo	 = DATA->ap;

	if (mode == WIFI_MODE_APSTA) {
		LT_EM(WIFI, "AP+STA mode not supported!");
		goto error;
	}

	if (sta == WLMODE_ENABLE) {
		LT_DM(WIFI, "Enabling STA");

		// 1. sta mac get
		uint8_t mac_addr[6];
		setMacAddress(macAddress(mac_addr));

		// 2. net device(lwip)
		netdev_set_active(NETIF_IDX_STA);

		// 3. wifi start
		wifi_sta_start(mac_addr, WIFI_NO_POWERSAVE);

		wifiEventSendArduino(ARDUINO_EVENT_WIFI_STA_START);
	} else if (sta == WLMODE_DISABLE) {
		LT_DM(WIFI, "Disabling STA");
		wifi_sta_disconnect();
		netdev_set_state(NETIF_IDX_STA, NETDEV_DOWN);
		wifiEventSendArduino(ARDUINO_EVENT_WIFI_STA_STOP);
	}

	LT_HEAP_I();

	if (ap == WLMODE_ENABLE) {
		LT_DM(WIFI, "Enabling AP");

		// 1. ap mac get
		uint8_t mac_addr[6];
		sysparam_softap_mac_get(mac_addr);
		netdev_set_mac_addr(NETIF_IDX_AP, mac_addr);

		// 2. net device(lwip)
		netdev_set_active(NETIF_IDX_AP);

		wifiEventSendArduino(ARDUINO_EVENT_WIFI_AP_START);
	} else if (ap == WLMODE_DISABLE) {
		LT_DM(WIFI, "Disabling AP");
		wifi_softap_deauth_all();
		netdev_set_state(NETIF_IDX_AP, NETDEV_DOWN);

		wifiEventSendArduino(ARDUINO_EVENT_WIFI_AP_STOP);
	}

	DATA->mode = mode;

	LT_HEAP_I();

	__wrap_ln_printf_enable();
	return true;

error:
	__wrap_ln_printf_enable();
	return false;
}

WiFiMode WiFiClass::getMode() {
	if (!DATA->initialized)
		return WIFI_MODE_NULL;

	// return fake value because the chip doesn't report the mode before it is started
	return DATA->mode;

	/*switch (wifi_current_mode_get()) {
		case LN_WIFI_MODE_STATION:
			return WIFI_MODE_STA;
		case LN_WIFI_MODE_AP:
			return WIFI_MODE_AP;
		case LN_WIFI_MODE_AP_STATION:
			return WIFI_MODE_APSTA;
	}
	return WIFI_MODE_NULL;*/
}

WiFiStatus WiFiClass::status() {
	wifi_sta_status_t status = WIFI_STA_STATUS_STARTUP;
	wifi_get_sta_status(&status);
	if (status == WIFI_STA_STATUS_CONNECTED) {
		return WL_CONNECTED;
	} else {
		return WL_DISCONNECTED;
	}
}

bool WiFiClass::setSleep(bool enable) {
	LT_DM(WIFI, "WiFi sleep mode %u", enable);

	if (enable) {
		if (wifi_sta_set_powersave(WIFI_MAX_POWERSAVE))
			return false;
	} else {
		if (wifi_sta_set_powersave(WIFI_NO_POWERSAVE))
			return false;
	}
	DATA->sleep = enable;
	return true;
}

bool WiFiClass::getSleep() {
	return DATA->sleep;
}

IPAddress WiFiClass::hostByName(const char *hostname) {
	ip_addr_t ip;
	int ret = netconn_gethostbyname(hostname, &ip);
	if (ret == ERR_OK) {
		return ip.addr;
	}
	return IPAddress();
}
