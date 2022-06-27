/* Copyright (c) Kuba Szczodrzyński 2022-06-27. */

#include "WiFiPriv.h"

static void scanHandler(void *ctx, uint8_t param) {
	LT_HEAP_I();
	WiFiClass *cls = (WiFiClass *)ctx;
	if (!cls) {
		LT_W("Called without ctx");
		return;
	}
	WiFiScanData *scan = cls->scan;
	if (!scan) {
		LT_W("Called without cls->scan");
		return;
	}

	struct sta_scan_res *result;
	uint8_t count = bk_wlan_get_scan_ap_result_numbers();
	if (count == 0) {
		LT_D_WG("No APs found");
		goto end;
	}
	LT_D_WG("Found %d APs", count);

	result = (struct sta_scan_res *)malloc(sizeof(struct sta_scan_res) * count);
	if (!result) {
		LT_W("sta_scan_res alloc failed");
		goto end;
	}
	bk_wlan_get_scan_ap_result(result, count);

	cls->scanAlloc(count);
	if (!scan->ap) {
		LT_W("scan->ap alloc failed");
		goto end;
	}

	for (uint8_t i = 0; i < count; i++) {
		scan->ap[i].ssid	= strdup(result[i].ssid);
		scan->ap[i].auth	= securityTypeToAuthMode(result[i].security);
		scan->ap[i].rssi	= result[i].level;
		scan->ap[i].channel = result[i].channel;
		memcpy(scan->ap[i].bssid.addr, result[i].bssid, 6);
	}

end:
	scan->running = false;
	xSemaphoreGive(cls->data.scanSem);
	LT_HEAP_I();
	return;
}

int16_t WiFiClass::scanNetworks(bool async, bool showHidden, bool passive, uint32_t maxMsPerChannel, uint8_t channel) {
	if (scan && scan->running)
		return WIFI_SCAN_RUNNING;
	enableSTA(true);
	scanDelete();
	scanInit();

	LT_I("Starting WiFi scan");

	__wrap_bk_printf_disable();
	mhdr_scanu_reg_cb(scanHandler, this);
	bk_wlan_start_scan();
	__wrap_bk_printf_enable();

	LT_HEAP_I();

	scan->running = true;

	if (!async) {
		LT_I("Waiting for results");
		xSemaphoreTake(data.scanSem, 1); // reset the semaphore quickly
		xSemaphoreTake(data.scanSem, pdMS_TO_TICKS(maxMsPerChannel * 20));
		return scan->count;
	}
	return WIFI_SCAN_RUNNING;
}
