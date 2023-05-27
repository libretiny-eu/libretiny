/* Copyright (c) Kuba Szczodrzyński 2022-04-25. */

#include "WiFiPrivate.h"

WiFiStatus
WiFiClass::begin(const char *ssid, const char *passphrase, int32_t channel, const uint8_t *bssid, bool connect) {
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
	info.auth	 = RTW_SECURITY_OPEN;

	if (passphrase) {
		if (info.password != passphrase)
			info.password = strdup(passphrase);
		info.auth = RTW_SECURITY_WPA2_AES_PSK;
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

	struct ip_addr d1, d2;
	d1.addr = info.dns1 = dns1;
	d2.addr = info.dns2 = dns2;
	if (d1.addr)
		dns_setserver(0, &d1);
	if (d2.addr)
		dns_setserver(0, &d2);

	if (!localIP[0]) {
		info.localIP = 0;
		LwIP_DHCP(0, DHCP_START);
		return true;
	}
	struct netif *ifs = NETIF_RTW_STA;
	struct ip_addr ipaddr, netmask, gw;
	ipaddr.addr = info.localIP = localIP;
	netmask.addr = info.subnet = subnet;
	gw.addr = info.gateway = gateway;
	netif_set_addr(ifs, &ipaddr, &netmask, &gw);
	LwIP_DHCP(0, DHCP_STOP);
	return true;
}

bool WiFiClass::reconnect(const uint8_t *bssid) {
	int ret;
	uint8_t dhcpRet;
	WiFiNetworkInfo &info = DATA->sta;

	LT_IM(WIFI, "Connecting to %s (bssid=%p)", info.ssid, bssid);
	DIAG_PRINTF_DISABLE();

	wext_set_ssid(WLAN0_NAME, (uint8_t *)"-", 1);

	if (!bssid) {
		ret = wifi_connect(
			info.ssid,
			(rtw_security_t)info.auth,
			info.password,
			strlen(info.ssid),
			strlen(info.password),
			-1,
			NULL
		);
	} else {
		if (info.bssid != bssid) {
			free(info.bssid);
			info.bssid = (uint8_t *)malloc(ETH_ALEN);
			memcpy(info.bssid, bssid, ETH_ALEN);
		}
		ret = wifi_connect_bssid(
			(unsigned char *)bssid,
			info.ssid,
			(rtw_security_t)info.auth,
			info.password,
			ETH_ALEN,
			strlen(info.ssid),
			strlen(info.password),
			-1,
			NULL
		);
	}

	if (ret == RTW_SUCCESS) {
		dhcpRet = LwIP_DHCP(0, DHCP_START);
		if (dhcpRet == DHCP_ADDRESS_ASSIGNED) {
			LT_HEAP_I();
			EventInfo *eventInfo				   = (EventInfo *)calloc(1, sizeof(EventInfo));
			eventInfo->got_ip.if_index			   = 0;
			eventInfo->got_ip.esp_netif			   = NULL;
			eventInfo->got_ip.ip_info.ip.addr	   = localIP();
			eventInfo->got_ip.ip_info.gw.addr	   = gatewayIP();
			eventInfo->got_ip.ip_info.netmask.addr = subnetMask();
			eventInfo->got_ip.ip_changed		   = true;
			// pass the event through the queue
			wifi_indication(WIFI_EVENT_CONNECT, (char *)eventInfo, ARDUINO_EVENT_WIFI_STA_GOT_IP, -2);
			// free memory as wifi_indication creates a copy
			free(eventInfo);
			DIAG_PRINTF_ENABLE();
			return true;
		}
		LT_EM(WIFI, "DHCP failed; dhcpRet=%d", dhcpRet);
		wifi_disconnect();
		goto error;
	}
	LT_EM(WIFI, "Connection failed; ret=%d", ret);
error:
	DIAG_PRINTF_ENABLE();
	return false;
}

bool WiFiClass::disconnect(bool wifiOff) {
	free(DATA->sta.ssid);
	DATA->sta.ssid = NULL;
	int ret		   = wifi_disconnect();
	if (wifiOff)
		enableSTA(false);
	return ret == RTW_SUCCESS;
}

bool WiFiClass::setAutoReconnect(bool autoReconnect) {
	return wifi_set_autoreconnect(autoReconnect) == RTW_SUCCESS;
}

bool WiFiClass::getAutoReconnect() {
	bool autoReconnect;
	wifi_get_autoreconnect((uint8_t *)&autoReconnect);
	return autoReconnect;
}

IPAddress WiFiClass::localIP() {
	if (!wifi_mode)
		return IPAddress();
	return netif_ip_addr4(NETIF_RTW_STA)->addr;
}

uint8_t *WiFiClass::macAddress(uint8_t *mac) {
	if ((getMode() & WIFI_MODE_STA) == 0) {
		lt_get_device_mac(mac);
		return mac;
	}
	memcpy(mac, NETIF_RTW_STA.hwaddr, ETH_ALEN);
	return mac;
}

IPAddress WiFiClass::subnetMask() {
	return netif_ip_netmask4(NETIF_RTW_STA)->addr;
}

IPAddress WiFiClass::gatewayIP() {
	return netif_ip_gw4(NETIF_RTW_STA)->addr;
}

IPAddress WiFiClass::dnsIP(uint8_t dns_no) {
	return dns_getserver(0)->addr;
}

IPAddress WiFiClass::broadcastIP() {
	return LwIP_GetBC(NETIF_RTW_STA);
}

const char *WiFiClass::getHostname() {
	return netif_get_hostname(NETIF_RTW_STA);
}

bool WiFiClass::setHostname(const char *hostname) {
	netif_set_hostname(NETIF_RTW_STA, (char *)hostname);
	return true;
}

bool WiFiClass::setMacAddress(const uint8_t *mac) {
	return wifi_set_mac_address((char *)mac) == RTW_SUCCESS;
}

const String WiFiClass::SSID() {
	if (!isConnected())
		return "";
	wifi_get_setting(NETNAME_STA, &wifi_setting);
	return (char *)wifi_setting.ssid;
}

const String WiFiClass::psk() {
	if (!isConnected() || !DATA->sta.password)
		return "";
	return DATA->sta.password;
}

uint8_t *WiFiClass::BSSID() {
	WiFiNetworkInfo &info = DATA->sta;
	if (!info.bssid)
		info.bssid = (uint8_t *)malloc(ETH_ALEN);
	wext_get_bssid(NETNAME_STA, info.bssid);
	return info.bssid;
}

int8_t WiFiClass::RSSI() {
	int rssi = 0;
	wifi_get_rssi(&rssi);
	return rssi;
}

WiFiAuthMode WiFiClass::getEncryption() {
	wifi_get_setting(NETNAME_STA, &wifi_setting);
	return securityTypeToAuthMode(wifi_setting.security_type);
}
