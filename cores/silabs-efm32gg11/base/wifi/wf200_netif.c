/* Copyright (c) WGM160P-LibreTiny port 2026-06-10.
 *
 * lwIP netif glue for the WF200 (STA interface, ethernetif-style).
 *
 * TASK / PRIORITY ARCHITECTURE (deliberate):
 *   WF200 bus task (configMAX_PRIORITIES-2 = 5)  >  lwIP tcpip thread
 *   (configMAX_PRIORITIES-3 = 4). The bus task must keep draining the radio's
 *   RX queue even under network load — if tcpip outranked it, a flood being
 *   processed in lwIP would starve the SDIO drain loop and back the chip up.
 *   Consequences for this file:
 *   - lt_wf200_netif_input() runs in BUS TASK context (via
 *     sl_wfx_host_post_event, with the FMAC host lock HELD). It copies the
 *     frame into a pbuf and hands off through netif->input = tcpip_input,
 *     which posts to the tcpip mbox with sys_mbox_trypost (NON-blocking) —
 *     the bus task never blocks on lwIP, it drops the frame instead.
 *   - The link/DHCP helpers are called from the lower-priority WiFi event
 *     task and defer their lwIP core calls to the tcpip thread via
 *     tcpip_callback().
 *
 * NOTE: no <libretiny.h> here (see sl_wfx_host.c) — this library builds in
 * the family queue, before the public defines libretiny.h needs exist.
 */

#include "wf200_netif.h"

#include "sl_wfx_host_conf.h"

#include <lwip/dhcp.h>
#include <lwip/etharp.h>
#include <lwip/netif.h>
#include <lwip/pbuf.h>
#include <lwip/tcpip.h>
#include <string.h>

#if LT_WFX_RX_TRACE
#include <stdio.h> // printf -> USART0 (family printf.c); LT_ macros unavailable in base/wifi
// Bug-2 instrumentation (opt-in via -DLT_WFX_RX_TRACE=1). Single writer (the
// bus task) updates these; the scan path reads them. Torn reads are cosmetic.
// Read by WiFiScan.cpp via the externs there.
volatile uint32_t lt_wfx_rx_sta;	// data frames demuxed to the STA netif
volatile uint32_t lt_wfx_rx_ap;		// data frames demuxed to the AP netif
volatile uint32_t lt_wfx_rx_drop;	// frames dropped by the length guard below
volatile uint32_t lt_wfx_rx_maxlen; // largest frame_length seen
#endif

// INVARIANT: lt_wf200_netif_create() must complete before the bus task processes
// its first frame.  Today this is naturally enforced: WIRQ is armed inside
// sl_wfx_init(), but RX traffic only flows after a successful join, and modePriv
// calls lt_wf200_netif_create() after sl_wfx_init() returns.  Anyone reordering
// the init sequence must re-verify this ordering guarantee.
static struct netif s_sta_netif;
// Non-static and named `ap_netif` so the reused GSDK Apache-2.0 dhcp_server.c
// can bind its UDP :67 socket to this exact netif (it takes &ap_netif via the
// compat/app_webpage.h shim). MUST be the same object that carries softAP
// traffic, which it is — ap_netif_init/low_level_output_ap below are wired onto
// it. Declared extern in compat/app_webpage.h.
struct netif ap_netif;
static bool s_tcpip_up;			  // tcpip_init must run at most once, even across retries
static volatile bool s_created;	  // volatile: compiler-reorder guard for the create-before-traffic invariant
static volatile bool s_ap_active; // gates RX demux: AP frames are dropped until the AP netif is up

struct netif *lt_wf200_netif_sta(void) {
	return s_created ? &s_sta_netif : NULL;
}

struct netif *lt_wf200_netif_ap(void) {
	return s_ap_active ? &ap_netif : NULL;
}

