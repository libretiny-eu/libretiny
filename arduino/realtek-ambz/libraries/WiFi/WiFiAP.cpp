#include "WiFi.h"
#include "WiFiPriv.h"

bool WiFiClass::softAP(const char *ssid, const char *passphrase, int channel, bool ssidHidden, int maxClients) {
	if (!enableAP(true))
		return false;

	if (!ssid || *ssid == 0x00 || strlen(ssid) > 32)
		return false;

	if (passphrase && strlen(passphrase) < 8)
		return false;

	strcpy((char *)ap.ssid.val, ssid);
	ap.ssid.len = strlen(ssid);
	ap.channel	= channel;

	ap.security_type = RTW_SECURITY_OPEN;
	ap.password		 = NULL;
	ap.password_len	 = 0;

	if (passphrase) {
		strcpy((char *)ap_password, passphrase);
		wifi.security_type = RTW_SECURITY_WPA2_AES_PSK;
		wifi.password	   = ap_password;
		wifi.password_len  = strlen(passphrase);
	}

	dhcps_deinit();

	int ret;
	if (!ssidHidden) {
		ret = wifi_start_ap(
			(char *)ap.ssid.val,
			ap.security_type,
			(char *)ap.password,
			ap.ssid.len,
			ap.password_len,
			ap.channel
		);
	} else {
		ret = wifi_start_ap_with_hidden_ssid(
			(char *)ap.ssid.val,
			ap.security_type,
			(char *)ap.password,
			ap.ssid.len,
			ap.password_len,
			ap.channel
		);
	}

	if (ret < 0)
		return false;

	uint8_t timeout = 20;
	unsigned char essid[33];

	const char *ifname = NETNAME_AP;
	struct netif *ifs  = NETIF_RTW_AP;

	while (1) {
		if (wext_get_ssid(ifname, essid) > 0) {
			if (strcmp((const char *)essid, (const char *)ap.ssid.val) == 0)
				break;
		}

		if (!timeout)
			return false;

		vTaskDelay(1 * configTICK_RATE_HZ);
		timeout--;
	}

	dhcps_init(ifs);
	return true;
}

bool WiFiClass::softAPConfig(IPAddress localIP, IPAddress gateway, IPAddress subnet) {
	if (!enableAP(true))
		return false;
	struct netif *ifs = NETIF_RTW_AP;
	struct ip_addr ipaddr, netmask, gw;
	ipaddr.addr	 = localIP;
	netmask.addr = subnet;
	gw.addr		 = gateway;
	netif_set_addr(ifs, &ipaddr, &netmask, &gw);
	return true;
}

bool WiFiClass::softAPdisconnect(bool wifiOff) {
	// TODO implement wifi_restart_ap
	if (wifiOff)
		return enableAP(false);
	return true;
}

uint8_t WiFiClass::softAPgetStationNum() {
	// TODO
	return 0;
}

IPAddress WiFiClass::softAPIP() {
	return LwIP_GetIP(NETIF_RTW_AP);
}

IPAddress WiFiClass::softAPBroadcastIP() {
	return calculateBroadcast(softAPIP(), LwIP_GetMASK(NETIF_RTW_AP));
}

IPAddress WiFiClass::softAPNetworkID() {
	return calculateNetworkID(softAPIP(), LwIP_GetMASK(NETIF_RTW_AP));
}

uint8_t WiFiClass::softAPSubnetCIDR() {
	return calculateSubnetCIDR(LwIP_GetMASK(NETIF_RTW_AP));
}

bool WiFiClass::softAPenableIpV6() {
	return false;
}

IPv6Address WiFiClass::softAPIPv6() {
	return IPv6Address();
}

const char *WiFiClass::softAPgetHostname() {
	return netif_get_hostname(NETIF_RTW_AP);
}

bool WiFiClass::softAPsetHostname(const char *hostname) {
	netif_set_hostname(NETIF_RTW_AP, (char *)hostname);
	return true;
}

uint8_t *WiFiClass::softAPmacAddress(uint8_t *mac) {
	uint8_t *macLocal = LwIP_GetMAC(NETIF_RTW_AP);
	memcpy(mac, macLocal, ETH_ALEN);
	free(macLocal);
	return mac;
}

String WiFiClass::softAPmacAddress(void) {
	uint8_t mac[ETH_ALEN];
	macAddress(mac);
	return macToString(mac);
}

const String WiFiClass::softAPSSID(void) {
	wifi_get_setting(NETNAME_AP, &wifi_setting);
	return (char *)wifi_setting.ssid;
}
