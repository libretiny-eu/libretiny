/* Copyright (c) Etienne Le Cousin 2024-03-13. */

#include "WiFiPrivate.h"

static void scanHandler(void *arg) {
	WiFiClass *cls	   = (WiFiClass *)pWiFi;
	WiFiScanData *scan = cls->scan;

	LT_HEAP_I();

	ln_list_t *list;
	uint8_t n = 0, node_count = 0;
	ap_info_node_t *pnode;

	wifi_manager_ap_list_update_enable(LN_FALSE);
	wifi_manager_reg_event_callback(WIFI_MGR_EVENT_STA_SCAN_COMPLETE, NULL);

	// 1.get ap info list.
	if (wifi_manager_get_ap_list(&list, &node_count)) {
		LT_EM(WIFI, "Failed to get scan result");
		goto end;
	}
	LT_IM(WIFI, "Found %d APs", node_count);

	cls->scanAlloc(node_count);
	if (!scan->ap) {
		LT_WM(WIFI, "scan->ap alloc failed");
		goto end;
	}

	// 2.get all ap info in the list.
	LN_LIST_FOR_EACH_ENTRY(pnode, ap_info_node_t, list, list) {
		uint8_t *mac	   = (uint8_t *)pnode->info.bssid;
		ap_info_t *ap_info = &pnode->info;

		scan->ap[n].ssid	= strdup(ap_info->ssid);
		scan->ap[n].auth	= securityTypeToAuthMode(ap_info->authmode);
		scan->ap[n].rssi	= ap_info->rssi;
		scan->ap[n].channel = ap_info->channel;
		memcpy(scan->ap[n].bssid.addr, mac, 6);
		n++;
	}

end:
	scan->timeout = 0;
	if (scan->running) {
		// running == false means it was discarded (timeout)
		scan->running = false;
		xSemaphoreGive(cDATA->scanSem);

		// Send event scan finished
		EventInfo eventInfo;
		memset(&eventInfo, 0, sizeof(EventInfo));
		eventInfo.wifi_scan_done.status = 0;
		eventInfo.wifi_scan_done.number = scan->count;
		pWiFi->postEvent(ARDUINO_EVENT_WIFI_SCAN_DONE, eventInfo);
	}
	// wifi_manager_ap_list_update_enable(LN_TRUE);
	// wifi_sta_disconnect();
	LT_HEAP_I();
	return;
}

int16_t WiFiClass::scanNetworks(bool async, bool showHidden, bool passive, uint32_t maxMsPerChannel, uint8_t channel) {
	if (scan && scan->running) {
		if (scan->timeout && millis() > scan->timeout) {
			LT_WM(WIFI, "Scan timeout, discarding");
			scan->running = false;
		} else {
			return WIFI_SCAN_RUNNING;
		}
	}
	enableSTA(true);
	scanDelete();
	scanInit();

	LT_IM(WIFI, "Starting WiFi scan");

	__wrap_ln_printf_disable();

	wifi_manager_reg_event_callback(WIFI_MGR_EVENT_STA_SCAN_COMPLETE, &scanHandler);
	wifi_manager_ap_list_update_enable(LN_TRUE);
	wifi_manager_cleanup_scan_results();

	wifi_scan_cfg_t scan_cfg = {
		.channel   = 0,
		.scan_type = WIFI_SCAN_TYPE_ACTIVE,
		.scan_time = 5,
	};
	wifi_sta_scan(&scan_cfg);

	LT_HEAP_I();

	scan->running = true;
	scan->timeout = millis() + maxMsPerChannel * 20 + 1000;

	int16_t ret = WIFI_SCAN_RUNNING;
	if (!async) {
		LT_IM(WIFI, "Waiting for results");
		xSemaphoreTake(DATA->scanSem, 1); // reset the semaphore quickly
		xSemaphoreTake(DATA->scanSem, pdMS_TO_TICKS(maxMsPerChannel * 20));
		if (scan->running) {
			scanDelete();
			ret = WIFI_SCAN_FAILED;
			goto exit;
		}
		ret = scan->count;
		goto exit;
	}

exit:
	__wrap_ln_printf_enable();
	return ret;
}
