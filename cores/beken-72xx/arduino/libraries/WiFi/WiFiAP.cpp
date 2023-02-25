/* Copyright (c) Kuba Szczodrzyński 2022-07-01. */

#include "WiFiPriv.h"

bool WiFiClass::softAP(const char *ssid, const char *passphrase, int channel, bool ssidHidden, int maxClients) {
	if (!enableAP(true))
		return WL_CONNECT_FAILED;
	if (!validate(ssid, passphrase))
		return WL_CONNECT_FAILED;

	LT_HEAP_I();

	// Beken SDK bug: bk_wlan_ap_init_adv() doesn't null-terminate the passphrase
	memset(g_ap_param_ptr->key, '\0', 65);

	strcpy(AP_CFG->wifi_ssid, ssid);
	if (passphrase) {
		strcpy(AP_CFG->wifi_key, passphrase);
		AP_CFG->security = BK_SECURITY_TYPE_WPA2_MIXED;
	} else {
		AP_CFG->wifi_key[0] = '\0';
		AP_CFG->security	= BK_SECURITY_TYPE_NONE;
	}

	AP_CFG->channel				= channel;
	AP_CFG->ssid_hidden			= ssidHidden;
	AP_CFG->max_con				= maxClients;
	AP_CFG->dhcp_mode			= DHCP_SERVER;
	AP_CFG->wifi_retry_interval = 100;

	LT_IM(WIFI, "Creating SoftAP %s", ssid);

	if (!AP_CFG->local_ip_addr[0]) {
		LT_DM(WIFI, "Setting default IP config");
		softAPConfig((uint32_t)0, (uint32_t)0, (uint32_t)0);
	}

	LT_DM(WIFI, "Static IP: %s / %s / %s", AP_CFG->local_ip_addr, AP_CFG->net_mask, AP_CFG->gateway_ip_addr);

	__wrap_bk_printf_disable();
	OSStatus ret = bk_wlan_start_ap_adv(AP_CFG);
	__wrap_bk_printf_enable();

	if (ret != 0) {
		LT_EM(WIFI, "SoftAP failed; ret=%d", ret);
		return false;
	}
	LT_DM(WIFI, "AP start OK");
	return true;
}

bool WiFiClass::softAPConfig(IPAddress localIP, IPAddress gateway, IPAddress subnet) {
	dataInitialize();
	if (!localIP) {
		localIP = gateway = IPAddress(192, 168, 43, 1);
		subnet			  = IPAddress(255, 255, 255, 0);
	}
	sprintf(AP_CFG->local_ip_addr, IP_FMT, localIP[0], localIP[1], localIP[2], localIP[3]);
	sprintf(AP_CFG->net_mask, IP_FMT, subnet[0], subnet[1], subnet[2], subnet[3]);
	sprintf(AP_CFG->gateway_ip_addr, IP_FMT, gateway[0], gateway[1], gateway[2], gateway[3]);
	// from wlan_ui.c:1370
	if (uap_ip_is_start()) {
		uap_ip_down();
		ip_address_set(
			BK_STATION,
			AP_CFG->dhcp_mode,
			AP_CFG->local_ip_addr,
			AP_CFG->net_mask,
			AP_CFG->gateway_ip_addr,
			AP_CFG->dns_server_ip_addr
		);
		uap_ip_start();
	}
	return true;
}

bool WiFiClass::softAPdisconnect(bool wifiOff) {
	if (!(getMode() & WIFI_MODE_AP))
		// do not call SDK methods before even initializing WiFi first
		return true;
	return bk_wlan_stop(BK_SOFT_AP) == kNoErr;
}

uint8_t WiFiClass::softAPgetStationNum() {
	return 0;
}

IPAddress WiFiClass::softAPIP() {
	AP_GET_IP_STATUS_RETURN((uint32_t)0);
	IPAddress ip;
	ip.fromString(IP_STATUS->ip);
	return ip;
}

IPAddress WiFiClass::softAPSubnetMask() {
	AP_GET_IP_STATUS_RETURN((uint32_t)0);
	IPAddress ip;
	ip.fromString(IP_STATUS->mask);
	return ip;
}

const char *WiFiClass::softAPgetHostname() {
	struct netif *ifs = (struct netif *)net_get_uap_handle();
	return netif_get_hostname(ifs);
}

bool WiFiClass::softAPsetHostname(const char *hostname) {
	struct netif *ifs = (struct netif *)net_get_uap_handle();
	netif_set_hostname(ifs, (char *)hostname);
	return true;
}

uint8_t *WiFiClass::softAPmacAddress(uint8_t *mac) {
	setMacAddress(mac);
	return mac;
}

String WiFiClass::softAPmacAddress(void) {
	uint8_t mac[ETH_ALEN];
	wifi_get_mac_address((char *)mac, CONFIG_ROLE_AP);
	return macToString(mac);
}

const String WiFiClass::softAPSSID(void) {
	AP_GET_LINK_STATUS_RETURN("");
	return AP_CFG->wifi_ssid;
}
