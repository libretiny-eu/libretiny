#include "WiFi.h"
#include "WiFiPriv.h"

int32_t WiFiClass::channel() {
	int channel;
	wifi_get_channel(&channel);
	return channel;
}

bool WiFiClass::mode(WiFiMode mode) {
	WiFiMode currentMode = getMode();
	if (mode == currentMode)
		return true;

	if (!currentMode && mode && !_initialized) {
		// initialize wifi first
		LwIP_Init();
		reset_wifi_struct();
		wifi_manager_init();
		_initialized = true;
	}
	if (currentMode) {
		// stop wifi to change mode
		dhcps_deinit();
		if (wifi_off() != RTW_SUCCESS)
			return false;
		vTaskDelay(20);
	}

	if (wifi_on((rtw_mode_t)mode) != RTW_SUCCESS)
		return false;

	if (mode & WIFI_MODE_AP) {
		dhcps_init(NETIF_RTW_STA);
	}
	return true;
}

WiFiMode WiFiClass::getMode() {
	if (!_initialized)
		return WIFI_MODE_NULL;
	return (WiFiMode)wifi_mode;
}

WiFiStatus WiFiClass::status() {
	if (wifi_is_connected_to_ap() == 0) {
		return WL_CONNECTED;
	} else {
		return WL_DISCONNECTED;
	}
}

bool WiFiClass::enableSTA(bool enable) {
	WiFiMode currentMode = getMode();
	if ((currentMode & WIFI_MODE_STA != 0) != enable) {
		return mode((WiFiMode)(currentMode ^ WIFI_MODE_STA));
	}
	return true;
}

bool WiFiClass::enableAP(bool enable) {
	WiFiMode currentMode = getMode();
	if ((currentMode & WIFI_MODE_AP != 0) != enable) {
		return mode((WiFiMode)(currentMode ^ WIFI_MODE_AP));
	}
	return true;
}

bool WiFiClass::setSleep(bool enable) {
	if (enable)
		if (wifi_enable_powersave() != RTW_SUCCESS)
			return false;
		else if (wifi_disable_powersave() != RTW_SUCCESS)
			return false;
	_sleep = enable;
	return true;
}

bool WiFiClass::getSleep() {
	return _sleep;
}

bool WiFiClass::setTxPower(int power) {
	return false; // wifi_set_txpower(power) == RTW_SUCCESS;
}

int WiFiClass::getTxPower() {
	return 0;
	int power = 0;
	wifi_get_txpower(&power);
	return power;
}

IPAddress WiFiClass::hostByName(const char *hostname) {
	ip_addr_t ip;
	int ret = netconn_gethostbyname(hostname, &ip);
	if (ret == ERR_OK) {
		return ip.addr;
	}
	return IPAddress();
}

IPAddress WiFiClass::calculateNetworkID(IPAddress ip, IPAddress subnet) {
	IPAddress networkID;

	for (size_t i = 0; i < 4; i++)
		networkID[i] = subnet[i] & ip[i];

	return networkID;
}

IPAddress WiFiClass::calculateBroadcast(IPAddress ip, IPAddress subnet) {
	IPAddress broadcastIp;

	for (int i = 0; i < 4; i++)
		broadcastIp[i] = ~subnet[i] | ip[i];

	return broadcastIp;
}

uint8_t WiFiClass::calculateSubnetCIDR(IPAddress subnetMask) {
	uint8_t CIDR = 0;

	for (uint8_t i = 0; i < 4; i++) {
		if (subnetMask[i] == 0x80) // 128
			CIDR += 1;
		else if (subnetMask[i] == 0xC0) // 192
			CIDR += 2;
		else if (subnetMask[i] == 0xE0) // 224
			CIDR += 3;
		else if (subnetMask[i] == 0xF0) // 242
			CIDR += 4;
		else if (subnetMask[i] == 0xF8) // 248
			CIDR += 5;
		else if (subnetMask[i] == 0xFC) // 252
			CIDR += 6;
		else if (subnetMask[i] == 0xFE) // 254
			CIDR += 7;
		else if (subnetMask[i] == 0xFF) // 255
			CIDR += 8;
	}

	return CIDR;
}

String WiFiClass::macToString(uint8_t *mac) {
	char macStr[ETH_ALEN * 3];
	sprintf(macStr, "%02X:%02X:%02X:%02X:%02X:%02X", mac[0], mac[1], mac[2], mac[3], mac[4], mac[5]);
	return macStr;
}
