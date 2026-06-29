/* Copyright (c) WGM160P-LibreTiny port 2026-06-10. */
#pragma once

#include <sl_wfx.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

struct netif;

// One-time: tcpip_init() + netif_add for the STA interface. Needs wfx_context
// populated (call AFTER sl_wfx_init succeeds — MAC comes from the chip).
// Idempotent. Returns true on success.
bool lt_wf200_netif_create(void);

// Read-only queries or tcpip_callback context only — direct netif mutation from
// other threads races the tcpip thread.
struct netif *lt_wf200_netif_sta(void);
struct netif *lt_wf200_netif_ap(void);

// AP netif lifecycle (static IP, no DHCP client). Self-serializes against the
// tcpip thread via LOCK_TCPIP_CORE. Needs lt_wf200_netif_create() to have run
// first (it performs the one-time tcpip_init). ip/mask/gw are network-order
// (lwIP ip4_addr_t.addr) words. create/destroy are expected to be called from a
// single thread (the WiFi event task) — they are not serialized against each
// other, only against the tcpip thread. Both are idempotent.
bool lt_wf200_netif_ap_create(uint32_t ip, uint32_t mask, uint32_t gw, const uint8_t mac[6]);
// Idempotent.
void lt_wf200_netif_ap_destroy(void);

// Link state from the WiFi event task (thread-safe: defers to tcpip thread).
void lt_wf200_netif_set_link(bool up);

// DHCP client control (thread-safe).
void lt_wf200_netif_dhcp_start(void);
void lt_wf200_netif_dhcp_stop(void);

// RX entry, called from sl_wfx_host_post_event (bus task context). Overrides
// the weak no-op in sl_wfx_host.c.
void lt_wf200_netif_input(sl_wfx_received_ind_t *ind);

#ifdef __cplusplus
}
#endif
