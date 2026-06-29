/* Copyright (c) Kuba Szczodrzyński 2022-04-25. */

#include "WiFiPrivate.h"

typedef struct {
	int count;
	rtw_mac_t mac_list[AP_STA_NUM];
} client_info_t;

bool WiFiClass::softAP(const char *ssid, const char *passphrase, int channel, bool ssidHidden, int maxClients) {
	if (!enableAP(true))
		return WL_CONNECT_FAILED;
	if (!validate(ssid, passphrase))
		return WL_CONNECT_FAILED;

	LT_HEAP_I();
	vTaskDelay(20);

	WiFiNetworkInfo &info = DATA->ap;
	if (info.ssid != ssid)
		// free network info, if not called from restoreAPConfig()
		resetNetworkInfo(info);

	if (info.ssid != ssid)
		info.ssid = strdup(ssid);
	info.ssidHidden = ssidHidden;
	info.channel	= channel;
	info.auth		= RTW_SECURITY_OPEN;

	if (passphrase) {
		if (info.password != passphrase)
			info.password = strdup(passphrase);
		info.auth = RTW_SECURITY_WPA2_AES_PSK;
	}

	dhcps_deinit();

	LT_IM(WIFI, "Creating SoftAP %s", ssid);

	int ret;
	if (!ssidHidden) {
		ret = wifi_start_ap(
			info.ssid,
			(rtw_security_t)info.auth,
			info.password,
			strlen(info.ssid),
			strlen(info.password),
			info.channel
		);
	} else {
		ret = wifi_start_ap_with_hidden_ssid(
			info.ssid,
			(rtw_security_t)info.auth,
			info.password,
			strlen(info.ssid),
			strlen(info.password),
			info.channel
		);
	}

	if (ret < 0) {
		LT_EM(WIFI, "SoftAP failed; ret=%d", ret);
		return false;
	}

	uint8_t timeout = 20;
	unsigned char essid[33];

	const char *ifname = NETNAME_AP;
	struct netif *ifs  = NETIF_RTW_AP;

	while (1) {
		if (wext_get_ssid(ifname, essid) > 0) {
			if (strcmp((const char *)essid, info.ssid) == 0)
				break;
		}

		if (!timeout)
			return false;

		vTaskDelay(1 * configTICK_RATE_HZ);
		timeout--;
	}

	wifi_indication(WIFI_EVENT_CONNECT, NULL, ARDUINO_EVENT_WIFI_AP_START, -2);

	dhcps_init(ifs);
	dns_server_deinit();
	return true;
}

bool WiFiClass::softAPConfig(IPAddress localIP, IPAddress gateway, IPAddress subnet) {
	if (!enableAP(true))
		return false;
	WiFiNetworkInfo &info = DATA->ap;
	struct netif *ifs	  = NETIF_RTW_AP;
	struct ip_addr ipaddr, netmask, gw;
	ipaddr.addr = info.localIP = localIP;
	netmask.addr = info.subnet = subnet;
	gw.addr = info.gateway = gateway;
	netif_set_addr(ifs, &ipaddr, &netmask, &gw);
	return true;
}

bool WiFiClass::softAPdisconnect(bool wifiOff) {
	return enableAP(false);
}

uint8_t WiFiClass::softAPgetStationNum() {
	// the struct is at wifi_conf.c:2576
	client_info_t info;
	info.count = AP_STA_NUM;
	wifi_get_associated_client_list(&info, sizeof(info));
	return info.count;
}

IPAddress WiFiClass::softAPIP() {
	return netif_ip_addr4(NETIF_RTW_AP)->addr;
}

IPAddress WiFiClass::softAPSubnetMask() {
	return netif_ip_netmask4(NETIF_RTW_AP)->addr;
}

const char *WiFiClass::softAPgetHostname() {
	return netif_get_hostname(NETIF_RTW_AP);
}

bool WiFiClass::softAPsetHostname(const char *hostname) {
	netif_set_hostname(NETIF_RTW_AP, (char *)hostname);
	return true;
}

uint8_t *WiFiClass::softAPmacAddress(uint8_t *mac) {
	memcpy(mac, NETIF_RTW_AP->hwaddr, ETH_ALEN);
	return mac;
}

String WiFiClass::softAPmacAddress(void) {
	uint8_t mac[ETH_ALEN];
	softAPmacAddress(mac);
	return macToString(mac);
}

const String WiFiClass::softAPSSID(void) {
	wext_get_ssid(NETNAME_AP, wifi_setting.ssid);
	return (char *)wifi_setting.ssid;
}
