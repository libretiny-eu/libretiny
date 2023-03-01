/* Copyright (c) Kuba Szczodrzyński 2022-06-27. */

#include "WiFiPrivate.h"

static void scanHandler(void *ctx, uint8_t param) {
	LT_HEAP_I();
	WiFiClass *cls = (WiFiClass *)ctx;
	if (!cls) {
		LT_WM(WIFI, "Called without ctx");
		return;
	}
	WiFiScanData *scan = cls->scan;
	if (!scan) {
		LT_WM(WIFI, "Called without cls->scan");
		return;
	}

	ScanResult_adv result;
	if (wlan_sta_scan_result(&result)) {
		LT_EM(WIFI, "Failed to get scan result");
		goto end;
	}
	LT_IM(WIFI, "Found %d APs", result.ApNum);

	cls->scanAlloc(result.ApNum);
	if (!scan->ap) {
		LT_WM(WIFI, "scan->ap alloc failed");
		goto end;
	}

	for (uint8_t i = 0; i < result.ApNum; i++) {
		scan->ap[i].ssid	= strdup(result.ApList[i].ssid);
		scan->ap[i].auth	= securityTypeToAuthMode(result.ApList[i].security);
		scan->ap[i].rssi	= result.ApList[i].ApPower;
		scan->ap[i].channel = result.ApList[i].channel;
		memcpy(scan->ap[i].bssid.addr, result.ApList[i].bssid, 6);
	}

	cls->data.scannedAt = millis();

	wifiEventSendArduino(ARDUINO_EVENT_WIFI_SCAN_DONE);

end:
	scan->timeout = 0;
	if (scan->running) {
		// running == false means it was discarded (timeout)
		scan->running = false;
		xSemaphoreGive(cls->data.scanSem);
	}
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

	__wrap_bk_printf_disable();
	mhdr_scanu_reg_cb(scanHandler, this);
	bk_wlan_start_scan();

	LT_HEAP_I();

	scan->running = true;
	scan->timeout = millis() + maxMsPerChannel * 20 + 1000;

	int16_t ret = WIFI_SCAN_RUNNING;
	if (!async) {
		LT_IM(WIFI, "Waiting for results");
		xSemaphoreTake(data.scanSem, 1); // reset the semaphore quickly
		xSemaphoreTake(data.scanSem, pdMS_TO_TICKS(maxMsPerChannel * 20));
		if (scan->running) {
			scanDelete();
			ret = WIFI_SCAN_FAILED;
			goto exit;
		}
		ret = scan->count;
		goto exit;
	}

exit:
	__wrap_bk_printf_enable();
	return ret;
}
