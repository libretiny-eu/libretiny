/* Copyright (c) Etienne Le Cousin 2024-02-10. */

#pragma once

#include <WiFi.h>
#include <sdk_private.h>

extern "C" {
// copy defines from PIO builder (for IDE to understand)
#define LWIP_TIMEVAL_PRIVATE 0
#define LWIP_NETIF_HOSTNAME	 1
#define LWIP_SO_RCVBUF		 1

#include <lwip/api.h>
#include <lwip/dns.h>
#include <lwip/ip_addr.h>
#include <lwip/netif.h>
#include <lwip/netifapi.h>
#include <netif/ethernetif.h>

#include <FreeRTOS.h>
#include <semphr.h>

} // extern "C"

// WiFi.cpp
extern WiFiAuthMode securityTypeToAuthMode(uint8_t type);
uint8_t authModeToSecurityType(WiFiAuthMode auth);

// WiFiEvents.cpp
extern void wifiEventSendArduino(EventId event);
extern void registerWifiHandlers();

typedef struct {
	bool initialized;
	bool sleep;
	WiFiMode mode;
	SemaphoreHandle_t scanSem;
	WiFiNetworkInfo sta;
	WiFiNetworkInfo ap;
} WiFiData;

#define DATA  ((WiFiData *)data)
#define pDATA ((WiFiData *)pWiFi->data)
#define cDATA ((WiFiData *)cls->data)

#define IP_FMT "%u.%u.%u.%u"
