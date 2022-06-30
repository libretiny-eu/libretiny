/* Copyright (c) Kuba Szczodrzyński 2022-06-27. */

#include "WiFiPriv.h"

WiFiStatus
WiFiClass::begin(const char *ssid, const char *passphrase, int32_t channel, const uint8_t *bssid, bool connect) {
	enableSTA(true);

	if (!ssid || *ssid == 0x00 || strlen(ssid) > 32) {
		LT_W("SSID not specified or too long");
		return WL_CONNECT_FAILED;
	}

	if (passphrase && strlen(passphrase) > 64) {
		LT_W("Passphrase too long");
		return WL_CONNECT_FAILED;
	}

	// store the network data for later
	strcpy(data.ssid, ssid);
	if (passphrase)
		strcpy(data.pass, passphrase);
	else
		data.pass[0] = 0;

	if (reconnect(bssid))
		return WL_CONNECTED;

	return WL_CONNECT_FAILED;
}

bool WiFiClass::config(IPAddress localIP, IPAddress gateway, IPAddress subnet, IPAddress dns1, IPAddress dns2) {
	ADDR_STA_IP	  = (uint32_t)localIP;
	ADDR_STA_GW	  = (uint32_t)gateway;
	ADDR_STA_MASK = (uint32_t)subnet;
	ADDR_STA_DNS  = (uint32_t)dns1;

	if (status() == WL_CONNECTED) {
		IPStatusTypedef config;
		config.dhcp = !localIP[0];
		if (localIP) {
			sprintf(config.ip, "%u.%u.%u.%u", ADDR_STA_IP);
			sprintf(config.mask, "%u.%u.%u.%u", ADDR_STA_MASK);
			sprintf(config.gate, "%u.%u.%u.%u", ADDR_STA_GW);
		}
		if (dns1)
			sprintf(config.dns, "%u.%u.%u.%u", ADDR_STA_DNS);
		bk_wlan_set_ip_status(&config, BK_STATION);
	}
	return true;
}

bool WiFiClass::reconnect(const uint8_t *bssid) {
	if (!bssid && !data.ssid[0]) {
		LT_E("(B)SSID not specified");
		goto error;
	}

	if (bssid) {
		LT_D_WG("Connecting to " MACSTR, MAC2STR(bssid));
	} else {
		LT_D_WG("Connecting to %s", data.ssid);
	}

	network_InitTypeDef_st config;
	memset(&config, 0, sizeof(network_InitTypeDef_st));
	// network_InitTypeDef_adv_st config;
	// memset(&config, 0, sizeof(network_InitTypeDef_adv_st));

	config.wifi_mode		   = BK_STATION;
	config.wifi_retry_interval = 100;
	// config.ap_info.security	   = BK_SECURITY_TYPE_WPA2_MIXED;
	// config.ap_info.channel	   = 6;
	strcpy(config.wifi_ssid, data.ssid);
	// strcpy(config.ap_info.ssid, data.ssid);
	strcpy(config.wifi_key, data.pass);
	// strcpy(config.key, data.pass);
	// config.key_len = strlen(data.pass);
	if (bssid)
		memcpy(config.wifi_bssid, bssid, 6);
	// memcpy(config.ap_info.bssid, bssid, 6);

	if (ADDR_STA_IP && ADDR_STA_MASK && ADDR_STA_GW) {
		config.dhcp_mode = DHCP_DISABLE;
		sprintf(config.local_ip_addr, "%u.%u.%u.%u", ADDR_STA_IP);
		sprintf(config.net_mask, "%u.%u.%u.%u", ADDR_STA_MASK);
		sprintf(config.gateway_ip_addr, "%u.%u.%u.%u", ADDR_STA_GW);
		LT_D_WG("Static IP: %s / %s / %s", config.local_ip_addr, config.net_mask, config.gateway_ip_addr);
	} else {
		config.dhcp_mode = DHCP_CLIENT;
		LT_D_WG("Using DHCP");
	}

	if (ADDR_STA_DNS) {
		sprintf(config.dns_server_ip_addr, "%u.%u.%u.%u", ADDR_STA_DNS);
		LT_D_WG("Static DNS: %s", config.dns_server_ip_addr);
	}

	if (!data.scannedAt || millis() - data.scannedAt > 10000) {
		LT_D_WG("Scan needed");
		// apparently a scan must be performed first,
		// else it hangs at "[sa_sta]MM_START_REQ"
		scanNetworks(false);
	}

	LT_D_WG("Starting WiFi...");
	__wrap_bk_printf_disable();
	bk_wlan_start_sta(&config);
	__wrap_bk_printf_enable();
	LT_D_WG("bk_wlan_start() OK");
	return true;

error:
	return false;
}

