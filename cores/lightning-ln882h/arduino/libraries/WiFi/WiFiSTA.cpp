/* Copyright (c) Etienne Le Cousin 2024-03-10. */

#include "WiFiPrivate.h"

WiFiStatus WiFiClass::begin(
	const char *ssid,
	const char *passphrase,
	int32_t channel,
	const uint8_t *bssid,
	bool connect
) {
	if (!enableSTA(true))
		return WL_CONNECT_FAILED;
	if (!validate(ssid, passphrase))
		return WL_CONNECT_FAILED;

	LT_HEAP_I();

	WiFiNetworkInfo &info = DATA->sta;
	if (info.ssid != ssid)
		// free network info, if not called from restoreSTAConfig()
		resetNetworkInfo(info);

	if (info.ssid != ssid)
		info.ssid = strdup(ssid);
	info.channel = channel;
	info.auth	 = LN_WIFI_AUTH_OPEN;

	if (passphrase) {
		if (info.password != passphrase)
			info.password = strdup(passphrase);
		info.auth = LN_WIFI_AUTH_WPA_WPA2_PSK;
	}

	if (reconnect(bssid))
		return WL_CONNECTED;
	else
		return WL_CONNECT_FAILED;
}

bool WiFiClass::config(IPAddress localIP, IPAddress gateway, IPAddress subnet, IPAddress dns1, IPAddress dns2) {
	if (!enableSTA(true))
		return false;
	WiFiNetworkInfo &info = DATA->sta;
	struct netif *ifs	  = netdev_get_netif(NETIF_IDX_STA);

	ip4_addr_t d1, d2;
	d1.addr = info.dns1 = dns1;
	d2.addr = info.dns2 = dns2;
	if (d1.addr)
		dns_setserver(0, &d1);
	if (d2.addr)
		dns_setserver(0, &d2);

	if (!localIP[0]) {
		info.localIP = 0;
		netifapi_dhcp_start(ifs);
		return true;
	}
	ip4_addr_t ipaddr, netmask, gw;
	ipaddr.addr = info.localIP = localIP;
	netmask.addr = info.subnet = subnet;
	gw.addr = info.gateway = gateway;
	netif_set_addr(ifs, &ipaddr, &netmask, &gw);
	netifapi_dhcp_release_and_stop(ifs);

	return true;
}

bool WiFiClass::reconnect(const uint8_t *bssid) {
	WiFiNetworkInfo &info = DATA->sta;
	LT_IM(WIFI, "Connecting to %s (bssid=%p)", info.ssid, bssid);

	if (bssid != info.bssid) {
		if (!info.bssid)
			info.bssid = (uint8_t *)malloc(BSSID_LEN);
		memcpy(info.bssid, bssid, BSSID_LEN);
	}

	uint8_t psk_value[40]	   = {0x0};
	wifi_sta_connect_t connect = {
		.ssid	   = info.ssid,
		.pwd	   = info.password,
		.bssid	   = NULL, // info.bssid,
		.psk_value = NULL,
	};
	wifi_scan_cfg_t scan_cfg = {
		.channel   = (uint8_t)info.channel,
		.scan_type = WIFI_SCAN_TYPE_ACTIVE,
		.scan_time = 5,
	};

	if (0 == ln_psk_calc(connect.ssid, connect.pwd, psk_value, sizeof(psk_value)))
		connect.psk_value = psk_value;

	LT_DM(WIFI, "Starting WiFi...");

	__wrap_ln_printf_disable();
	int ret = wifi_sta_connect(&connect, &scan_cfg);
	// int ret = wifi_sta_connect_v2(&connect, &scan_cfg, 10);
	__wrap_ln_printf_enable();

	LT_DM(WIFI, "Start OK (%d)", ret);

	return true;
}

bool WiFiClass::disconnect(bool wifiOff) {
	free(DATA->sta.ssid);
	DATA->sta.ssid = NULL;
	int ret		   = wifi_sta_disconnect();
	if (wifiOff)
		enableSTA(false);
	return ret == 0;
}

bool WiFiClass::setAutoReconnect(bool autoReconnect) {
	return false;
}

bool WiFiClass::getAutoReconnect() {
	return false;
}

IPAddress WiFiClass::localIP() {
	tcpip_ip_info_t ip_info;
	netdev_get_ip_info(NETIF_IDX_STA, &ip_info);
	return IPAddress(ip_info.ip.addr);
}

IPAddress WiFiClass::subnetMask() {
	tcpip_ip_info_t ip_info;
	netdev_get_ip_info(NETIF_IDX_STA, &ip_info);
	return IPAddress(ip_info.netmask.addr);
}

IPAddress WiFiClass::gatewayIP() {
	tcpip_ip_info_t ip_info;
	netdev_get_ip_info(NETIF_IDX_STA, &ip_info);
	return IPAddress(ip_info.gw.addr);
}

IPAddress WiFiClass::dnsIP(uint8_t dns_no) {
	return dns_getserver(dns_no)->addr;
}

IPAddress WiFiClass::broadcastIP() {
	return calculateBroadcast(localIP(), subnetMask());
}

const char *WiFiClass::getHostname() {
	struct netif *ifs = netdev_get_netif(NETIF_IDX_STA);
	return netif_get_hostname(ifs);
}

bool WiFiClass::setHostname(const char *hostname) {
	struct netif *ifs = netdev_get_netif(NETIF_IDX_STA);
	netif_set_hostname(ifs, (char *)hostname);
	return true;
}

uint8_t *WiFiClass::macAddress(uint8_t *mac) {
	if (SYSPARAM_ERR_NONE == sysparam_sta_mac_get(mac))
		return mac;
	else
		LT_WM(WIFI, "sysparam sta mac get failed!");

	lt_get_device_mac(mac);
	return mac;
}

bool WiFiClass::setMacAddress(const uint8_t *mac) {
	sysparam_sta_mac_update(mac);
	return netdev_set_mac_addr(NETIF_IDX_STA, (uint8_t *)mac) == 0;
}

const String WiFiClass::SSID() {
	const char *ssid	 = "";
	const uint8_t *bssid = NULL;
	wifi_get_sta_conn_info(&ssid, &bssid);
	return ssid;
}

const String WiFiClass::psk() {
	if (!isConnected() || !DATA->sta.password)
		return "";
	return DATA->sta.password;
}

uint8_t *WiFiClass::BSSID() {
	const char *ssid	 = NULL;
	const uint8_t *bssid = NULL;
	wifi_get_sta_conn_info(&ssid, &bssid);
	return (uint8_t *)bssid;
}

int32_t WiFiClass::channel() {
	uint8_t channel = 0;
	wifi_get_channel(&channel);
	return (int32_t)channel;
}

int8_t WiFiClass::RSSI() {
	int8_t rssi = 0;
	wifi_sta_get_rssi(&rssi);
	return rssi;
}

WiFiAuthMode WiFiClass::getEncryption() {
	ln_wifi_auth_mode_t mode;
	wifi_sta_get_connected_ap_security(&mode);
	return securityTypeToAuthMode(mode);
}
