/* Copyright (c) WGM160P-LibreTiny port 2026-06-12. */

// MIT glue over the GSDK Apache-2.0 DHCP server (dhcp_server.c, reused
// unmodified from framework-silabs-gecko-sdk via the apps/ include dir). That
// file binds its UDP :67 socket to the global `ap_netif` (our softAP netif,
// defined in wf200_netif.c) and reads its AP/server IP + netmask from the
// `ap_*` octet globals defined here. The compat/app_webpage.h shim declares all
// of them, replacing the GSDK example's webpage-coupled header.

#include "lt_dhcpserver.h"

#include "dhcp_server.h" // GSDK Apache-2.0, via the apps/ include dir
#include "lwip/def.h"
#include "lwip/netif.h"

// Definitions of the config octets the GSDK dhcp_server.c reads (declared in
// compat/app_webpage.h). dhcp_server.c builds handed-out leases as
// 192.168.4.{10..17} from ap_ip_addr0..2 when these are set to 192.168.4.x.
uint8_t ap_ip_addr0, ap_ip_addr1, ap_ip_addr2, ap_ip_addr3;
uint8_t ap_netmask_addr0, ap_netmask_addr1, ap_netmask_addr2, ap_netmask_addr3;

void lt_dhcpserver_start(struct netif *nif, uint32_t server_ip) {
	(void)nif; // dhcp_server.c binds the global &ap_netif (which IS our AP netif)
	// server_ip is network byte order (ip4_addr_t.addr); split MSB-first into
	// the dotted-quad octets dhcp_server.c expects.
	uint32_t host = lwip_ntohl(server_ip);
	ap_ip_addr0	  = (host >> 24) & 0xFF;
	ap_ip_addr1	  = (host >> 16) & 0xFF;
	ap_ip_addr2	  = (host >> 8) & 0xFF;
	ap_ip_addr3	  = host & 0xFF;
	// Hardcoded /24, tracking the softAP default (192.168.4.1/255.255.255.0);
	// the lt_dhcpserver_start API only carries the server IP, not the mask.
	// dhcp_server.c uses the mask only for DHCP option 1, and its lease pool
	// (host octet 10..17) stays within the /24.
	// NOTE: a non-/24 mask set via softAPConfig() is NOT propagated to DHCP
	// option 1 (the API passes only server_ip); widen lt_dhcpserver_start() to
	// take a mask if needed.
	ap_netmask_addr0 = 255;
	ap_netmask_addr1 = 255;
	ap_netmask_addr2 = 255;
	ap_netmask_addr3 = 0;
	dhcpserver_start();
}

void lt_dhcpserver_stop(void) {
	dhcpserver_stop();
}
