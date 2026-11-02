/* Copyright (c) Kuba Szczodrzyński 2022-04-25. */

#include "WiFiMacText.h"
#include "WiFiPrivate.h"
#include <libretiny.h>

// Station MAC override handed to the driver by setMacAddress().
// The driver keeps a pointer to the text and re-parses it on every radio
// start, so these must stay valid for the whole program lifetime.
static uint8_t stagedMac[ETH_ALEN];
static char stagedMacText[WIFI_MAC_TEXT_SIZE];
static bool stagedMacUsed	 = false;
static bool vendorMacChecked = false;

// Hand the address to the driver and remember it for macAddress().
static void stageMac(const uint8_t *mac) {
	memcpy(stagedMac, mac, ETH_ALEN);
	wifiMacToText(stagedMac, stagedMacText);
	rtw_initmac	  = stagedMacText;
	stagedMacUsed = true;
}

void stageVendorMac() {
#if LT_RTL8720C
	if (vendorMacChecked)
		return;
	vendorMacChecked = true;
	if (stagedMacUsed || rtw_initmac != nullptr)
		return; // an explicit override (setMacAddress() or the application) wins

	uint8_t chipMac[ETH_ALEN];
	lt_get_device_mac(chipMac);
	if (wifiMacIsAssignable(chipMac))
		return; // the chip has its own address

	uint8_t flashMac[ETH_ALEN];
	if (!lt_get_flash_mac(flashMac))
		return;
	stageMac(flashMac);
	LT_IM(WIFI, "No MAC in eFuse, using the one from flash: %s", stagedMacText);
#endif
}

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

// Enable or disable partial scan for a specific channel
static void set_pscan_channel(uint8_t channel, bool enable) {
	uint8_t channel_list[1] = {channel};
	uint8_t pscan_config[1] = {enable ? (PSCAN_ENABLE | PSCAN_FAST_SURVEY) : 0};
	wifi_set_pscan_chan(channel_list, pscan_config, 1);
}

bool WiFiClass::reconnect(const uint8_t *bssid) {
	int ret;
	uint8_t dhcpRet;
	WiFiNetworkInfo &info = DATA->sta;
	EventInfo *eventInfo  = nullptr;

	LT_IM(WIFI, "Connecting to %s (bssid=%p)", info.ssid, bssid);
	DIAG_PRINTF_DISABLE();

	wext_set_ssid(WLAN0_NAME, (uint8_t *)"-", 1);

	// Feed watchdog before potentially long-blocking connect
	lt_wdt_feed();

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
		// If we have both BSSID and channel, use partial scan for faster connection
		if (info.channel > 0) {
			set_pscan_channel(info.channel, true);
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
		// Reset partial scan config
		if (info.channel > 0) {
			set_pscan_channel(info.channel, false);
		}
	}

	// Feed watchdog after connect, before DHCP
	lt_wdt_feed();

	if (ret != RTW_SUCCESS) {
		LT_EM(WIFI, "Connection failed; ret=%d", ret);
		goto error;
	}

	// Run DHCP only if not using static IP configuration
	if (!DATA->sta.localIP) {
		dhcpRet = LwIP_DHCP(0, DHCP_START);
		// Feed watchdog after DHCP
		lt_wdt_feed();
		if (dhcpRet != DHCP_ADDRESS_ASSIGNED) {
			LT_EM(WIFI, "DHCP failed; dhcpRet=%d", dhcpRet);
			wifi_disconnect();
			goto error;
		}
	}

	LT_HEAP_I();
	eventInfo							   = (EventInfo *)calloc(1, sizeof(EventInfo));
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
		// the radio is off: report what the station will use once it starts
		stageVendorMac();
		if (stagedMacUsed)
			memcpy(mac, stagedMac, ETH_ALEN);
		else
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
	netif_set_hostname(NETIF_RTW_STA, (char *)storeHostname(staHostname, hostname));
	return true;
}

/**
 * Override the station MAC address for the next start of the radio.
 *
 * This deliberately does NOT use wifi_set_mac_address(). That SDK call is
 * implemented as "write_mac" -> rtw_efuse_map_write(), which programs the
 * one-time-programmable eFuse: it cannot be undone, every attempt consumes a
 * limited number of eFuse entries, and the driver expects twelve hex digits
 * rather than the raw bytes this method used to pass.
 *
 * Instead the address is handed to the driver through `rtw_initmac`, the
 * hook its probe routine parses right after reading the eFuse. The override
 * lives in RAM only and is gone after a reset.
 *
 * It has to be called while the radio is off (before the first WiFi.mode()
 * or WiFi.begin()); the address is applied when the driver starts.
 */
bool WiFiClass::setMacAddress(const uint8_t *mac) {
	if (!wifiMacIsAssignable(mac)) {
		LT_WM(WIFI, "Refusing unusable MAC address");
		return false;
	}
	if (getMode() != WIFI_MODE_NULL) {
		LT_WM(WIFI, "MAC address can only be changed while WiFi is off");
		return false;
	}

	stageMac(mac);
	LT_IM(WIFI, "MAC address override staged: %s", stagedMacText);
	return true;
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
