/* Copyright (c) Kuba Szczodrzyński 2022-04-25. */

#include "WiFiPrivate.h"

WiFiStatus
WiFiClass::begin(const char *ssid, const char *passphrase, int32_t channel, const uint8_t *bssid, bool connect) {
	if (!enableSTA(true))
		return WL_CONNECT_FAILED;
	if (!validate(ssid, passphrase))
		return WL_CONNECT_FAILED;

	LT_HEAP_I();

	memset(wifi.bssid.octet, 0, ETH_ALEN);
	strcpy((char *)wifi.ssid.val, ssid);
	wifi.ssid.len = strlen(ssid);

	wifi.security_type = RTW_SECURITY_OPEN;
	wifi.password	   = NULL;
	wifi.password_len  = 0;
	wifi.key_id		   = 0;

	if (passphrase) {
		strcpy((char *)sta_password, passphrase);
		wifi.security_type = RTW_SECURITY_WPA2_AES_PSK;
		wifi.password	   = sta_password;
		wifi.password_len  = strlen(passphrase);
	}

	if (reconnect(bssid))
		return WL_CONNECTED;
	else
		return WL_CONNECT_FAILED;
}

bool WiFiClass::config(IPAddress localIP, IPAddress gateway, IPAddress subnet, IPAddress dns1, IPAddress dns2) {
	if (!enableSTA(true))
		return false;

	struct ip_addr d1, d2;
	d1.addr = dns1;
	d2.addr = dns2;
	if (dns1[0])
		dns_setserver(0, &d1);
	if (dns2[0])
		dns_setserver(0, &d2);

	if (!localIP[0]) {
		LwIP_DHCP(0, DHCP_START);
		return true;
	}
	struct netif *ifs = NETIF_RTW_STA;
	struct ip_addr ipaddr, netmask, gw;
	ipaddr.addr	 = localIP;
	netmask.addr = subnet;
	gw.addr		 = gateway;
	netif_set_addr(ifs, &ipaddr, &netmask, &gw);
	LwIP_DHCP(0, DHCP_STOP);
	return true;
}

bool WiFiClass::reconnect(const uint8_t *bssid) {
	int ret;
	uint8_t dhcpRet;

	LT_IM(WIFI, "Connecting to %s", wifi.ssid.val);
	__wrap_rtl_printf_disable();
	__wrap_DiagPrintf_disable();

	if (!bssid) {
		ret = wifi_connect(
			(char *)wifi.ssid.val,
			wifi.security_type,
			(char *)wifi.password,
			wifi.ssid.len,
			wifi.password_len,
			wifi.key_id,
			NULL
		);
	} else {
		ret = wifi_connect_bssid(
			(unsigned char *)bssid,
			(char *)wifi.ssid.val,
			wifi.security_type,
			(char *)wifi.password,
			ETH_ALEN,
			wifi.ssid.len,
			wifi.password_len,
			wifi.key_id,
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
			__wrap_rtl_printf_enable();
			__wrap_DiagPrintf_enable();
			return true;
		}
		LT_EM(WIFI, "DHCP failed; dhcpRet=%d", dhcpRet);
		wifi_disconnect();
		goto error;
	}
	LT_EM(WIFI, "Connection failed; ret=%d", ret);
error:
	__wrap_rtl_printf_enable();
	__wrap_DiagPrintf_enable();
	return false;
}

bool WiFiClass::disconnect(bool wifiOff) {
	int ret = wifi_disconnect();
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
	return LwIP_GetIP(NETIF_RTW_STA);
}

uint8_t *WiFiClass::macAddress(uint8_t *mac) {
	if (getMode() == WIFI_MODE_NULL) {
		uint8_t *efuse = (uint8_t *)malloc(512);
		EFUSE_LogicalMap_Read(efuse);
		memcpy(mac, efuse + 0x11A, ETH_ALEN);
		free(efuse);
		return mac;
	}
	memcpy(mac, LwIP_GetMAC(NETIF_RTW_STA), ETH_ALEN);
	return mac;
}

IPAddress WiFiClass::subnetMask() {
	return LwIP_GetMASK(NETIF_RTW_STA);
}

IPAddress WiFiClass::gatewayIP() {
	return LwIP_GetGW(NETIF_RTW_STA);
}

IPAddress WiFiClass::dnsIP(uint8_t dns_no) {
	struct ip_addr dns;
	LwIP_GetDNS(&dns);
	return dns.addr;
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
	if (!isConnected() || !wifi.password)
		return "";
	return (char *)wifi.password;
}

uint8_t *WiFiClass::BSSID() {
	wext_get_bssid(NETNAME_STA, wifi.bssid.octet);
	return wifi.bssid.octet;
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
