/* Copyright (c) WGM160P-LibreTiny port 2026-06-10. */

#include "WiFiPrivate.h"

#include <string.h>

extern "C" {
#include "lt_dhcpserver.h"

#include <lwip/def.h>
}

// One-time radio + lwIP/STA-netif bring-up. Shared by the STA-enable and
// AP-enable paths so neither duplicates sl_wfx_init() (the confirmation
// machinery supports a single in-flight command; a second init would corrupt
// it). Idempotent via DATA->stackUp: the first caller (STA or AP) pays for it,
// the second sees stackUp and returns true immediately. On any failure it sets
// WL_NO_SHIELD + the fail LED and returns false, exactly as the original
// STA-enable block did. The AP netif is created later, per-enable, in modePriv.
static bool bringUpRadio(void *data) {
	if (DATA->stackUp)
		return true;

	// Light the status LED red the instant bring-up starts: if anything
	// below hangs (silent radio) the board stays red; a clean fault turns
	// it magenta. First call here also configures the LED pins (app thread).
	ltWifiStatusLed(LT_WIFI_LED_RADIO_INIT);
	// also attaches WIRQ + lt_wfx_host_setup(); fail cleanly before
	// sl_wfx_init() if the host primitives/event task couldn't be created
	if (!startWifiEventTask()) {
		DATA->staStatus = WL_NO_SHIELD;
		ltWifiStatusLed(LT_WIFI_LED_RADIO_FAIL);
		return false;
	}

	unsigned long t0 = millis();
	WIFI_CMD_TAKE(DATA);
	sl_status_t st = sl_wfx_init(&wfx_context);
	WIFI_CMD_GIVE(DATA);
	if (st != SL_STATUS_OK) {
		LT_EM(WIFI, "WF200 init failed: 0x%08lX (radio off / SDIO fault)", (unsigned long)st);
		DATA->staStatus = WL_NO_SHIELD;
		ltWifiStatusLed(LT_WIFI_LED_RADIO_FAIL);
		return false;
	}
	LT_IM(
		WIFI,
		"WF200 FW %u.%u.%u up in %lu ms",
		wfx_context.firmware_major,
		wfx_context.firmware_minor,
		wfx_context.firmware_build,
		millis() - t0
	);

	if (!lt_wf200_netif_create()) {
		LT_EM(WIFI, "lwIP/netif bring-up failed");
		DATA->staStatus = WL_NO_SHIELD;
		ltWifiStatusLed(LT_WIFI_LED_RADIO_FAIL);
		return false;
	}
	// The netif only exists from this point on — install the status
	// callback now (couldn't be done in startWifiEventTask()). Link is
	// still down and no traffic flows yet, so mutating the netif from
	// this thread can't race the tcpip thread (lwIP threading rules).
	ltWifiAttachNetifCallback();
	DATA->stackUp	= true;
	DATA->staStatus = WL_DISCONNECTED;
	// Radio + IP stack alive, not yet joined: blue.
	ltWifiStatusLed(LT_WIFI_LED_IDLE);
	return true;
}

