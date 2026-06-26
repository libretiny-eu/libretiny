/* Copyright (c) WGM160P-LibreTiny port 2026-06-12. */
#pragma once

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

struct netif;

// Thin MIT glue over the GSDK Apache-2.0 dhcp_server.c (reused unmodified from
// framework-silabs-gecko-sdk). lt_dhcpserver_start() sets the AP IP/mask octets
// the reused server reads (via compat/app_webpage.h) and starts it on the AP
// netif. server_ip is the AP's own address (network byte order ip4_addr_t.addr
// word); the server hands out leases on the same /24.
void lt_dhcpserver_start(struct netif *nif, uint32_t server_ip);
void lt_dhcpserver_stop(void);

#ifdef __cplusplus
}
#endif
