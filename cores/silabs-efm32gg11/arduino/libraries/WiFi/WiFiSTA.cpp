/* Copyright (c) WGM160P-LibreTiny port 2026-06-10. */

#include "WiFiPrivate.h"

extern "C" {
#include <lwip/api.h> // netconn_gethostbyname
}

WiFiStatus WiFiClass::begin(
	const char *ssid,
	const char *passphrase,
	int32_t channel,
	const uint8_t *bssid,
	bool connect
) {
	(void)connect;
	if (!enableSTA(true))
		return WL_CONNECT_FAILED;
	if (!validate(ssid, passphrase))
		return WL_CONNECT_FAILED;

	// A fresh begin() is a new attempt: disarm auto-reconnect until this one
	// actually reaches GOT_IP. Keeps a wrong-credential begin() from looping in
	// the supervisor — a failed initial join stays the caller's to handle.
	ltWifiReconnectDisarm();

	// Re-begin while associated must deassociate first (SSID may have changed)
	// — standard family behavior; avoids a duplicate-join WRONG_STATE rejection
	// leaving WL_CONNECT_FAILED with a live link.
	if (isConnected())
		disconnect(false);

	WiFiNetworkInfo &info = DATA->sta;
	// `info.ssid == ssid` happens when restoreSTAConfig() passes our own
	// storage back in — don't free/realloc what we're copying from.
	if (info.ssid != ssid)
		strcpy(info.ssid = (char *)realloc(info.ssid, strlen(ssid) + 1), ssid);
	if (passphrase && info.password != passphrase)
		strcpy(info.password = (char *)realloc(info.password, strlen(passphrase) + 1), passphrase);
	if (!passphrase) {
		free(info.password);
		info.password = NULL;
	}
	info.channel = channel;

	if (reconnect(bssid))
		return WL_CONNECTED;
	return WL_CONNECT_FAILED;
}

bool WiFiClass::config(IPAddress localIP, IPAddress gateway, IPAddress subnet, IPAddress dns1, IPAddress dns2) {
	if (!enableSTA(true))
		return false;
	WiFiNetworkInfo &info = DATA->sta;

	info.dns1 = dns1;
	info.dns2 = dns2;
	// LWIP_TCPIP_CORE_LOCKING=1: direct lwIP core mutation from the app
	// thread is legal under the core lock.
	LOCK_TCPIP_CORE();
	if (info.dns1) {
		ip_addr_t d1 = IPADDR4_INIT(info.dns1);
		dns_setserver(0, &d1);
	}
	if (info.dns2) {
		ip_addr_t d2 = IPADDR4_INIT(info.dns2);
		dns_setserver(1, &d2);
	}
	UNLOCK_TCPIP_CORE();

	if (!localIP[0]) {
		// config(0.0.0.0, ...) reverts to DHCP (ESP32 API convention)
		info.localIP = 0;
		lt_wf200_netif_dhcp_start();
		return true;
	}
	info.localIP = localIP;
	info.subnet	 = subnet;
	info.gateway = gateway;
	lt_wf200_netif_dhcp_stop();
	struct netif *nif = lt_wf200_netif_sta();
	if (nif) {
		ip4_addr_t ip	= {.addr = info.localIP};
		ip4_addr_t mask = {.addr = info.subnet};
		ip4_addr_t gw	= {.addr = info.gateway};
		LOCK_TCPIP_CORE();
		netif_set_addr(nif, &ip, &mask, &gw);
		UNLOCK_TCPIP_CORE();
	}
	// if the netif doesn't exist yet, reconnect() applies info.* before joining
	return true;
}

