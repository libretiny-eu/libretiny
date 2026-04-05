/* Copyright (c) Etienne Le Cousin 2024-03-10. */

#include "WiFiPrivate.h"
#include <lwip/dhcp.h>  // dhcp_supplied_address, dhcp_release_and_stop
#include <lwip/tcpip.h> // tcpip_callback

static void wifiEventIpReceived(struct netif *nif);

// ---------------------------------------------------------------------------
// Async callback: scheduled via tcpip_callback() from within wifiEventIpReceived
// when DHCP completes despite our best-effort stop in wifiEventStaConnected().
//
// Runs in the LwIP tcpip_thread AFTER dhcp_recv() has fully returned, so it
// is safe to call dhcp_release_and_stop() directly here (no use-after-free).
// We must NOT use netifapi_dhcp_release_and_stop() here — that function posts
// a message to the tcpip_thread mailbox and waits for completion, which would
// deadlock since we ARE the tcpip_thread.
// ---------------------------------------------------------------------------
static void wifiApplyStaticIpCallback(void *arg) {
	struct netif *nif = (struct netif *)arg;
	if (!pWiFi || !nif)
		return;
	WiFiNetworkInfo &info = pDATA->sta;
	if (!info.localIP)
		return;

	// Stop DHCP — safe here, we're outside dhcp_recv()
	dhcp_release_and_stop(nif);

	// Apply the configured static address
	ip4_addr_t ipaddr, netmask, gw;
	ipaddr.addr  = info.localIP;
	netmask.addr = info.subnet;
	gw.addr		 = info.gateway;
	netif_set_addr(nif, &ipaddr, &netmask, &gw);
	// netif_set_addr triggers the netif status callback again, but since DHCP
	// is now stopped dhcp_supplied_address() returns false, so g_get_ip_cb is
	// NOT re-entered — no infinite recursion.

	// Restore configured DNS servers — DHCP may have overwritten them via option 6
	// before we could stop it (reconnect/roam fallback path only).
	ip4_addr_t d1, d2;
	d1.addr = info.dns1;
	d2.addr = info.dns2;
	if (d1.addr)
		dns_setserver(0, &d1);
	if (d2.addr)
		dns_setserver(1, &d2);

	// Emit GOT_IP with the correct static address
	wifiEventIpReceived(nif);
}

// ---------------------------------------------------------------------------

void wifiEventSendArduino(EventId event) {
	EventInfo eventInfo;
	memset(&eventInfo, 0, sizeof(EventInfo));
	if (!pWiFi)
		return; // failsafe
	pWiFi->postEvent(event, eventInfo);
}

static void wifiEventStaStartup(void *arg) {
	wifiEventSendArduino(ARDUINO_EVENT_WIFI_STA_START);
}

static void wifiEventStaConnected(void *arg) {
	EventInfo eventInfo;
	memset(&eventInfo, 0, sizeof(EventInfo));
	if (!pWiFi)
		return; // failsafe

	String ssid							  = pWiFi->SSID();
	eventInfo.wifi_sta_connected.ssid_len = ssid.length();
	eventInfo.wifi_sta_connected.channel  = pWiFi->channel();
	eventInfo.wifi_sta_connected.authmode = pWiFi->getEncryption();
	memcpy(eventInfo.wifi_sta_connected.ssid, ssid.c_str(), eventInfo.wifi_sta_connected.ssid_len + 1);
	memcpy(eventInfo.wifi_sta_connected.bssid, pWiFi->BSSID(), 6);

	pWiFi->postEvent(ARDUINO_EVENT_WIFI_STA_CONNECTED, eventInfo);

	// If static IP is configured, apply it now and stop DHCP before the SDK
	// starts it. wifi_sta_connect() always starts DHCP internally; we have no
	// way to pass dhcp_mode to it (unlike BK72XX's bk_wlan_start_sta_adv_fix).
	//
	// IMPORTANT: stop DHCP *before* calling netif_set_addr. If DHCP is already
	// BOUND when this event fires, netif_set_addr would trigger the netif status
	// callback with dhcp_supplied_address==true, which would schedule an
	// unnecessary wifiApplyStaticIpCallback. Stopping DHCP first ensures the
	// status callback sees dhcp_supplied_address==false and stays silent.
	WiFiNetworkInfo &info = pDATA->sta;
	if (info.localIP) {
		struct netif *ifs = netdev_get_netif(NETIF_IDX_STA);
		ip4_addr_t ipaddr, netmask, gw;
		ipaddr.addr	 = info.localIP;
		netmask.addr = info.subnet;
		gw.addr		 = info.gateway;
		netifapi_dhcp_release_and_stop(ifs); // Stop first — safe (wifi_manager task)
		netif_set_addr(ifs, &ipaddr, &netmask, &gw);
		// Emit GOT_IP manually — DHCP is stopped so the netdev callback won't fire
		wifiEventIpReceived(ifs);
	}
}

static void wifiEventStaDisconnected(void *arg) {
	EventInfo eventInfo;
	memset(&eventInfo, 0, sizeof(EventInfo));
	if (!pWiFi)
		return; // failsafe

	eventInfo.wifi_sta_disconnected.ssid_len = 0;
	eventInfo.wifi_sta_disconnected.reason	 = WIFI_REASON_ASSOC_LEAVE;
	pWiFi->postEvent(ARDUINO_EVENT_WIFI_STA_DISCONNECTED, eventInfo);
}