bool WiFiClass::modePriv(WiFiMode mode, WiFiModeAction sta, WiFiModeAction ap) {
	// Radio bring-up: either STA-enable or AP-enable triggers it (whichever
	// comes first). Shared helper so sl_wfx_init() runs exactly once.
	if ((sta == WLMODE_ENABLE || ap == WLMODE_ENABLE) && !DATA->stackUp) {
		if (!bringUpRadio(data))
			return false;
	}

	if (ap == WLMODE_ENABLE && !DATA->apActive) {
		// Default AP address plan 192.168.4.1/24 unless the app set one.
		// PP_HTONL(LWIP_MAKEU32(...)) builds a network-order ip4_addr_t.addr
		// word regardless of host endianness; Task 2's ap_create assigns it
		// straight into ip4_addr_t.addr (also network order), so the bytes
		// land as 192.168.4.1 on the wire.
		uint32_t ip	  = DATA->apLocalIP ? DATA->apLocalIP : PP_HTONL(LWIP_MAKEU32(192, 168, 4, 1));
		uint32_t mask = DATA->apSubnet ? DATA->apSubnet : PP_HTONL(LWIP_MAKEU32(255, 255, 255, 0));

		// AP MAC: derive from the STA MAC that sl_wfx_init() read back into
		// wfx_context.mac_addr_0 (this driver has no sl_wfx_get_mac_address).
		// Set the locally-administered bit so STA and AP MACs differ on-air.
		// The SAME octets are pushed to the SOFTAP interface AND used as the
		// AP netif hwaddr (ap_create stages them into netif->hwaddr) — they
		// MUST match or AP-side ARP/RX breaks.
		sl_wfx_mac_address_t apMac = wfx_context.mac_addr_0;
		apMac.octet[0] |= 0x02;

		WIFI_CMD_TAKE(DATA);
		sl_wfx_set_mac_address(&apMac, SL_WFX_SOFTAP_INTERFACE);
		WIFI_CMD_GIVE(DATA);

		if (!lt_wf200_netif_ap_create(ip, mask, ip /* gw = self */, apMac.octet)) {
			LT_EM(WIFI, "AP netif create failed");
			return false; // STA state untouched
		}
		lt_dhcpserver_start(lt_wf200_netif_ap(), ip);

		// Channel coexistence: a single radio can't run STA and AP on
		// different channels — ride the STA channel when joined, else use the
		// app's requested channel (or 1).
		uint16_t ch				   = (DATA->staStatus == WL_CONNECTED && DATA->staChannel)
										 ? DATA->staChannel
										 : (DATA->apChannel ? (uint16_t)DATA->apChannel : 1);
		sl_wfx_security_mode_t sec = DATA->apPassword ? WFM_SECURITY_MODE_WPA2_PSK : WFM_SECURITY_MODE_OPEN;

		WIFI_CMD_TAKE(DATA);
		sl_status_t st = sl_wfx_start_ap_command(
			ch,
			(uint8_t *)DATA->apSsid,
			(uint32_t)strlen(DATA->apSsid),
			0, // hidden_ssid
			0, // client_isolation
			sec,
			WFM_MGMT_FRAME_PROTECTION_OPTIONAL,
			(const uint8_t *)(DATA->apPassword ? DATA->apPassword : ""),
			(uint16_t)(DATA->apPassword ? strlen(DATA->apPassword) : 0),
			NULL,
			0, // beacon IE
			NULL,
			0 // probe-response IE
		);
		WIFI_CMD_GIVE(DATA);
		if (st != SL_STATUS_OK) {
			lt_dhcpserver_stop();
			lt_wf200_netif_ap_destroy();
			LT_EM(WIFI, "start_ap failed: 0x%08lX", (unsigned long)st);
			return false;
		}
		// Optimistic; START_AP_IND in WiFiEvents confirms authoritatively.
		DATA->apActive = true;
	}

	if (ap == WLMODE_DISABLE && DATA->apActive) {
		WIFI_CMD_TAKE(DATA);
		sl_wfx_stop_ap_command();
		WIFI_CMD_GIVE(DATA);
		lt_dhcpserver_stop();
		lt_wf200_netif_ap_destroy();
		DATA->apActive		= false;
		DATA->apClientCount = 0;
		free(DATA->apSsid);
		DATA->apSsid = NULL;
		free(DATA->apPassword);
		DATA->apPassword = NULL;
	}

	if (sta == WLMODE_DISABLE && DATA->stackUp && (DATA->mode & WIFI_MODE_STA)) {
		WIFI_CMD_TAKE(DATA);
		sl_wfx_send_disconnect_command();
		WIFI_CMD_GIVE(DATA);
		lt_wf200_netif_set_link(false);
		// no WL_STOPPED in WiFiType.h — STA-disabled reads as Disconnected
		DATA->staStatus = WL_DISCONNECTED;
		// Radio stays up; back to the idle (not-joined) color.
		ltWifiStatusLed(LT_WIFI_LED_IDLE);
	}

	DATA->mode = mode;
	return true;
}

WiFiMode WiFiClass::getMode() {
	return DATA->mode;
}

WiFiStatus WiFiClass::status() {
	return DATA->staStatus;
}
