/* Copyright (c) Kuba Szczodrzyński 2022-05-16. */

#include "WiFiPrivate.h"

#include <vector>

#define WIFI_EVENT_MAX_ROW 3

// Lowest address treated as a real pointer in wifi_indication(). Drivers on
// some families pass a small integer in buf instead of a pointer.
#define WIFI_EVENT_BUF_MIN_ADDR 0x1000

static xQueueHandle wifiEventQueueHandle = NULL;
static xTaskHandle wifiEventTaskHandle	 = NULL;

// C code to support SDK-defined events (in wifi_conf.c)
extern "C" {
// SDK events
static event_list_elem_t event_callback_list[WIFI_EVENT_MAX][WIFI_EVENT_MAX_ROW];

typedef struct {
	rtw_event_indicate_t event;
	char *buf;
	int buf_len;
	int flags;
} rtw_event_t;

// reset callbacks
void init_event_callback_list() {
	memset(event_callback_list, 0, sizeof(event_callback_list));
}

// dummy
int wifi_manager_init() {
	return 0;
}

void wifi_reg_event_handler(unsigned int event_cmds, rtw_event_handler_t handler_func, void *handler_user_data) {
	int i = 0, j = 0;
	if (event_cmds < WIFI_EVENT_MAX) {
		for (i = 0; i < WIFI_EVENT_MAX_ROW; i++) {
			if (event_callback_list[event_cmds][i].handler == NULL) {
				for (j = 0; j < WIFI_EVENT_MAX_ROW; j++) {
					if (event_callback_list[event_cmds][j].handler == handler_func) {
						return;
					}
				}
				event_callback_list[event_cmds][i].handler			 = handler_func;
				event_callback_list[event_cmds][i].handler_user_data = handler_user_data;
				return;
			}
		}
	}
}

void wifi_unreg_event_handler(unsigned int event_cmds, rtw_event_handler_t handler_func) {
	int i;
	if (event_cmds < WIFI_EVENT_MAX) {
		for (i = 0; i < WIFI_EVENT_MAX_ROW; i++) {
			if (event_callback_list[event_cmds][i].handler == handler_func) {
				event_callback_list[event_cmds][i].handler			 = NULL;
				event_callback_list[event_cmds][i].handler_user_data = NULL;
				return;
			}
		}
	}
}
} // extern "C"

// TODO queue handling is apparently done all wrong here
// (passing pointers to pointers in xQueueSend/xQueueReceive)

// function called by wext_wlan_indicate
void wifi_indication(rtw_event_indicate_t event, char *buf, int buf_len, int flags) {
	if (event >= WIFI_EVENT_MAX)
		return;
	if (wifiEventQueueHandle && wifiEventTaskHandle) {
		rtw_event_t *ev = (rtw_event_t *)malloc(sizeof(rtw_event_t));
		if (!ev) {
			LT_EM(WIFI, "Event %u dropped: no heap for event record", event);
			return;
		}
		// flags == -2 is an Arduino event: buf points to an EventInfo and buf_len is
		// the event id. Some drivers pass a value instead of a pointer in buf, so copy
		// only from a real address; test buf_len signed before the cast.
		char *bufCopy = NULL;
		if ((uintptr_t)buf >= WIFI_EVENT_BUF_MIN_ADDR && (flags == -2 || buf_len > 0)) {
			size_t copyLen = flags == -2 ? sizeof(EventInfo) : (size_t)buf_len;
			// copy data to allow freeing from calling scopes
			bufCopy = (char *)malloc(copyLen);
			if (!bufCopy) {
				// drop the event instead of queueing a NULL payload: !bufCopy
				// below then means exactly "no payload was supplied"
				LT_EM(WIFI, "Event %u dropped: no heap for %u byte payload", event, (unsigned)copyLen);
				free(ev);
				return;
			}
			memcpy(bufCopy, buf, copyLen);
		}
		ev->buf = bufCopy;
		if (!bufCopy && flags != -2)
			// no payload survived, so the length must not be forwarded either;
			// on flags == -2 buf_len is the event id, not a length — keep it
			buf_len = 0;
		ev->event	= event;
		ev->buf_len = buf_len;
		ev->flags	= flags;
		xQueueSend(wifiEventQueueHandle, &ev, portMAX_DELAY);
	} else {
		handleRtwEvent(event, buf, buf_len, flags);
	}
}

static void wifiEventTask(void *arg) {
	rtw_event_t *data = NULL;
	for (;;) {
		if (xQueueReceive(wifiEventQueueHandle, &data, portMAX_DELAY) == pdTRUE) {
			handleRtwEvent(data->event, data->buf, data->buf_len, data->flags);
			// the queue owns the copy made in wifi_indication, for every flags
			// value; handleRtwEvent() never frees what it is passed
			if (data->buf) {
				free(data->buf);
			}
			free(data);
		}
	}
}

