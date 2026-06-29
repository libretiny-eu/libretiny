/* Copyright (c) Etienne Le Cousin 2024-02-18. */

#include "WiFiPrivate.h"

static uint8_t psk_value[40] = {0x0};
static uint8_t mac_addr[6]	 = {0x00, 0x50, 0xC2, 0x7F, 0xBC, 0x01};

bool WiFiClass::softAP(const char *ssid, const char *passphrase, int channel, bool ssidHidden, int maxClients) {
	if (!enableAP(true))
		return false;
	if (!validate(ssid, passphrase))
		return false;

	LT_HEAP_I();

	WiFiNetworkInfo &info = DATA->ap;
	if (info.ssid != ssid)
		// free network info, if not called from restoreAPConfig()
		resetNetworkInfo(info);

	if (info.ssid != ssid)
		info.ssid = strdup(ssid);
	if (info.password != passphrase)
		info.password = strdup(passphrase);
	info.ssidHidden = ssidHidden;
	info.bssid		= softAPmacAddress(mac_addr);
	info.channel	= channel;
	info.auth		= passphrase ? LN_WIFI_AUTH_WPA2_PSK : LN_WIFI_AUTH_OPEN;

	LT_IM(WIFI, "Creating SoftAP %s", ssid);

	// clang-format off
	wifi_softap_cfg_t ap_cfg = {
		.ssid  = info.ssid,
		.pwd   = info.password,
		.bssid = info.bssid,
		.ext_cfg = {
			.channel		 = (uint8_t)info.channel,
			.authmode		 = (ln_wifi_auth_mode_t)info.auth,
			.ssid_hidden	 = info.ssidHidden,
			.beacon_interval = 100,
			.psk_value		 = NULL,
		}
	};
	// clang-format on

	if ((strlen(ap_cfg.pwd) != 0) && (ap_cfg.ext_cfg.authmode != LN_WIFI_AUTH_OPEN) &&
		(ap_cfg.ext_cfg.authmode != LN_WIFI_AUTH_WEP)) {
		memset(psk_value, 0, sizeof(psk_value));
		if (0 == ln_psk_calc(ap_cfg.ssid, ap_cfg.pwd, psk_value, sizeof(psk_value))) {
			ap_cfg.ext_cfg.psk_value = psk_value;
			hexdump(psk_value, sizeof(psk_value));
		}
	}

	// wifi start
	__wrap_ln_printf_disable();
	int ret = wifi_softap_start(&ap_cfg);
	__wrap_ln_printf_enable();
	if (ret != 0) {
		LT_EM(WIFI, "SoftAP failed; ret=%d", ret);
		return false;
	}

	return true;
}

bool WiFiClass::softAPConfig(IPAddress localIP, IPAddress gateway, IPAddress subnet) {
	if (!enableAP(true))
		return false;
	WiFiNetworkInfo &info = DATA->ap;

	// dhcp server config
	server_config_t server_config;
	server_config.server.addr	= localIP;
	server_config.port			= 67;
	server_config.lease			= 2880;
	server_config.renew			= 2880;
	server_config.ip_start.addr = IPAddress(localIP[0], localIP[1], localIP[2], 100);
	server_config.ip_end.addr	= IPAddress(localIP[0], localIP[1], localIP[2], 150);
	server_config.client_max	= 3;
	dhcpd_curr_config_set(&server_config);

	// net device config
	tcpip_ip_info_t ip_info;
	ip_info.ip.addr = info.localIP = localIP;
	ip_info.netmask.addr = info.subnet = subnet;
	ip_info.gw.addr = info.gateway = gateway;
	netdev_set_ip_info(NETIF_IDX_AP, &ip_info);
	netdev_set_active(NETIF_IDX_AP);

	return true;
}

bool WiFiClass::softAPdisconnect(bool wifiOff) {
	return enableAP(false);
}

uint8_t WiFiClass::softAPgetStationNum() {
	return 0;
}

IPAddress WiFiClass::softAPIP() {
	return netif_ip_addr4(netdev_get_netif(NETIF_IDX_AP))->addr;
}

IPAddress WiFiClass::softAPSubnetMask() {
	return netif_ip_netmask4(netdev_get_netif(NETIF_IDX_AP))->addr;
}

const char *WiFiClass::softAPgetHostname() {
	return netif_get_hostname(netdev_get_netif(NETIF_IDX_AP));
}

bool WiFiClass::softAPsetHostname(const char *hostname) {
	netif_set_hostname(netdev_get_netif(NETIF_IDX_AP), (char *)hostname);
	return true;
}

uint8_t *WiFiClass::softAPmacAddress(uint8_t *mac) {
	memcpy(mac, netdev_get_netif(NETIF_IDX_AP)->hwaddr, MAC_ADDRESS_LEN);
	return mac;
}

String WiFiClass::softAPmacAddress(void) {
	uint8_t mac[MAC_ADDRESS_LEN];
	softAPmacAddress(mac);
	return macToString(mac);
}

const String WiFiClass::softAPSSID(void) {
	WiFiNetworkInfo &info = DATA->ap;
	return (char *)info.ssid;
}
