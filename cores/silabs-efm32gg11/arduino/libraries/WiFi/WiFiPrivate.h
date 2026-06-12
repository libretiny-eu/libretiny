/* Copyright (c) WGM160P-LibreTiny port 2026-06-10. */
#pragma once

#include <WiFi.h>

#include "WiFiStatusLed.h"

extern "C" {
#include "sl_wfx_host_conf.h"
#include "wf200_netif.h"

#include <lwip/dns.h>
#include <lwip/ip4_addr.h>
#include <lwip/netif.h>
#include <lwip/tcpip.h>

#include <sl_wfx.h>
}

// STA status state machine (staStatus):
//
//   WL_DISCONNECTED --begin()/reconnect()--> WL_IDLE_STATUS   (join sent; after
//                                            CONNECT ind: L2 up, DHCP pending)
//   WL_IDLE_STATUS  --netif got IP--------> WL_CONNECTED
//   WL_IDLE_STATUS/WL_CONNECTED --join refused / timeout--> WL_CONNECT_FAILED
//   WL_IDLE_STATUS/WL_CONNECTED --unsolicited DISCONNECT ind--> WL_CONNECTION_LOST
//   WL_CONNECTED   --IP lost (netif addr cleared)--> WL_DISCONNECTED
//   any            --disconnect()---------> WL_DISCONNECTED  (set BEFORE the
//                                            command, marking it intentional)
//   bring-up failure (host setup / sl_wfx_init / netif)----> WL_NO_SHIELD
typedef struct {
	bool stackUp;  // sl_wfx_init + netif done
	WiFiMode mode; // bookkeeping; WF200 STA interface only in Phase 2
	volatile WiFiStatus staStatus;
	// Last CONNECT indication data (event task fills; BSSID()/channel() read).
	uint8_t staBssid[6];
	uint16_t staChannel;
	// SERIALIZATION CONTRACT (sl_wfx_host_conf.h): the confirmation machinery
	// supports ONE in-flight command. ALL sl_wfx_* command calls (init, join,
	// disconnect, scan, ...) must be taken behind this mutex. TX data frames
	// are exempt (fire-and-forget; the netif glue handles them).
	SemaphoreHandle_t cmdMutex;
	SemaphoreHandle_t scanSem;
	WiFiNetworkInfo sta;
	WiFiNetworkInfo ap; // unused in Phase 2 (softAP is a follow-on)
	// softAP state (consumed by later softAP tasks; dormant until then).
	bool apActive;		   // softAP currently started
	uint8_t apClientCount; // tracked via AP_CLIENT_(DIS)CONNECTED_IND
	char *apSsid;		   // strdup'd; freed on disable
	char *apPassword;	   // strdup'd or NULL (open); freed on disable
	uint32_t apLocalIP;	   // default 192.168.4.1 (network byte order)
	uint32_t apSubnet;	   // default 255.255.255.0
	int32_t apChannel;	   // requested channel (0/unset = follow STA or default 1)
} WiFiData;

#define DATA  ((WiFiData *)data)
#define pDATA ((WiFiData *)pWiFi->data)
#define cDATA ((WiFiData *)cls->data)

// Command serialization helpers — Tasks 12/13 (join/disconnect/scan) follow
// the same pattern established in WiFiGeneric.cpp's modePriv().
#define WIFI_CMD_TAKE(d) xSemaphoreTake((d)->cmdMutex, portMAX_DELAY)
#define WIFI_CMD_GIVE(d) xSemaphoreGive((d)->cmdMutex)

// WiFiEvents.cpp
// Returns false if host setup or the event task couldn't be brought up;
// modePriv must fail cleanly (WL_NO_SHIELD) without calling sl_wfx_init().
bool startWifiEventTask();
// Installs the lwIP netif status callback (IP-acquisition -> WL_CONNECTED +
// ARDUINO_EVENT_WIFI_STA_GOT_IP). Must be called AFTER lt_wf200_netif_create()
// succeeds — the netif doesn't exist before modePriv's bring-up creates it.
void ltWifiAttachNetifCallback();
// Maps the WF200 scan-result security bitmask to Arduino auth modes.
// Lives in WiFiEvents.cpp (consolidated there; Task 13 must NOT duplicate it).
WiFiAuthMode wfxSecurityToAuthMode(uint32_t security);
