/* Copyright (c) Kuba Szczodrzyński 2022-06-27. */

#include "WiFiPrivate.h"

WiFiStatus
WiFiClass::begin(const char *ssid, const char *passphrase, int32_t channel, const uint8_t *bssid, bool connect) {
	if (!enableSTA(true))
		return WL_CONNECT_FAILED;
	if (!validate(ssid, passphrase))
		return WL_CONNECT_FAILED;

	LT_HEAP_I();

	disconnect(false);

	strcpy(STA_CFG.wifi_ssid, ssid);
	if (passphrase) {
		strcpy(STA_CFG.wifi_key, passphrase);
	} else {
		STA_CFG.wifi_bssid[0] = '\0';
	}

	if (reconnect(bssid))
		return WL_CONNECTED;

	return WL_CONNECT_FAILED;
}

bool WiFiClass::config(IPAddress localIP, IPAddress gateway, IPAddress subnet, IPAddress dns1, IPAddress dns2) {
	STA_CFG.dhcp_mode = localIP ? DHCP_DISABLE : DHCP_CLIENT;
	if (localIP) {
		sprintf(STA_CFG.local_ip_addr, IP_FMT, localIP[0], localIP[1], localIP[2], localIP[3]);
		sprintf(STA_CFG.net_mask, IP_FMT, subnet[0], subnet[1], subnet[2], subnet[3]);
		sprintf(STA_CFG.gateway_ip_addr, IP_FMT, gateway[0], gateway[1], gateway[2], gateway[3]);
		if (dns1) {
			sprintf(STA_CFG.dns_server_ip_addr, IP_FMT, dns1[0], dns1[1], dns1[2], dns1[3]);
		} else {
			STA_CFG.dns_server_ip_addr[0] = '\0';
		}
	} else {
		STA_CFG.local_ip_addr[0]	  = '\0';
		STA_CFG.net_mask[0]			  = '\0';
		STA_CFG.gateway_ip_addr[0]	  = '\0';
		STA_CFG.dns_server_ip_addr[0] = '\0';
	}

	// from wlan_ui.c:1370
	if (sta_ip_is_start()) {
		sta_ip_down();
		ip_address_set(
			BK_STATION,
			STA_CFG.dhcp_mode,
			STA_CFG.local_ip_addr,
			STA_CFG.net_mask,
			STA_CFG.gateway_ip_addr,
			STA_CFG.dns_server_ip_addr
		);
		sta_ip_start();
	}
	return true;
}

bool WiFiClass::reconnect(const uint8_t *bssid) {
	if (!bssid && !STA_CFG.wifi_ssid[0]) {
		LT_EM(WIFI, "(B)SSID not specified");
		goto error;
	}

	if (bssid) {
		LT_IM(WIFI, "Connecting to " MACSTR, MAC2STR(bssid));
	} else {
		LT_IM(WIFI, "Connecting to %s", STA_CFG.wifi_ssid);
	}

	LT_DM(WIFI, "Data = %p", DATA->configSta);

	STA_CFG.wifi_mode			= BK_STATION;
	STA_CFG.wifi_retry_interval = 100;
	if (bssid)
		memcpy(STA_CFG.wifi_bssid, bssid, 6);
	else
		memset(STA_CFG.wifi_bssid, 0x00, 6);

	if (STA_CFG.dhcp_mode == DHCP_DISABLE) {
		LT_DM(WIFI, "Static IP: %s / %s / %s", STA_CFG.local_ip_addr, STA_CFG.net_mask, STA_CFG.gateway_ip_addr);
		LT_DM(WIFI, "Static DNS: %s", STA_CFG.dns_server_ip_addr);
	} else {
		LT_DM(WIFI, "Using DHCP");
	}

	LT_DM(WIFI, "Starting WiFi...");

	__wrap_bk_printf_disable();
	bk_wlan_start_sta(&STA_CFG);
	__wrap_bk_printf_enable();

	LT_DM(WIFI, "Start OK");
	return true;

error:
	return false;
}

bool WiFiClass::disconnect(bool wifiOff) {
#if LT_DEBUG_WIFI
	memset(&LINK_STATUS, 0x00, sizeof(LinkStatusTypeDef));
	bk_wlan_get_link_status(&LINK_STATUS);
	LT_DM(WIFI, "Disconnecting from %s (wifiOff=%d)", LINK_STATUS.ssid, wifiOff);
#endif
	bk_wlan_connection_loss();
	if (wifiOff)
		enableSTA(false);
	return true;
}

bool WiFiClass::setAutoReconnect(bool autoReconnect) {
	return false;
}

bool WiFiClass::getAutoReconnect() {
	return false;
}

IPAddress WiFiClass::localIP() {
	STA_GET_IP_STATUS_RETURN((uint32_t)0);
	IPAddress ip;
	ip.fromString(IP_STATUS.ip);
	return ip;
}

IPAddress WiFiClass::subnetMask() {
	STA_GET_IP_STATUS_RETURN((uint32_t)0);
	IPAddress ip;
	ip.fromString(IP_STATUS.mask);
	return ip;
}

IPAddress WiFiClass::gatewayIP() {
	STA_GET_IP_STATUS_RETURN((uint32_t)0);
	IPAddress ip;
	ip.fromString(IP_STATUS.gate);
	return ip;
}

IPAddress WiFiClass::dnsIP(uint8_t dns_no) {
	STA_GET_IP_STATUS_RETURN((uint32_t)0);
	IPAddress ip;
	ip.fromString(IP_STATUS.dns);
	return ip;
}

IPAddress WiFiClass::broadcastIP() {
	return calculateBroadcast(localIP(), subnetMask());
}

const char *WiFiClass::getHostname() {
	struct netif *ifs = (struct netif *)net_get_sta_handle();
	return netif_get_hostname(ifs);
}

bool WiFiClass::setHostname(const char *hostname) {
	struct netif *ifs = (struct netif *)net_get_sta_handle();
	netif_set_hostname(ifs, (char *)hostname);
	return true;
}

uint8_t *WiFiClass::macAddress(uint8_t *mac) {
	wifi_get_mac_address((char *)mac, CONFIG_ROLE_STA);
	return mac;
}

bool WiFiClass::setMacAddress(const uint8_t *mac) {
	if (mac[0] & 0x01) {
		LT_EM(WIFI, "Invalid MAC address");
		return false;
	}
	// ensure "mac_inited" is true
	wifi_get_mac_address((char *)system_mac, BK_STATION);
	// store the MAC globally
	memcpy(system_mac, mac, 6);
	WiFiMode previousMode = getMode();
	if (previousMode) {
		mode(WIFI_MODE_NULL);
		mode(previousMode);
	}
	return true;
}

const String WiFiClass::SSID() {
	STA_GET_LINK_STATUS_RETURN("");
	return (char *)LINK_STATUS.ssid;
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
	STA_GET_LINK_STATUS_RETURN(NULL);
	return LINK_STATUS.bssid;
}

int32_t WiFiClass::channel() {
	STA_GET_LINK_STATUS_RETURN(0);
	return LINK_STATUS.channel;
}

int8_t WiFiClass::RSSI() {
	STA_GET_LINK_STATUS_RETURN(0);
	return LINK_STATUS.wifi_strength;
}

WiFiAuthMode WiFiClass::getEncryption() {
	STA_GET_LINK_STATUS_RETURN(WIFI_AUTH_INVALID);
	return securityTypeToAuthMode(LINK_STATUS.security);
}