bool WiFiClass::disconnect(bool wifiOff) {
	bk_wlan_connection_loss();
	return true;
}

bool WiFiClass::setAutoReconnect(bool autoReconnect) {
	return false;
}

bool WiFiClass::getAutoReconnect() {
	return false;
}

IPAddress WiFiClass::localIP() {
	IPStatusTypedef config;
	bk_wlan_get_ip_status(&config, BK_STATION);
	IPAddress ip;
	ip.fromString(config.ip);
	return ip;
}

IPAddress WiFiClass::subnetMask() {
	IPStatusTypedef config;
	bk_wlan_get_ip_status(&config, BK_STATION);
	IPAddress ip;
	ip.fromString(config.mask);
	return ip;
}

IPAddress WiFiClass::gatewayIP() {
	IPStatusTypedef config;
	bk_wlan_get_ip_status(&config, BK_STATION);
	IPAddress ip;
	ip.fromString(config.gate);
	return ip;
}

IPAddress WiFiClass::dnsIP(uint8_t dns_no) {
	IPStatusTypedef config;
	bk_wlan_get_ip_status(&config, BK_STATION);
	IPAddress ip;
	ip.fromString(config.dns);
	return ip;
}

IPAddress WiFiClass::broadcastIP() {
	return calculateBroadcast(localIP(), subnetMask());
}

const char *WiFiClass::getHostname() {
	struct netif *ifs = net_get_sta_handle();
	return netif_get_hostname(ifs);
}

bool WiFiClass::setHostname(const char *hostname) {
	struct netif *ifs = net_get_sta_handle();
	netif_set_hostname(ifs, (char *)hostname);
	return true;
}

uint8_t *WiFiClass::macAddress(uint8_t *mac) {
	wifi_get_mac_address((char *)mac, CONFIG_ROLE_STA);
	return mac;
}

bool WiFiClass::setMacAddress(const uint8_t *mac) {
	wifi_set_mac_address((char *)mac);
	return true;
}

const String WiFiClass::SSID() {
	bk_wlan_get_link_status(LINK_STATUS);
	return (char *)LINK_STATUS->ssid;
}

const String WiFiClass::psk() {
	if (!isConnected())
		return "";
	struct wpa_supplicant *wpas = wpa_suppliant_ctrl_get_wpas();
	if (!wpas || !wpas->conf || !wpas->conf->ssid)
		return "";
	return (char *)wpas->conf->ssid->passphrase;
}

uint8_t *WiFiClass::BSSID() {
	bk_wlan_get_link_status(LINK_STATUS);
	return LINK_STATUS->bssid;
}

int32_t WiFiClass::channel() {
	bk_wlan_get_link_status(LINK_STATUS);
	return LINK_STATUS->channel;
}

int8_t WiFiClass::RSSI() {
	bk_wlan_get_link_status(LINK_STATUS);
	return LINK_STATUS->wifi_strength;
}

WiFiAuthMode WiFiClass::getEncryption() {
	bk_wlan_get_link_status(LINK_STATUS);
	return securityTypeToAuthMode(LINK_STATUS->security);
}