// --- TX: lwIP -> FMAC ---------------------------------------------------------
//
// Locking/confirmation shape (verified against the vendored driver):
// sl_wfx_send_ethernet_frame (sl_wfx.c:569) -> sl_wfx_send_request
// (sl_wfx.c:1699), which takes sl_wfx_host_lock() ITSELF and, for
// SL_WFX_SEND_FRAME_REQ_ID, explicitly SKIPS setup_waited_event
// (sl_wfx.c:1730) — data TX is fire-and-forget: no confirmation wait, no use
// of the single waited_event_id. So no extra mutex is needed here, and a TX
// from the tcpip thread cannot corrupt a WiFi command's confirmation
// in flight from another task. (Commands DO share the one-deep confirmation
// machinery among themselves — serializing sl_wfx_* command calls behind one
// mutex is the WiFi library's job, per the contract in sl_wfx_host_conf.h —
// Task 11.) The async SEND_FRAME_CNF only decrements the driver's
// used_buffers flow-control counter in sl_wfx_receive_frame (sl_wfx.c:1818);
// when the chip's buffers are full, send_request fails fast with
// SL_STATUS_WOULD_OVERFLOW instead of blocking.

// Shared low-level sender. The per-netif linkoutput wrappers below fix the
// interface so a frame queued on the AP netif is transmitted on the softAP
// interface and one queued on the STA netif on the station interface.
static err_t low_level_output_iface(struct netif *netif, struct pbuf *p, sl_wfx_interface_t iface) {
	(void)netif;
	sl_wfx_send_frame_req_t *req = NULL;
	uint32_t frame_len			 = SL_WFX_ROUND_UP(p->tot_len, 2);
	uint32_t alloc_len			 = frame_len + sizeof(sl_wfx_send_frame_req_t);

	// alloc/free are sequential, not concurrent: lwIP calls linkoutput one segment
	// at a time from the tcpip thread with the core lock held, so no burst
	// fragmentation of the TX buffer pool is possible.
	if (sl_wfx_host_allocate_buffer((void **)&req, SL_WFX_TX_FRAME_BUFFER, alloc_len) != SL_STATUS_OK)
		return ERR_MEM;
	memset(req, 0, sizeof(sl_wfx_send_frame_req_t));
	pbuf_copy_partial(p, req->body.packet_data, p->tot_len, 0);
	if (frame_len != p->tot_len)
		req->body.packet_data[p->tot_len] = 0; // don't transmit a heap byte as pad

	sl_status_t st = sl_wfx_send_ethernet_frame(req, frame_len, iface, 0);
	sl_wfx_host_free_buffer(req, SL_WFX_TX_FRAME_BUFFER);
	// ERR_IF on WOULD_OVERFLOW: tcp_output_segment treats a non-OK linkoutput as
	// TF_NAGLEMEMERR and schedules a timer-driven retry — not a connection abort.
	// Transient chip-buffer exhaustion therefore self-heals.  ERR_IF is used
	// (not ERR_MEM) to distinguish chip/bus errors from heap exhaustion.
	return (st == SL_STATUS_OK) ? ERR_OK : ERR_IF;
}

static err_t low_level_output_sta(struct netif *netif, struct pbuf *p) {
	return low_level_output_iface(netif, p, SL_WFX_STA_INTERFACE);
}

static err_t low_level_output_ap(struct netif *netif, struct pbuf *p) {
	return low_level_output_iface(netif, p, SL_WFX_SOFTAP_INTERFACE);
}

// --- RX: FMAC -> lwIP (bus task context; copy into a pbuf, hand to tcpip) ------
//
// Contract (sl_wfx_host.c): `ind` is valid only for the duration of this call
// — the driver frees its RX buffer as soon as post_event returns — and the
// FMAC host lock is HELD, so no driver API that takes the lock may be called
// from here. pbuf_alloc/pbuf_take copy synchronously (pools are safe across
// threads: SYS_LIGHTWEIGHT_PROT=1), and tcpip_input only tryposts a pointer.

