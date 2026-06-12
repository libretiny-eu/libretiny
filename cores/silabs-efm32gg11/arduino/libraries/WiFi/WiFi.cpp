/* Copyright (c) WGM160P-LibreTiny port 2026-06-10. */

#include "WiFiPrivate.h"

WiFiClass::WiFiClass() {
	data		   = (WiFiData *)calloc(1, sizeof(WiFiData));
	DATA->cmdMutex = xSemaphoreCreateMutex();
	DATA->scanSem  = xSemaphoreCreateBinary();
	DATA->mode	   = WIFI_MODE_NULL;
	// WiFiType.h has no "stopped" status; Disconnected is the closest
	// pre-bring-up state (WL_IDLE_STATUS == 0 would imply a join in progress).
	DATA->staStatus = WL_DISCONNECTED;
}

WiFiClass::~WiFiClass() {
	// free strdup'd ssid/password/bssid before dropping the struct
	resetNetworkInfo(DATA->sta);
	resetNetworkInfo(DATA->ap);
	vSemaphoreDelete(DATA->cmdMutex);
	vSemaphoreDelete(DATA->scanSem);
	free(data);
	data = NULL;
}