void startWifiTask() {
	if (!wifiEventQueueHandle) {
		LT_HEAP_I();
		wifiEventQueueHandle = xQueueCreate(32, sizeof(rtw_event_t *));
		LT_HEAP_I();
	}
	if (!wifiEventTaskHandle) {
		LT_HEAP_I();
		xTaskCreate(wifiEventTask, "wifievent", 512, NULL, 4, &wifiEventTaskHandle);
		LT_HEAP_I();
	}
}

void handleRtwEvent(uint16_t event, char *data, int len, int flags) {
	if (!pWiFi)
		return; // failsafe
	if (flags == -2) {
		// already an Arduino event, just pass it
		EventId eventId		 = (EventId)len;
		EventInfo *eventInfo = (EventInfo *)data;
		EventInfo empty		 = {};
		// the caller owns data: the queue frees its copy in wifiEventTask(),
		// and direct callers free their own buffer after wifi_indication()
		pWiFi->postEvent(eventId, eventInfo ? *eventInfo : empty);
		return;
	}

	// send to SDK listeners
	for (uint8_t i = 0; i < WIFI_EVENT_MAX_ROW; i++) {
		rtw_event_handler_t handler = event_callback_list[event][i].handler;
		if (!handler)
			continue;
		handler(data, len, flags, event_callback_list[event][i].handler_user_data);
	}

	if (pWiFi == NULL)
		return;

	EventId eventId;
	EventInfo eventInfo;
	String ssid;

	memset(&eventInfo, 0, sizeof(EventInfo));

	switch (event) {
		case WIFI_EVENT_DISCONNECT:
		case WIFI_EVENT_RECONNECTION_FAIL:
			eventId									 = ARDUINO_EVENT_WIFI_STA_DISCONNECTED;
			eventInfo.wifi_sta_disconnected.ssid_len = 0;
			eventInfo.wifi_sta_disconnected.reason	 = WIFI_REASON_UNSPECIFIED;
			if (event == WIFI_EVENT_RECONNECTION_FAIL)
				eventInfo.wifi_sta_disconnected.reason = WIFI_REASON_CONNECTION_FAIL;
			break;

		case WIFI_EVENT_FOURWAY_HANDSHAKE_DONE:
			eventId								  = ARDUINO_EVENT_WIFI_STA_CONNECTED;
			ssid								  = pWiFi->SSID();
			eventInfo.wifi_sta_connected.ssid_len = ssid.length();
			eventInfo.wifi_sta_connected.channel  = pWiFi->channel();
			eventInfo.wifi_sta_connected.authmode = pWiFi->getEncryption();
			memcpy(eventInfo.wifi_sta_connected.ssid, ssid.c_str(), eventInfo.wifi_sta_connected.ssid_len + 1);
			memcpy(eventInfo.wifi_sta_connected.bssid, pWiFi->BSSID(), 6);
			break;

		case WIFI_EVENT_SCAN_DONE:
			eventId							= ARDUINO_EVENT_WIFI_SCAN_DONE;
			eventInfo.wifi_scan_done.status = 0;
			if (pWiFi->scan)
				eventInfo.wifi_scan_done.number = pWiFi->scan->count;
			eventInfo.wifi_scan_done.scan_id = 0;
			break;

		case WIFI_EVENT_STA_ASSOC:
			// data(124) has MAC at 0x0A
			if (len != 124)
				return;
			eventId = ARDUINO_EVENT_WIFI_AP_STACONNECTED;
			memcpy(eventInfo.wifi_ap_staconnected.mac, (const char *)data + 10, 6);
			break;

		case WIFI_EVENT_STA_DISASSOC:
			// data(6) is MAC
			if (!data || len < 6)
				return;
			eventId = ARDUINO_EVENT_WIFI_AP_STADISCONNECTED;
			memcpy(eventInfo.wifi_ap_stadisconnected.mac, (const char *)data, 6);
			break;

		// case WIFI_EVENT_CONNECT:
		// case WIFI_EVENT_SCAN_RESULT_REPORT:
		// case WIFI_EVENT_SEND_ACTION_DONE:
		// case WIFI_EVENT_RX_MGNT:
		// case WIFI_EVENT_STA_WPS_START:
		// case WIFI_EVENT_WPS_FINISH:
		// case WIFI_EVENT_EAPOL_START:
		// case WIFI_EVENT_EAPOL_RECVD:
		// case WIFI_EVENT_NO_NETWORK:
		// case WIFI_EVENT_BEACON_AFTER_DHCP:
		// case WIFI_EVENT_IP_CHANGED:
		// case WIFI_EVENT_ICV_ERROR:
		// case WIFI_EVENT_CHALLENGE_FAIL:
		default:
			return;
	}

	pWiFi->postEvent(eventId, eventInfo);
}