void lt_wf200_netif_input(sl_wfx_received_ind_t *ind) {
	// Demux on the interface bits in the message header: STA-interface frames go
	// to the STA netif, softAP-interface frames to the AP netif.  The accessors
	// return NULL when the target interface isn't up (s_created / s_ap_active),
	// so frames for a down interface are dropped here.
	uint8_t iface	  = (ind->header.info & SL_WFX_MSG_INFO_INTERFACE_MASK) >> SL_WFX_MSG_INFO_INTERFACE_OFFSET;
	struct netif *nif = (iface == SL_WFX_SOFTAP_INTERFACE) ? lt_wf200_netif_ap() : lt_wf200_netif_sta();
	if (nif == NULL || !netif_is_link_up(nif))
		return;
	uint8_t *frame = ind->body.frame + ind->body.frame_padding;
	uint16_t len   = ind->body.frame_length;

#if LT_WFX_RX_TRACE
	// Bug-2 probe + guard: frame_length is taken straight from the indication.
	// If a malformed/secure-link-desynced frame carries a bogus length, the
	// pbuf_take(p, frame, len) below writes past the pbuf and corrupts a
	// neighbouring pbuf's ref field -> lwIP "pbuf_free: p->ref > 0" double-free
	// assert. Bound it to a max Ethernet II frame (1500 MTU + 14 hdr); record
	// and drop rather than corrupt. If Bug 2 is THIS path, enabling the trace
	// makes the crash stop and prints the offending length + interface.
	if (len == 0 || len > 1514) {
		lt_wfx_rx_drop++;
		printf("$WFXRX bogus len=%u iface=%u pad=%u -> dropped\r\n", (unsigned)len, (unsigned)iface,
			   (unsigned)ind->body.frame_padding);
		return;
	}
	if (iface == SL_WFX_SOFTAP_INTERFACE)
		lt_wfx_rx_ap++;
	else
		lt_wfx_rx_sta++;
	if (len > lt_wfx_rx_maxlen)
		lt_wfx_rx_maxlen = len;
#endif

	struct pbuf *p = pbuf_alloc(PBUF_RAW, len, PBUF_POOL);
	if (p == NULL)
		return; // drop; TCP retransmits
	pbuf_take(p, frame, len);
	if (nif->input(p, nif) != ERR_OK)
		pbuf_free(p); // tcpip mbox full -> trypost ERR_MEM; drop, don't block
}

// --- netif setup -----------------------------------------------------------------

static err_t sta_netif_init(struct netif *netif) {
	netif->name[0]	  = 'w';
	netif->name[1]	  = 'f';
	netif->mtu		  = 1500;
	netif->hwaddr_len = 6;
	memcpy(netif->hwaddr, wfx_context.mac_addr_0.octet, 6);
	netif->flags	  = NETIF_FLAG_BROADCAST | NETIF_FLAG_ETHARP | NETIF_FLAG_ETHERNET;
	netif->output	  = etharp_output;
	netif->linkoutput = low_level_output_sta;
	return ERR_OK;
}

static void tcpip_done(void *arg) {
	xSemaphoreGive((SemaphoreHandle_t)arg);
}

// ORDERING DEPENDENCY: sta_netif_init copies the STA MAC out of
// wfx_context.mac_addr_0, which sl_wfx_init() reads back from the chip — so
// this MUST be called after sl_wfx_init() succeeds (modePriv enforces the
// order, Task 11). Called once at bring-up, before any traffic exists; the
// bare netif_add/set_up here would otherwise need LOCK_TCPIP_CORE.
bool lt_wf200_netif_create(void) {
	if (s_created)
		return true;

	if (!s_tcpip_up) {
		SemaphoreHandle_t done = xSemaphoreCreateBinary();
		if (!done)
			return false;
		tcpip_init(tcpip_done, done);
		if (xSemaphoreTake(done, pdMS_TO_TICKS(5000)) != pdTRUE) {
			vSemaphoreDelete(done);
			return false;
		}
		vSemaphoreDelete(done);
		s_tcpip_up = true;
	}

	ip4_addr_t any;
	ip4_addr_set_zero(&any);
	if (netif_add(&s_sta_netif, &any, &any, &any, NULL, sta_netif_init, tcpip_input) == NULL)
		return false;
	netif_set_default(&s_sta_netif);
	netif_set_up(&s_sta_netif);
	s_created = true;
	return true;
}

// --- AP netif (static IP, no DHCP client) ------------------------------------------
//
// Unlike the STA netif (created once at bring-up before any traffic), the AP
// netif is added/removed at runtime — while the tcpip thread is already running
// and the STA netif may be carrying traffic. So every netif core call here is
// wrapped in LOCK_TCPIP_CORE()/UNLOCK_TCPIP_CORE() to serialize against the
// tcpip thread, rather than relying on the STA path's "before any traffic"
// guarantee. The MAC for ap_netif_init is staged in s_ap_mac because netif_add's
// init callback only receives the netif pointer; create is the only writer and
// is serialized by the core lock it holds across netif_add.

static uint8_t s_ap_mac[6];

