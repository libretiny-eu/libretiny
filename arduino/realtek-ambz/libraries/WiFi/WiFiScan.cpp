#include "WiFi.h"
#include "WiFiPriv.h"

rtw_result_t WiFiClass::scanHandler(rtw_scan_handler_result_t *result) {
	WiFiClass *cls = (WiFiClass *)result->user_data;

	if (result->scan_complete == RTW_TRUE) {
		cls->_scanning = false;
		xSemaphoreGive(cls->_scanSem);
		return RTW_SUCCESS;
	}

	rtw_scan_result_t *net		 = &result->ap_details;
	net->SSID.val[net->SSID.len] = '\0';

	uint8_t newSize	 = cls->_netCount + 1;
	cls->_netSsid	 = (char **)realloc(cls->_netSsid, newSize * sizeof(char *));
	cls->_netEncr	 = (WiFiAuthMode *)realloc(cls->_netEncr, newSize * sizeof(WiFiAuthMode));
	cls->_netRssi	 = (int32_t *)realloc(cls->_netRssi, newSize * sizeof(int32_t));
	cls->_netBssid	 = (rtw_mac_t *)realloc(cls->_netBssid, newSize * sizeof(rtw_mac_t));
	cls->_netChannel = (int32_t *)realloc(cls->_netChannel, newSize * sizeof(int32_t));

	cls->_netSsid[cls->_netCount] = (char *)malloc((net->SSID.len + 1) * sizeof(char));
	strcpy(cls->_netSsid[cls->_netCount], (char *)net->SSID.val);
	cls->_netEncr[cls->_netCount] = WiFiClass::securityTypeToAuthMode(net->security);
	cls->_netRssi[cls->_netCount] = net->signal_strength;
	memcpy(cls->_netBssid[cls->_netCount].octet, net->BSSID.octet, ETH_ALEN);
	cls->_netChannel[cls->_netCount] = net->channel;
	cls->_netCount++;

	return RTW_SUCCESS;
}

int16_t WiFiClass::scanNetworks(bool async, bool showHidden, bool passive, uint32_t maxMsPerChannel, uint8_t channel) {
	if (_scanning)
		return WIFI_SCAN_RUNNING;
	if (wifi_mode == WIFI_MODE_NULL)
		enableSTA(true);
	scanDelete();

	LT_I("Starting WiFi scan");

	if (wifi_scan_networks(scanHandler, this) != RTW_SUCCESS)
		return WIFI_SCAN_FAILED;

	_scanning = true;

	if (!async) {
		LT_I("Waiting for results");
		xSemaphoreTake(_scanSem, 1); // reset the semaphore quickly
		xSemaphoreTake(_scanSem, pdMS_TO_TICKS(maxMsPerChannel * 20));
		return _netCount;
	}
	return WIFI_SCAN_RUNNING;
}

bool WiFiClass::getNetworkInfo(
	uint8_t networkItem, String &ssid, WiFiAuthMode &encType, int32_t &rssi, uint8_t *&bssid, int32_t &channel
) {
	ssid	= SSID(networkItem);
	encType = encryptionType(networkItem);
	rssi	= RSSI(networkItem);
	bssid	= BSSID(networkItem);
	channel = this->channel(networkItem);
}

int16_t WiFiClass::scanComplete() {
	if (_scanning)
		return WIFI_SCAN_RUNNING;
	return _netCount;
}

void WiFiClass::scanDelete() {
	for (uint8_t i = 0; i < _netCount; i++) {
		free(_netSsid[i]);
	}
	free(_netSsid);
	free(_netEncr);
	free(_netRssi);
	free(_netBssid);
	free(_netChannel);
	_netCount	= 0;
	_netSsid	= NULL;
	_netEncr	= NULL;
	_netRssi	= NULL;
	_netBssid	= NULL;
	_netChannel = NULL;
}

String WiFiClass::SSID(uint8_t networkItem) {
	if (networkItem >= _netCount)
		return "";
	return _netSsid[networkItem];
}

WiFiAuthMode WiFiClass::encryptionType(uint8_t networkItem) {
	if (networkItem >= _netCount)
		return WIFI_AUTH_INVALID;
	return _netEncr[networkItem];
}

int32_t WiFiClass::RSSI(uint8_t networkItem) {
	if (networkItem >= _netCount)
		return 0;
	return _netRssi[networkItem];
}

uint8_t *WiFiClass::BSSID(uint8_t networkItem) {
	if (networkItem >= _netCount)
		return NULL;
	return _netBssid[networkItem].octet;
}

String WiFiClass::BSSIDstr(uint8_t networkItem) {
	return macToString(BSSID(networkItem));
}

int32_t WiFiClass::channel(uint8_t networkItem) {
	if (networkItem >= _netCount)
		return 0;
	return _netChannel[networkItem];
}
