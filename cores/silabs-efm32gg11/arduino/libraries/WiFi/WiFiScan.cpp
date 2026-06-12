/* Copyright (c) WGM160P-LibreTiny port 2026-06-10. */

#include "WiFiPrivate.h"

// NOTE: wfxSecurityToAuthMode() lives in WiFiEvents.cpp (the event task is its
// primary consumer) — declared in WiFiPrivate.h, do not duplicate here.

// UAF WARNING: scanDelete() while a scan is running is a use-after-free — the
// common scanDelete() has no running-scan guard and the event task writes into
// scan->ap as results arrive. The sync path below waits on scanSem before
// returning; ASYNC callers must wait for the SCAN_DONE event (or poll
// scanComplete() != WIFI_SCAN_RUNNING) before calling scanDelete().
int16_t WiFiClass::scanNetworks(bool async, bool showHidden, bool passive, uint32_t maxMsPerChannel, uint8_t channel) {
	(void)showHidden;
	if (scan && scan->running)
		return WIFI_SCAN_RUNNING;
	if (!enableSTA(true))
		return WIFI_SCAN_FAILED;

	scanDelete();
	scanInit();
	if (!scan)
		return WIFI_SCAN_FAILED;

	uint8_t channel_list[1] = {channel};
	LT_IM(WIFI, "Starting WiFi scan");
	// running=true BEFORE the command: the first SCAN_RESULT indications can
	// beat this thread, and the event task drops results while !running.
	scan->running = true;
	xSemaphoreTake(DATA->scanSem, 0); // drain a stale give
	WIFI_CMD_TAKE(DATA);
	sl_status_t st = sl_wfx_send_scan_command(
		passive ? WFM_SCAN_MODE_PASSIVE : WFM_SCAN_MODE_ACTIVE,
		channel ? channel_list : NULL,
		channel ? 1 : 0,
		NULL,
		0,
		NULL,
		0,
		NULL
	);
	WIFI_CMD_GIVE(DATA);
	if (st != SL_STATUS_OK && st != SL_STATUS_WIFI_WARNING) {
		LT_EM(WIFI, "scan command failed: 0x%08lX", (unsigned long)st);
		scan->running = false;
		return WIFI_SCAN_FAILED;
	}

	if (async)
		return WIFI_SCAN_RUNNING;

	// Sync: scan-complete indication gives scanSem (bounded).
	uint32_t timeout = (maxMsPerChannel ? maxMsPerChannel : 300) * 14 + 2000;
	if (xSemaphoreTake(DATA->scanSem, pdMS_TO_TICKS(timeout)) != pdTRUE) {
		LT_EM(WIFI, "scan timed out");
		scan->running = false;
		WIFI_CMD_TAKE(DATA);
		sl_wfx_send_stop_scan_command();
		WIFI_CMD_GIVE(DATA);
		return WIFI_SCAN_FAILED;
	}
	return scan->count;
}
