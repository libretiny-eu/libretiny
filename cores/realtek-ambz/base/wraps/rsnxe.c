/*
 * Fix WPA2 connections to WPA2/WPA3 transition-mode APs on AmebaZ (RTL8710B).
 *
 * WPA3 added the RSN Extension element (RSNXE). An AP that advertises one in
 * its beacon must repeat it in message 3 of the 4-way handshake, and the
 * station must check the two match, so an attacker can't strip WPA3 features
 * from the beacon to force a downgrade.
 *
 * The 4-way handshake code in lib_wlan.a (ClientEAPOLKeyRecvd) performs this
 * check, but only half of it is wired up. It compares the RSNXE received in
 * message 3 against a copy it expects to have saved from the AP's beacon when
 * joining, yet nothing in this driver build ever saves that copy
 * (rtw_parse_rsnxe is never called). The saved value is therefore always 0.
 *
 * A WPA2/WPA3 transition-mode AP (wpa2-psk + wpa3-psk) advertises an RSNXE
 * with non-zero capabilities (e.g. 0x20 for SAE H2E). The check fails every
 * time: message 3 is silently dropped, message 4 is never sent, and the AP
 * deauthenticates the station (reason 2). WPA2-only APs have no RSNXE, which
 * is why they are unaffected.
 *
 * Fix: wrap rtw_joinbss_cmd() and, before the join, save the RSNXE
 * capabilities from the selected network's beacon/probe-response elements
 * where the check looks for them. Networks without an RSNXE save 0, as before.
 *
 * lib_wlan.a is a closed binary with no headers for its structures, so the
 * locations below are byte offsets taken from this build's disassembly of
 * rtw_joinbss_cmd and ClientEAPOLKeyRecvd. builder/family/realtek-ambz.py only
 * enables the wrap when lib_wlan.a matches that build's SHA-256, so a changed
 * blob disables the fix instead of corrupting driver state.
 *
 * To re-derive the offsets for a new lib_wlan.a:
 *  - ClientEAPOLKeyRecvd: in the message 3 key-data loop, the RSNXE compare is
 *    "cmp rX, #0xF4" followed by a byte load from the adapter structure
 *    (here "add.w r3, r5, #0x1460; ldrb r2, [r3, #30]" = 0x147E).
 *  - rtw_joinbss_cmd: the network's IE length and IEs are the arguments it
 *    passes to rtw_get_sec_ie and rtw_restruct_sec_ie (here +0x94 and +0x98).
 * Then update LIB_WLAN_SHA256 in realtek-ambz.py.
 */

#include <stdint.h>

#define RSNXE_EID			   0xF4
#define FIXED_IE_LEN		   12	  // timestamp + beacon interval + capability
#define MAX_IE_LEN			   768	  // IE area of WLAN_BSSID_EX copied by the driver
#define NETWORK_IE_LENGTH_OFF  0x94	  // wlan_network.network.IELength
#define NETWORK_IES_OFF		   0x98	  // wlan_network.network.IEs
#define ADAPTER_RSNXE_CAPS_OFF 0x147E // saved AP RSNXE capabilities, read by ClientEAPOLKeyRecvd

extern uint8_t __real_rtw_joinbss_cmd(void *padapter, void *pnetwork);

uint8_t __wrap_rtw_joinbss_cmd(void *padapter, void *pnetwork) {
	if (padapter && pnetwork) {
		uint32_t ie_len	   = *(const uint32_t *)((const uint8_t *)pnetwork + NETWORK_IE_LENGTH_OFF);
		const uint8_t *ies = (const uint8_t *)pnetwork + NETWORK_IES_OFF;
		uint8_t caps	   = 0;

		if (ie_len > MAX_IE_LEN)
			ie_len = MAX_IE_LEN;
		for (uint32_t i = FIXED_IE_LEN; i + 2 <= ie_len; i += 2 + ies[i + 1]) {
			uint8_t len = ies[i + 1];
			if (i + 2 + len > ie_len)
				break;
			if (ies[i] == RSNXE_EID && len >= 1) {
				caps = ies[i + 2];
				break;
			}
		}
		*((uint8_t *)padapter + ADAPTER_RSNXE_CAPS_OFF) = caps;
	}
	return __real_rtw_joinbss_cmd(padapter, pnetwork);
}
