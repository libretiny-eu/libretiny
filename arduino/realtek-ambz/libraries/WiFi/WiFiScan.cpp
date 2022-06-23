/* Copyright (c) Kuba Szczodrzyński 2022-04-25. */

#include "WiFiPriv.h"

static rtw_result_t scanHandler(rtw_scan_handler_result_t *result) {
	WiFiClass *cls	   = (WiFiClass *)result->user_data;
	WiFiScanData *scan = cls->scan;
	if (!scan)
		return RTW_SUCCESS;

	if (result->scan_complete == RTW_TRUE) {
		scan->running = false;
		xSemaphoreGive(cls->data.scanSem);
		return RTW_SUCCESS;
	}

	rtw_scan_result_t *net		 = &result->ap_details;
	net->SSID.val[net->SSID.len] = '\0';

	uint8_t newSize = scan->count + 1;
	scan->ssid		= (char **)realloc(scan->ssid, newSize * sizeof(char *));
	scan->auth		= (WiFiAuthMode *)realloc(scan->auth, newSize * sizeof(WiFiAuthMode));
	scan->rssi		= (int32_t *)realloc(scan->rssi, newSize * sizeof(int32_t));
	scan->bssid		= (WiFiMacAddr *)realloc(scan->bssid, newSize * sizeof(WiFiMacAddr));
	scan->channel	= (int32_t *)realloc(scan->channel, newSize * sizeof(int32_t));

	scan->ssid[scan->count] = (char *)malloc((net->SSID.len + 1) * sizeof(char));
	strcpy(scan->ssid[scan->count], (char *)net->SSID.val);
	scan->auth[scan->count] = securityTypeToAuthMode(net->security);
	scan->rssi[scan->count] = net->signal_strength;
	memcpy(scan->bssid[scan->count].addr, net->BSSID.octet, ETH_ALEN);
	scan->channel[scan->count] = net->channel;
	scan->count++;

	return RTW_SUCCESS;
}

int16_t WiFiClass::scanNetworks(bool async, bool showHidden, bool passive, uint32_t maxMsPerChannel, uint8_t channel) {
	if (scan && scan->running)
		return WIFI_SCAN_RUNNING;
	if (wifi_mode == WIFI_MODE_NULL)
		enableSTA(true);
	scanDelete();
	scanInit();

	LT_I("Starting WiFi scan");

	if (wifi_scan_networks(scanHandler, this) != RTW_SUCCESS)
		return WIFI_SCAN_FAILED;

	scan->running = true;

	if (!async) {
		LT_I("Waiting for results");
		xSemaphoreTake(data.scanSem, 1); // reset the semaphore quickly
		xSemaphoreTake(data.scanSem, pdMS_TO_TICKS(maxMsPerChannel * 20));
		return scan->count;
	}
	return WIFI_SCAN_RUNNING;
}