bool WiFiClass::reconnect(const uint8_t *bssid) {
	WiFiNetworkInfo &info = DATA->sta;
	if (!info.ssid)
		return false;

	// Static IP before join, if configured (no-op when config() already
	// applied it — netif_set_addr with an unchanged address does nothing).
	if (info.localIP) {
		struct netif *nif = lt_wf200_netif_sta();
		ip4_addr_t ip	  = {.addr = info.localIP};
		ip4_addr_t mask	  = {.addr = info.subnet};
		ip4_addr_t gw	  = {.addr = info.gateway};
		LOCK_TCPIP_CORE();
		netif_set_addr(nif, &ip, &mask, &gw);
		if (info.dns1) {
			ip_addr_t d1 = IPADDR4_INIT(info.dns1);
			dns_setserver(0, &d1);
		}
		UNLOCK_TCPIP_CORE();
	}

	sl_wfx_security_mode_t security = info.password ? WFM_SECURITY_MODE_WPA2_PSK : WFM_SECURITY_MODE_OPEN;
	DATA->staStatus					= WL_IDLE_STATUS;

	LT_IM(WIFI, "Joining %s", info.ssid);
	WIFI_CMD_TAKE(DATA);
	sl_status_t st = sl_wfx_send_join_command(
		(const uint8_t *)info.ssid,
		strlen(info.ssid),
		bssid ? (const sl_wfx_mac_address_t *)bssid : NULL,
		0 /* channel: 0 = any */,
		security,
		1 /* prevent_roaming */,
		WFM_MGMT_FRAME_PROTECTION_OPTIONAL,
		(const uint8_t *)(info.password ? info.password : ""),
		info.password ? strlen(info.password) : 0,
		NULL,
		0
	);
	WIFI_CMD_GIVE(DATA);
	if (st != SL_STATUS_OK) {
		LT_EM(WIFI, "join command failed: 0x%08lX", (unsigned long)st);
		DATA->staStatus = WL_CONNECT_FAILED;
		ltWifiStatusLed(LT_WIFI_LED_JOIN_FAIL);
		return false;
	}
	// Join command accepted; awaiting CONNECT ind + IP: yellow.
	ltWifiStatusLed(LT_WIFI_LED_JOINING);

	// Bounded wait for the full ladder: connect ind -> (DHCP) -> got IP.
	// 20 s covers WPA2 4-way + a slow DHCP server.
	for (uint32_t waited = 0; waited < 20000; waited += 100) {
		WiFiStatus s = DATA->staStatus;
		if (s == WL_CONNECTED)
			return true;
		if (s == WL_CONNECT_FAILED || s == WL_CONNECTION_LOST ||
			s == WL_DISCONNECTED) // only writable here by a concurrent intentional disconnect() — spinning further is
								  // pointless
			return false;
		// Static IP: link-up alone is success.
		if (info.localIP && s == WL_IDLE_STATUS && netif_is_link_up(lt_wf200_netif_sta())) {
			DATA->staStatus = WL_CONNECTED;
			// Static IP: netifStatusCb won't fire (no DHCP) — arm auto-reconnect
			// here, the equivalent of the GOT_IP arm on the DHCP path.
			ltWifiReconnectArm();
			// Static IP: netifStatusCb won't fire (no DHCP), so green here.
			ltWifiStatusLed(LT_WIFI_LED_JOINED);
			return true;
		}
		vTaskDelay(pdMS_TO_TICKS(100));
	}
	LT_EM(WIFI, "join timed out");
	// Tear down before marking failure: leaves no orphaned L2 association/DHCP
	// client behind, so a later begin() starts clean (realtek does the same on
	// DHCP failure).  Do NOT call WiFiClass::disconnect() — it would overwrite
	// the status we are about to set.
	lt_wf200_netif_set_link(false);
	lt_wf200_netif_dhcp_stop();
	WIFI_CMD_TAKE(DATA);
	sl_wfx_send_disconnect_command();
	WIFI_CMD_GIVE(DATA);
	DATA->staStatus = WL_CONNECT_FAILED;
	ltWifiStatusLed(LT_WIFI_LED_JOIN_FAIL);
	return false;
}

bool WiFiClass::disconnect(bool wifiOff) {
	// Intentional teardown: disarm first so a concurrent supervisor tick can't
	// rejoin between the status write below and the command.
	ltWifiReconnectDisarm();
	// Mark the disconnect as intentional BEFORE sending the command: the
	// DISCONNECT indication handler (WiFiEvents.cpp) only flags
	// WL_CONNECTION_LOST when status is CONNECTED/IDLE, so setting
	// WL_DISCONNECTED first makes the indication read as expected, not lost.
	DATA->staStatus = WL_DISCONNECTED;
	WIFI_CMD_TAKE(DATA);
	sl_wfx_send_disconnect_command();
	WIFI_CMD_GIVE(DATA);
	lt_wf200_netif_set_link(false);
	if (wifiOff)
		enableSTA(false);
	return true;
}

