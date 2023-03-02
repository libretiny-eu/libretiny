/* Copyright (c) Kuba Szczodrzyński 2022-04-25. */

#include "WiFiPrivate.h"

static rtw_result_t scanHandler(rtw_scan_handler_result_t *result) {
	WiFiClass *cls	   = (WiFiClass *)result->user_data;
	WiFiScanData *scan = cls->scan;
	if (!scan)
		return RTW_SUCCESS;

	if (result->scan_complete == RTW_TRUE) {
		scan->running = false;
		xSemaphoreGive(cDATA->scanSem);
		return RTW_SUCCESS;
	}

	rtw_scan_result_t *net		 = &result->ap_details;
	net->SSID.val[net->SSID.len] = '\0';

	if (!net->SSID.len)
		return RTW_SUCCESS;

	uint8_t last = cls->scanAlloc(scan->count + 1);

	scan->ap[last].ssid	   = strdup((char *)net->SSID.val);
	scan->ap[last].auth	   = securityTypeToAuthMode(net->security);
	scan->ap[last].rssi	   = net->signal_strength;
	scan->ap[last].channel = net->channel;
	memcpy(scan->ap[last].bssid.addr, net->BSSID.octet, ETH_ALEN);

	return RTW_SUCCESS;
}

int16_t WiFiClass::scanNetworks(bool async, bool showHidden, bool passive, uint32_t maxMsPerChannel, uint8_t channel) {
	if (scan && scan->running)
		return WIFI_SCAN_RUNNING;
	if (wifi_mode == WIFI_MODE_NULL)
		enableSTA(true);
	scanDelete();
	scanInit();

	LT_IM(WIFI, "Starting WiFi scan");

	if (wifi_scan_networks(scanHandler, this) != RTW_SUCCESS)
		return WIFI_SCAN_FAILED;

	scan->running = true;

	if (!async) {
		LT_IM(WIFI, "Waiting for results");
		xSemaphoreTake(DATA->scanSem, 1); // reset the semaphore quickly
		xSemaphoreTake(DATA->scanSem, pdMS_TO_TICKS(maxMsPerChannel * 20));
		return scan->count;
	}
	return WIFI_SCAN_RUNNING;
}