static void wifiEventStaConnectFailed(void *arg) {
	wifi_sta_connect_failed_reason_t reason = *(wifi_sta_connect_failed_reason_t *)arg;
	EventInfo eventInfo;
	memset(&eventInfo, 0, sizeof(EventInfo));
	if (!pWiFi)
		return; // failsafe

	eventInfo.wifi_sta_disconnected.ssid_len = 0;
	eventInfo.wifi_sta_disconnected.reason	 = WIFI_REASON_UNSPECIFIED;
	switch (reason) {
		case WIFI_STA_CONN_WRONG_PWD:
			eventInfo.wifi_sta_disconnected.reason = WIFI_REASON_AUTH_FAIL;
			break;
		case WIFI_STA_CONN_TARGET_AP_NOT_FOUND:
			eventInfo.wifi_sta_disconnected.reason = WIFI_REASON_NO_AP_FOUND;
			break;
		case WIFI_STA_CONN_TIMEOUT:
			eventInfo.wifi_sta_disconnected.reason = WIFI_REASON_BEACON_TIMEOUT;
			break;
		case WIFI_STA_CONN_REFUSED:
			eventInfo.wifi_sta_disconnected.reason = WIFI_REASON_CONNECTION_FAIL;
			break;
	}
	pWiFi->postEvent(ARDUINO_EVENT_WIFI_STA_DISCONNECTED, eventInfo);
}

static void wifiEventSoftAPStartup(void *arg) {
	netdev_set_state(NETIF_IDX_AP, NETDEV_UP);
	wifiEventSendArduino(ARDUINO_EVENT_WIFI_AP_START);
}

static void wifiEventSoftAPAssociated(void *arg) {
	const uint8_t *mac_addr = (const uint8_t *)arg;
	EventInfo eventInfo;
	memset(&eventInfo, 0, sizeof(EventInfo));
	if (!pWiFi)
		return; // failsafe

	memcpy(eventInfo.wifi_ap_staconnected.mac, mac_addr, 6);
	pWiFi->postEvent(ARDUINO_EVENT_WIFI_AP_STACONNECTED, eventInfo);
}

static void wifiEventSoftAPDisassociated(void *arg) {
	const uint8_t *mac_addr = (const uint8_t *)arg;
	EventInfo eventInfo;
	memset(&eventInfo, 0, sizeof(EventInfo));
	if (!pWiFi)
		return; // failsafe

	memcpy(eventInfo.wifi_ap_staconnected.mac, mac_addr, 6);
	pWiFi->postEvent(ARDUINO_EVENT_WIFI_AP_STADISCONNECTED, eventInfo);
}

static void wifiEventIpReceived(struct netif *nif) {
	EventInfo eventInfo;
	memset(&eventInfo, 0, sizeof(EventInfo));
	if (!pWiFi || !nif)
		return; // failsafe

	// Fallback: DHCP completed and assigned an address despite our attempt to
	// stop it in wifiEventStaConnected(). This can happen when:
	//   (a) WIFI_MGR_EVENT_STA_CONNECTED fires before the SDK starts DHCP, so
	//       netifapi_dhcp_release_and_stop() was a NOP and DHCP ran afterward; or
	//   (b) a reconnect/roam caused the SDK to restart DHCP without triggering
	//       WIFI_MGR_EVENT_STA_CONNECTED again (e.g. SDK-internal auto-reconnect).
	//
	// In this case we are being called from sta_netif_status_changed_cb(), which
	// is the LwIP netif status callback invoked from within dhcp_recv() in the
	// tcpip_thread.  Two important constraints apply:
	//   - Cannot call dhcp_release_and_stop() directly: dhcp_recv() hasn't
	//     returned yet, freeing the dhcp struct now would be a use-after-free.
	//   - Cannot call netifapi_dhcp_release_and_stop(): it posts a message to
	//     the tcpip_thread mailbox and waits for a reply — deadlock because we
	//     ARE the tcpip_thread.
	//
	// Solution: schedule wifiApplyStaticIpCallback via tcpip_callback(). It will
	// run in the tcpip_thread after the current message (dhcp_recv) has returned,
	// so neither constraint applies there.
	WiFiNetworkInfo &info = pDATA->sta;
	if (info.localIP && dhcp_supplied_address(nif)) {
		tcpip_callback(wifiApplyStaticIpCallback, nif);
		// Do not emit GOT_IP here with the wrong DHCP address.
		// wifiApplyStaticIpCallback will emit it with the static address.
		return;
	}

	eventInfo.got_ip.if_index			  = 0;
	eventInfo.got_ip.ip_changed			  = true;
	eventInfo.got_ip.ip_info.ip.addr	  = nif->ip_addr.addr;
	eventInfo.got_ip.ip_info.gw.addr	  = nif->gw.addr;
	eventInfo.got_ip.ip_info.netmask.addr = nif->netmask.addr;
	pWiFi->postEvent(ARDUINO_EVENT_WIFI_STA_GOT_IP, eventInfo);
}

void registerWifiHandlers() {
	wifi_manager_reg_event_callback(WIFI_MGR_EVENT_STA_STARTUP, &wifiEventStaStartup);
	wifi_manager_reg_event_callback(WIFI_MGR_EVENT_STA_CONNECTED, &wifiEventStaConnected);
	wifi_manager_reg_event_callback(WIFI_MGR_EVENT_STA_DISCONNECTED, &wifiEventStaDisconnected);
	wifi_manager_reg_event_callback(WIFI_MGR_EVENT_STA_CONNECT_FAILED, &wifiEventStaConnectFailed);

	netdev_get_ip_cb_set(&wifiEventIpReceived);

	wifi_manager_reg_event_callback(WIFI_MGR_EVENT_SOFTAP_STARTUP, &wifiEventSoftAPStartup);
	wifi_manager_reg_event_callback(WIFI_MGR_EVENT_SOFTAP_ASSOCIATED, &wifiEventSoftAPAssociated);
	wifi_manager_reg_event_callback(WIFI_MGR_EVENT_SOFTAP_DISASSOCIATED, &wifiEventSoftAPDisassociated);
}