// The WF200 FMAC has no autonomous rejoin, so the backend supervises it itself:
// a link that reached GOT_IP and then dropped is auto-rejoined from the WiFi
// event task (ESP32 parity; default ON). See ltWifiReconnect* in WiFiEvents.cpp.
bool WiFiClass::setAutoReconnect(bool autoReconnect) {
	ltWifiSetAutoReconnect(autoReconnect);
	return true;
}

bool WiFiClass::getAutoReconnect() {
	return ltWifiGetAutoReconnect();
}

// --- getters (netif/driver state) ---------------------------------------------

IPAddress WiFiClass::localIP() {
	struct netif *nif = lt_wf200_netif_sta();
	return nif ? IPAddress(netif_ip4_addr(nif)->addr) : IPAddress();
}

IPAddress WiFiClass::subnetMask() {
	struct netif *nif = lt_wf200_netif_sta();
	return nif ? IPAddress(netif_ip4_netmask(nif)->addr) : IPAddress();
}

IPAddress WiFiClass::gatewayIP() {
	struct netif *nif = lt_wf200_netif_sta();
	return nif ? IPAddress(netif_ip4_gw(nif)->addr) : IPAddress();
}

IPAddress WiFiClass::dnsIP(uint8_t dns_no) {
	const ip_addr_t *d = dns_getserver(dns_no);
	return IPAddress(ip_addr_get_ip4_u32(d));
}

IPAddress WiFiClass::broadcastIP() {
	return calculateBroadcast(localIP(), subnetMask());
}

uint8_t *WiFiClass::macAddress(uint8_t *mac) {
	// zeros before sl_wfx_init() populates wfx_context from the chip
	memcpy(mac, wfx_context.mac_addr_0.octet, 6);
	return mac;
}

const String WiFiClass::SSID() {
	return DATA->sta.ssid ? String(DATA->sta.ssid) : String();
}

const String WiFiClass::psk() {
	return DATA->sta.password ? String(DATA->sta.password) : String();
}

uint8_t *WiFiClass::BSSID() {
	// filled by the CONNECT indication (WiFiEvents.cpp); zeros before that
	return DATA->staBssid;
}

int32_t WiFiClass::channel() {
	return DATA->staChannel;
}

int8_t WiFiClass::RSSI() {
	if (!isConnected())
		return 0;
	uint32_t rcpi = 0;
	WIFI_CMD_TAKE(DATA);
	sl_status_t st = sl_wfx_get_signal_strength(&rcpi);
	WIFI_CMD_GIVE(DATA);
	if (st != SL_STATUS_OK)
		return 0;
	// RCPI (IEEE 802.11k): 0 = -110 dBm, 0.5 dB steps -> dBm = (RCPI - 220) / 2
	return (int8_t)(((int32_t)rcpi - 220) / 2);
}

WiFiAuthMode WiFiClass::getEncryption() {
	if (!isConnected())
		return WIFI_AUTH_OPEN;
	// We join with WFM_SECURITY_MODE_WPA2_PSK whenever a passphrase is set
	// (WiFiScan results carry the AP's real mode; this reports the link's).
	return DATA->sta.password ? WIFI_AUTH_WPA2_PSK : WIFI_AUTH_OPEN;
}

const char *WiFiClass::getHostname() {
	struct netif *nif = lt_wf200_netif_sta();
	if (!nif || !netif_get_hostname(nif))
		return "";
	return netif_get_hostname(nif);
}

bool WiFiClass::setHostname(const char *hostname) {
	struct netif *nif = lt_wf200_netif_sta();
	if (!nif)
		return false;
	// lwIP stores the pointer, not a copy — keep our own stable storage.
	static char s_hostname[33];
	strncpy(s_hostname, hostname, sizeof(s_hostname) - 1);
	s_hostname[sizeof(s_hostname) - 1] = '\0';
	LOCK_TCPIP_CORE();
	netif_set_hostname(nif, s_hostname);
	UNLOCK_TCPIP_CORE();
	return true;
}

IPAddress WiFiClass::hostByName(const char *hostname) {
	// netconn_gethostbyname = lwIP's blocking resolver (same pattern as the
	// beken/realtek families); DNS server comes from DHCP or config().
	ip_addr_t ip;
	err_t ret = netconn_gethostbyname(hostname, &ip);
	if (ret == ERR_OK)
		return IPAddress(ip_addr_get_ip4_u32(&ip));
	return IPAddress();
}