static err_t ap_netif_init(struct netif *netif) {
	netif->name[0]	  = 'w';
	netif->name[1]	  = 'a';
	netif->mtu		  = 1500;
	netif->hwaddr_len = 6;
	memcpy(netif->hwaddr, s_ap_mac, 6);
	netif->flags	  = NETIF_FLAG_BROADCAST | NETIF_FLAG_ETHARP | NETIF_FLAG_ETHERNET;
	netif->output	  = etharp_output;
	netif->linkoutput = low_level_output_ap;
	return ERR_OK;
}

bool lt_wf200_netif_ap_create(uint32_t ip, uint32_t mask, uint32_t gw, const uint8_t mac[6]) {
	if (s_ap_active)
		return true;
	if (!s_tcpip_up) // STA create runs tcpip_init; AP cannot come up before it
		return false;

	memcpy(s_ap_mac, mac, 6);
	ip4_addr_t ip4, mask4, gw4;
	ip4.addr   = ip;
	mask4.addr = mask;
	gw4.addr   = gw;

	LOCK_TCPIP_CORE();
	struct netif *added = netif_add(&ap_netif, &ip4, &mask4, &gw4, NULL, ap_netif_init, tcpip_input);
	if (added != NULL) {
		netif_set_up(&ap_netif);
		netif_set_link_up(&ap_netif);
	}
	UNLOCK_TCPIP_CORE();
	if (added == NULL)
		return false;

	s_ap_active = true;
	return true;
}

// Teardown race (benign): the lock-free RX demux (lt_wf200_netif_input, bus
// task) may have already loaded nif=&ap_netif via lt_wf200_netif_ap() before
// we clear s_ap_active below.  Delivering that stale pointer is safe: ap_netif
// is statically allocated and never freed, and netif_remove only unlinks it from
// the lwIP list.  AP teardown is user-initiated and rare, so the one-frame window
// is acceptable; revisit if teardown ever becomes frequent.
void lt_wf200_netif_ap_destroy(void) {
	if (!s_ap_active)
		return;
	s_ap_active = false; // stop RX demux from selecting the AP netif first
	LOCK_TCPIP_CORE();
	netif_set_down(&ap_netif);
	netif_remove(&ap_netif);
	UNLOCK_TCPIP_CORE();
}

// --- Thread-safe link/DHCP helpers (callable from the WiFi event task) -------------
//
// tcpip_callback (lwIP 2.1.2, api/tcpip.c:309) calls memp_malloc for a
// TCPIP_MSG_API message BEFORE attempting the mbox post — if the pool of 8 is
// exhausted, memp_malloc returns NULL and tcpip_callback returns ERR_MEM
// immediately, without ever posting.  Retry after a brief delay so the tcpip
// thread drains the pool.  The callers run in the WiFi event task where a short
// sleep is acceptable.

static void do_link_up(void *arg) {
	(void)arg;
	netif_set_link_up(&s_sta_netif);
}

static void do_link_down(void *arg) {
	(void)arg;
	dhcp_stop(&s_sta_netif); // belt-and-suspenders with do_dhcp_stop; dhcp_stop is idempotent
	netif_set_link_down(&s_sta_netif);
}

static void do_dhcp_start(void *arg) {
	(void)arg;
	dhcp_start(&s_sta_netif);
}

static void do_dhcp_stop(void *arg) {
	(void)arg;
	dhcp_stop(&s_sta_netif);
}

static void tcpip_call_retry(tcpip_callback_fn fn) {
	for (int i = 0; i < 5; i++) {
		if (tcpip_callback(fn, NULL) == ERR_OK)
			return;
		vTaskDelay(pdMS_TO_TICKS(10)); // MEMP_TCPIP_MSG_API exhausted; let tcpip drain
	}
	// Final failure is silently dropped.  A dropped link-down means DHCP keeps
	// running on a dead link until the next connect/disconnect cycle re-issues it.
	// Accepted for Phase 2.
}

void lt_wf200_netif_set_link(bool up) {
	if (s_created)
		tcpip_call_retry(up ? do_link_up : do_link_down);
}

void lt_wf200_netif_dhcp_start(void) {
	if (s_created)
		tcpip_call_retry(do_dhcp_start);
}

void lt_wf200_netif_dhcp_stop(void) {
	if (s_created)
		tcpip_call_retry(do_dhcp_stop);
}
