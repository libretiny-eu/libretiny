/* Copyright (c) WGM160P-LibreTiny port 2026-06-10. */

#include "WiFiPrivate.h"

// One-time radio + IP stack bring-up. Phase 2: STA only; AP actions rejected.
bool WiFiClass::modePriv(WiFiMode mode, WiFiModeAction sta, WiFiModeAction ap) {
	if (ap == WLMODE_ENABLE) {
		LT_EM(WIFI, "softAP not implemented on WF200 yet (Phase 2 follow-on)");
		return false;
	}

	if (sta == WLMODE_ENABLE && !DATA->stackUp) {
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
