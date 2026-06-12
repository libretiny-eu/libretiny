/* Copyright (c) WGM160P-LibreTiny port 2026-06-12. */
#pragma once

#include "lwip/netif.h"

// Config surface consumed by the GSDK Apache-2.0 dhcp_server.c (reused
// unmodified from framework-silabs-gecko-sdk). Definitions live in
// lt_dhcpserver.c (the IP/netmask octets) and wf200_netif.c (ap_netif). This
// shim replaces the GSDK example's app_webpage.h, which otherwise drags in the
// webpage/Micrium example surface; dhcp_server.c references nothing else from
// it (the AP IP octets, the netmask octets, and the AP netif object).
extern uint8_t ap_ip_addr0, ap_ip_addr1, ap_ip_addr2, ap_ip_addr3;
extern uint8_t ap_netmask_addr0, ap_netmask_addr1, ap_netmask_addr2, ap_netmask_addr3;
extern struct netif ap_netif;
