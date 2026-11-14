#!/usr/bin/env python3
"""Host regression test for the Ameba-1 plain-join policy.

Candidate012 mirrors the known-working vendor firmware on ``LT_AMB1``:

  * plain ``wifi_connect(ssid, security, password, ssid_len, pass_len, 0,
    NULL)`` is the only join call;
  * no ``wext_set_ssid("-")`` pre-clear, no ``wifi_connect_bssid()`` and no
    ``wifi_set_pscan_chan()``;
  * a BSSID argument from ``WiFi.begin()`` is accepted but ignored;
  * non-secret diagnostics log SSID length, password length, security value,
    channel and whether the BSSID argument was ignored.

The test compiles the real WiFiSTA.cpp against host mocks (no toolchain, no
radio, no hardware) and asserts the AMB1 call sequence, key_id, diagnostics and
that the non-AMB1 build still performs exactly the original path.

Run:
    python3 cores/realtek-amb1/tests/test_connect_diag.py
"""

import pathlib
import subprocess
import tempfile
import unittest

HERE = pathlib.Path(__file__).resolve()
CORES = HERE.parents[2]
WIFI_DIR = CORES / "realtek-amb" / "arduino" / "libraries" / "WiFi"
WIFI_STA = WIFI_DIR / "WiFiSTA.cpp"
HELPER = WIFI_DIR / "WiFiConnectDiag.h"

MOCK_WIFI_PRIVATE = r"""
#pragma once
/* Host-test mock of the real WiFiPrivate.h: only the surface WiFiSTA.cpp
 * uses. Never part of a firmware build. System headers are included by
 * test.cpp before this file is seen. */

#include <stdint.h>
#include <stddef.h>
#include <string.h>
#include <stdlib.h>

typedef int rtw_security_t;
typedef int wifi_mode_t;
typedef int WiFiAuthMode;
typedef int WiFiStatus;

#define WL_CONNECTED 3
#define WL_CONNECT_FAILED 4

#define WIFI_MODE_NULL 0
#define WIFI_MODE_STA 1
#define WIFI_MODE_AP 2
#define WIFI_MODE_APSTA 3

#define WIFI_AUTH_OPEN 0
#define WIFI_AUTH_WPA2_PSK 3

#define ETH_ALEN 6
#define WLAN0_NAME "wlan0"
#define NETNAME_STA WLAN0_NAME
#define PSCAN_ENABLE 0x01
#define PSCAN_FAST_SURVEY 0x02
#define DHCP_START 0
#define DHCP_STOP 1
#define DHCP_ADDRESS_ASSIGNED 1
#define WIFI_EVENT_CONNECT 0
#define ARDUINO_EVENT_WIFI_STA_GOT_IP 1

#define RTW_SUCCESS 0
#define RTW_SECURITY_OPEN 0x00000000
#define RTW_SECURITY_WPA_AES_PSK 0x00200004
#define RTW_SECURITY_WPA2_AES_PSK 0x00400004
#define RTW_SECURITY_WPA2_AES_CMAC 0x00400010
#define RTW_SECURITY_WPA3_AES_PSK 0x00800004
#define RTW_SECURITY_WPA_WPA2_MIXED 0x00600000

#define LT_HEAP_I()
#define LT_IM(module, ...) mock_log_i(__VA_ARGS__)
#define LT_EM(module, ...) mock_log_e(__VA_ARGS__)
#define DIAG_PRINTF_DISABLE()
#define DIAG_PRINTF_ENABLE()

class String {
  public:
	String(const char *s = "") : value(s) {}
	const char *c_str() const { return value; }

  private:
	const char *value;
};

class IPAddress {
  public:
	IPAddress(uint32_t a = 0) : addr(a) {}
	operator uint32_t() const { return addr; }
	uint8_t operator[](int i) const { return ((const uint8_t *)&addr)[i]; }
	uint32_t addr;
};

typedef struct {
	uint8_t addr[6];
} WiFiMacAddr;

typedef struct {
	char *ssid;
	char *password;
	uint8_t *bssid;
	bool ssidHidden;
	int channel;
	int auth;
	uint32_t localIP;
	uint32_t subnet;
	uint32_t gateway;
	uint32_t dns1;
	uint32_t dns2;
} WiFiNetworkInfo;

typedef struct {
	char *ssid;
	WiFiAuthMode auth;
	int32_t rssi;
	WiFiMacAddr bssid;
	int32_t channel;
} WiFiScanAP;

typedef struct {
	bool running = false;
	unsigned long timeout = 0;
	uint8_t count = 0;
	WiFiScanAP *ap = NULL;
} WiFiScanData;

typedef struct {
	struct {
		int if_index;
		void *esp_netif;
		struct {
			struct {
				uint32_t addr;
			} ip, gw, netmask;
		} ip_info;
		bool ip_changed;
	} got_ip;
} EventInfo;

struct ip_addr {
	uint32_t addr;
};

struct netif {
	struct ip_addr ip_addr;
	struct ip_addr netmask;
	struct ip_addr gw;
	uint8_t hwaddr[ETH_ALEN];
};

extern struct netif mock_netif;
extern struct ip_addr mock_dns;
#define NETIF_RTW_STA &mock_netif
#define NETIF_RTW_AP &mock_netif
#define netif_ip_addr4(n) (&(n)->ip_addr)
#define netif_ip_netmask4(n) (&(n)->netmask)
#define netif_ip_gw4(n) (&(n)->gw)
#define dns_getserver(n) (&mock_dns)

typedef struct {
	int mode;
	unsigned char ssid[33];
	unsigned char channel;
	rtw_security_t security_type;
	unsigned char password[65];
	unsigned char key_idx;
} rtw_wifi_setting_t;

typedef struct {
	WiFiNetworkInfo sta;
	WiFiNetworkInfo ap;
} WiFiData;

extern wifi_mode_t wifi_mode;
extern rtw_wifi_setting_t wifi_setting;

extern "C" {
void lt_wdt_feed(void);
int wext_set_ssid(const char *ifname, const uint8_t *ssid, uint16_t len);
int wifi_connect(char *ssid, rtw_security_t security_type, char *password, int ssid_len, int password_len, int key_id, void *semaphore);
int wifi_connect_bssid(unsigned char bssid[ETH_ALEN], char *ssid, rtw_security_t security_type, char *password, int bssid_len, int ssid_len, int password_len, int key_id, void *semaphore);
int wifi_set_pscan_chan(uint8_t *channel_list, uint8_t *pscan_config, uint8_t length);
int wifi_disconnect(void);
void wifi_indication(int event, char *buf, int len, int flags);
int LwIP_DHCP(int idx, int start);
int LwIP_GetBC(struct netif *n);
void dns_setserver(int i, struct ip_addr *server);
void netif_set_addr(struct netif *n, struct ip_addr *ip, struct ip_addr *mask, struct ip_addr *gw);
const char *netif_get_hostname(struct netif *n);
int netif_set_hostname(struct netif *n, char *name);
int wifi_get_setting(const char *ifname, rtw_wifi_setting_t *setting);
int wifi_get_rssi(int *rssi);
int wifi_set_autoreconnect(uint8_t mode);
int wifi_get_autoreconnect(uint8_t *mode);
int wifi_set_mac_address(char *mac);
int wext_get_bssid(const char *ifname, uint8_t *bssid);
void lt_get_device_mac(uint8_t *mac);
}

WiFiAuthMode securityTypeToAuthMode(uint8_t type);

#define DATA ((WiFiData *)data)

class WiFiClass {
  public:
	void *data = NULL;
	WiFiScanData *scan = NULL;
	char staHostname[64] = {0};
	char apHostname[64] = {0};

	WiFiStatus begin(const char *ssid, const char *passphrase = NULL, int32_t channel = 0, const uint8_t *bssid = NULL, bool connect = true);
	bool config(IPAddress localIP, IPAddress gateway, IPAddress subnet, IPAddress dns1 = IPAddress(), IPAddress dns2 = IPAddress());
	bool reconnect(const uint8_t *bssid = NULL);
	bool disconnect(bool wifiOff = false);
	bool isConnected();
	bool setAutoReconnect(bool autoReconnect);
	bool getAutoReconnect();
	IPAddress localIP();
	uint8_t *macAddress(uint8_t *mac);
	IPAddress subnetMask();
	IPAddress gatewayIP();
	IPAddress dnsIP(uint8_t dns_no = 0);
	IPAddress broadcastIP();
	const char *getHostname();
	bool setHostname(const char *hostname);
	bool setMacAddress(const uint8_t *mac);
	const String SSID();
	const String psk();
	uint8_t *BSSID();
	int8_t RSSI();
	WiFiAuthMode getEncryption();

	wifi_mode_t getMode() { return (wifi_mode_t)wifi_mode; }

	static bool validate(const char *ssid, const char *passphrase);
	static const char *storeHostname(char *dst, const char *src, size_t size = 64);
	static void resetNetworkInfo(WiFiNetworkInfo &info);
	static bool enableSTA(bool enable);
};

void mock_log_i(const char *fmt, ...);
void mock_log_e(const char *fmt, ...);
"""

MOCK_LIBRETINY = r"""
#pragma once
/* Nothing: WiFiPrivate.h supplies the host-test surface. */
"""

TEST_CPP = r"""
#include <cstdarg>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <string>
#include <vector>

#include "WiFiPrivate.h"

/* ---- recording surface ---- */
std::vector<std::string> g_logs;
std::vector<std::string> g_calls;
int g_wdt_feeds = 0;
int g_dhcp_calls = 0;
int g_preclear_ret = 0;
int g_wext_calls = 0;
int g_connect_ret = 0;
int g_connect_bssid_ret = 0;
int g_last_key_id = -12345;
int g_last_bssid_key_id = -12345;

struct netif mock_netif = {};
struct ip_addr mock_dns = {};
WiFiData g_data = {};
WiFiClass g_wifi;
int wifi_mode = WIFI_MODE_STA;
rtw_wifi_setting_t wifi_setting = {};
static int g_plain_attempts = 0;
static int g_bssid_attempts = 0;
static int g_pscan_enables = 0;
static int g_pscan_disables = 0;

void mock_log_i(const char *fmt, ...) {
	char buf[512];
	va_list ap;
	va_start(ap, fmt);
	vsnprintf(buf, sizeof(buf), fmt, ap);
	va_end(ap);
	g_logs.push_back(buf);
}

void mock_log_e(const char *fmt, ...) {
	char buf[512];
	va_list ap;
	va_start(ap, fmt);
	vsnprintf(buf, sizeof(buf), fmt, ap);
	va_end(ap);
	g_logs.push_back(std::string("E: ") + buf);
}

static bool has_log(const char *needle) {
	for (const std::string &line : g_logs)
		if (line.find(needle) != std::string::npos)
			return true;
	return false;
}

static int log_index(const char *needle) {
	for (size_t i = 0; i < g_logs.size(); i++)
		if (g_logs[i].find(needle) != std::string::npos)
			return (int)i;
	return -1;
}

/* ---- SDK / Arduino stubs ---- */
extern "C" void lt_wdt_feed(void) { g_wdt_feeds++; }

extern "C" {
int wext_set_ssid(const char *ifname, const uint8_t *ssid, uint16_t len) {
	(void)ifname;
	(void)ssid;
	(void)len;
	g_wext_calls++;
	g_calls.push_back("wext_set_ssid");
	return g_preclear_ret;
}

int wifi_connect(char *ssid, rtw_security_t security_type, char *password, int ssid_len, int password_len, int key_id, void *semaphore) {
	(void)password;
	(void)semaphore;
	g_last_key_id = key_id;
	g_plain_attempts++;
	g_calls.push_back("wifi_connect");
	if (security_type != RTW_SECURITY_WPA2_AES_PSK)
		return -99;
	if (ssid_len <= 0 || password_len <= 0)
		return -98;
	return g_connect_ret;
}

int wifi_connect_bssid(unsigned char bssid[ETH_ALEN], char *ssid, rtw_security_t security_type, char *password, int bssid_len, int ssid_len, int password_len, int key_id, void *semaphore) {
	(void)bssid;
	(void)ssid;
	(void)password;
	(void)bssid_len;
	(void)ssid_len;
	(void)password_len;
	(void)semaphore;
	g_last_bssid_key_id = key_id;
	g_bssid_attempts++;
	g_calls.push_back("wifi_connect_bssid");
	if (security_type != RTW_SECURITY_WPA2_AES_PSK)
		return -99;
	return g_connect_bssid_ret;
}

int wifi_set_pscan_chan(uint8_t *channel_list, uint8_t *pscan_config, uint8_t length) {
	(void)channel_list;
	(void)length;
	if (pscan_config[0]) {
		g_pscan_enables++;
		g_calls.push_back("pscan_enable");
	} else {
		g_pscan_disables++;
		g_calls.push_back("pscan_disable");
	}
	return 0;
}

int wifi_disconnect(void) { g_calls.push_back("wifi_disconnect"); return 0; }
void wifi_indication(int event, char *buf, int len, int flags) { (void)event; (void)buf; (void)len; (void)flags; }
int LwIP_DHCP(int idx, int start) { (void)idx; (void)start; g_dhcp_calls++; return DHCP_ADDRESS_ASSIGNED; }
int LwIP_GetBC(struct netif *n) { (void)n; return 0; }
void dns_setserver(int i, struct ip_addr *server) { (void)i; (void)server; }
void netif_set_addr(struct netif *n, struct ip_addr *ip, struct ip_addr *mask, struct ip_addr *gw) {
	n->ip_addr = *ip; n->netmask = *mask; n->gw = *gw;
}
const char *netif_get_hostname(struct netif *n) { (void)n; return "host"; }
int netif_set_hostname(struct netif *n, char *name) { (void)n; (void)name; return 0; }
int wifi_get_setting(const char *ifname, rtw_wifi_setting_t *setting) {
	(void)ifname;
	memset(setting, 0, sizeof(*setting));
	return 0;
}
int wifi_get_rssi(int *rssi) { *rssi = -40; return 0; }
int wifi_set_autoreconnect(uint8_t mode) { (void)mode; return 0; }
int wifi_get_autoreconnect(uint8_t *mode) { *mode = 0; return 0; }
int wifi_set_mac_address(char *mac) { (void)mac; return 0; }
int wext_get_bssid(const char *ifname, uint8_t *bssid) { (void)ifname; memset(bssid, 0x11, ETH_ALEN); return 0; }
void lt_get_device_mac(uint8_t *mac) { memset(mac, 0x22, ETH_ALEN); }
}

WiFiAuthMode securityTypeToAuthMode(uint8_t type) { (void)type; return WIFI_AUTH_WPA2_PSK; }

bool WiFiClass::validate(const char *ssid, const char *passphrase) { (void)passphrase; return ssid != NULL; }
const char *WiFiClass::storeHostname(char *dst, const char *src, size_t size) {
	strncpy(dst, src, size - 1);
	dst[size - 1] = '\0';
	return dst;
}
void WiFiClass::resetNetworkInfo(WiFiNetworkInfo &info) {
	free(info.ssid); free(info.password); free(info.bssid);
	memset(&info, 0, sizeof(info));
}
bool WiFiClass::enableSTA(bool enable) { (void)enable; return true; }
bool WiFiClass::isConnected() { return true; }

static int fail(const char *what) {
	fprintf(stderr, "FAIL: %s\n", what);
	fprintf(stderr, "--- logs ---\n");
	for (const std::string &line : g_logs)
		fprintf(stderr, "%s\n", line.c_str());
	fprintf(stderr, "--- calls ---\n");
	for (const std::string &call : g_calls)
		fprintf(stderr, "%s\n", call.c_str());
	return 1;
}

static void reset_run(void) {
	g_logs.clear();
	g_calls.clear();
	g_wdt_feeds = 0;
	g_dhcp_calls = 0;
	g_preclear_ret = 0;
	g_wext_calls = 0;
	g_connect_ret = 0;
	g_connect_bssid_ret = 0;
	g_last_key_id = -12345;
	g_last_bssid_key_id = -12345;
	g_plain_attempts = 0;
	g_bssid_attempts = 0;
	g_pscan_enables = 0;
	g_pscan_disables = 0;
}

static void prepare(const char *ssid, const char *pass, int channel) {
	memset(&g_data, 0, sizeof(g_data));
	g_data.sta.ssid = strdup(ssid);
	g_data.sta.password = strdup(pass);
	g_data.sta.channel = channel;
	g_data.sta.auth = RTW_SECURITY_WPA2_AES_PSK;
	g_wifi.data = &g_data;
	memset(&mock_netif, 0, sizeof(mock_netif));
}

static const uint8_t g_bssid[6] = {0x02, 0x11, 0x22, 0x33, 0x44, 0x55};

static int scenario_amb1_bssid_ignored(void) {
	reset_run();
	prepare("test-network-iot", "secret123", 6);
	g_connect_ret = 0;
	WiFiStatus st = g_wifi.begin("test-network-iot", "secret123", 6, g_bssid, true);
	if (st != WL_CONNECTED) return fail("expected WL_CONNECTED");
	if (g_plain_attempts != 1) return fail("AMB1 must make exactly one plain wifi_connect");
	if (g_bssid_attempts != 0) return fail("AMB1 must ignore the bssid argument");
	if (g_pscan_enables != 0 || g_pscan_disables != 0) return fail("AMB1 must not touch pscan");
	if (g_wext_calls != 0) return fail("AMB1 must never call wext_set_ssid pre-clear");
	if (g_last_key_id != 0) return fail("AMB1 must use the vendor key_id=0");
	if (g_dhcp_calls != 1) return fail("DHCP must run after a successful plain join");
	if (g_wdt_feeds < 2) return fail("watchdog must be fed before and after the join");
	if (!has_log("AMB1 diag: ssid_len=16 pass_len=9 auth=0x00400004 channel=6 bssid_arg=ignored path=plain"))
		return fail("diagnostic line missing/mismatched");
	if (g_calls.size() != 1 || g_calls[0] != "wifi_connect")
		return fail("AMB1 call sequence must be exactly one plain wifi_connect");
	for (const std::string &line : g_logs)
		if (line.find("secret123") != std::string::npos)
			return fail("password leaked to log");
	puts("amb1_bssid_ignored OK");
	return 0;
}

static int scenario_amb1_fail(void) {
	reset_run();
	prepare("test-network-iot", "secret123", 6);
	g_connect_ret = -1;
	WiFiStatus st = g_wifi.begin("test-network-iot", "secret123", 6, g_bssid, true);
	if (st != WL_CONNECT_FAILED) return fail("expected WL_CONNECT_FAILED");
	if (g_plain_attempts != 1) return fail("AMB1 must not retry a second security/path");
	if (g_bssid_attempts != 0) return fail("AMB1 must not fall back to bssid");
	if (g_pscan_enables != 0 || g_pscan_disables != 0) return fail("AMB1 must not touch pscan");
	if (g_wext_calls != 0) return fail("AMB1 must not pre-clear");
	if (g_dhcp_calls != 0) return fail("DHCP must not run after a failed join");
	if (!has_log("Connection failed; ret=-1")) return fail("existing ret error log missing");
	puts("amb1_fail OK");
	return 0;
}

static int scenario_amb1_no_bssid(void) {
	reset_run();
	prepare("test-network-iot", "secret123", 0);
	g_connect_ret = 0;
	WiFiStatus st = g_wifi.begin("test-network-iot", "secret123", 0, NULL, true);
	if (st != WL_CONNECTED) return fail("expected WL_CONNECTED");
	if (g_plain_attempts != 1) return fail("AMB1 must make one plain attempt");
	if (g_last_key_id != 0) return fail("AMB1 must use key_id=0");
	if (!has_log("AMB1 diag: ssid_len=16 pass_len=9 auth=0x00400004 channel=0 bssid_arg=absent path=plain"))
		return fail("no-bssid diagnostic mismatch");
	puts("amb1_no_bssid OK");
	return 0;
}

static int scenario_amb1_repeat(void) {
	reset_run();
	prepare("test-network-iot", "secret123", 6);
	g_connect_ret = 0;
	if (g_wifi.begin("test-network-iot", "secret123", 6, g_bssid, true) != WL_CONNECTED)
		return fail("first plain join should succeed");
	if (!g_wifi.reconnect(g_bssid)) return fail("second reconnect should succeed");
	if (!g_wifi.reconnect(NULL)) return fail("third reconnect should succeed");
	if (g_plain_attempts != 3) return fail("every reconnect must use the plain path");
	if (g_bssid_attempts != 0 || g_pscan_enables != 0 || g_pscan_disables != 0 || g_wext_calls != 0)
		return fail("AMB1 must never use bssid/pscan/pre-clear, including reconnects");
	if (g_last_key_id != 0) return fail("AMB1 reconnects must keep key_id=0");
	puts("amb1_repeat OK");
	return 0;
}

static int scenario_non_amb1_bssid(void) {
	reset_run();
	prepare("test-network-iot", "secret123", 6);
	g_connect_bssid_ret = 0;
	WiFiStatus st = g_wifi.begin("test-network-iot", "secret123", 6, g_bssid, true);
	if (st != WL_CONNECTED) return fail("expected WL_CONNECTED");
	if (g_plain_attempts != 0) return fail("non-AMB1 must not use the plain path when bssid is present");
	if (g_bssid_attempts != 1) return fail("non-AMB1 must make one bssid attempt");
	if (g_wext_calls != 1) return fail("non-AMB1 pre-clear must remain");
	if (g_last_bssid_key_id != -1) return fail("non-AMB1 bssid key_id must stay -1");
	if (g_pscan_enables != 1 || g_pscan_disables != 1) return fail("non-AMB1 pscan order changed");
	if (has_log("AMB1")) return fail("AMB1 diagnostics leaked into non-AMB1 build");
	if (!has_log("Connecting to test-network-iot")) return fail("original non-AMB1 log missing");
	if (g_calls.size() != 4) return fail("non-AMB1 call ordering changed");
	if (g_calls[0] != "wext_set_ssid" || g_calls[1] != "pscan_enable" ||
		g_calls[2] != "wifi_connect_bssid" || g_calls[3] != "pscan_disable")
		return fail("non-AMB1 call order mismatch");
	puts("non_amb1_bssid OK");
	return 0;
}

static int scenario_non_amb1_plain_fail(void) {
	reset_run();
	prepare("test-network-iot", "secret123", 0);
	g_connect_ret = -1;
	WiFiStatus st = g_wifi.begin("test-network-iot", "secret123", 0, NULL, true);
	if (st != WL_CONNECT_FAILED) return fail("expected WL_CONNECT_FAILED");
	if (g_plain_attempts != 1) return fail("non-AMB1 must make exactly one plain attempt");
	if (g_bssid_attempts != 0) return fail("non-AMB1 must not use bssid");
	if (g_wext_calls != 1) return fail("non-AMB1 pre-clear missing");
	if (g_last_key_id != -1) return fail("non-AMB1 plain key_id must stay -1");
	if (has_log("AMB1")) return fail("AMB1 diagnostics leaked into non-AMB1 build");
	if (g_dhcp_calls != 0) return fail("DHCP must not run after failure");
	puts("non_amb1_plain_fail OK");
	return 0;
}

int main(int argc, char **argv) {
	if (argc != 2) {
		fprintf(stderr, "usage: %s <scenario>\n", argv[0]);
		return 2;
	}
	std::string s = argv[1];
	if (s == "amb1_bssid_ignored") return scenario_amb1_bssid_ignored();
	if (s == "amb1_fail") return scenario_amb1_fail();
	if (s == "amb1_no_bssid") return scenario_amb1_no_bssid();
	if (s == "amb1_repeat") return scenario_amb1_repeat();
	if (s == "non_amb1_bssid") return scenario_non_amb1_bssid();
	if (s == "non_amb1_plain_fail") return scenario_non_amb1_plain_fail();
	fprintf(stderr, "unknown scenario: %s\n", s.c_str());
	return 2;
}
"""

HELPER_TEST = r"""
#include <cstdio>
#include <cstring>
#include "WiFiConnectDiag.h"

int main(void) {
	if (LT_AMB1_JOIN_KEY_ID != 0) return 10;
	if (LT_AMB1_JOIN_USES_BSSID != 0) return 11;
	if (LT_AMB1_JOIN_USES_PSCAN != 0) return 12;
	if (strcmp(lt_amb1_path_name(), "plain") != 0) return 13;
	puts("helper OK");
	return 0;
}
"""


def _compile(
    work: pathlib.Path, source: pathlib.Path, out: str, amb1: bool, extra=None
):
    cmd = [
        "c++",
        "-std=c++17",
        "-Wall",
        "-Wextra",
        "-Wno-unused-parameter",
        "-Wno-c++11-narrowing",
        "-I",
        str(work),
    ]
    if amb1:
        cmd.append("-DLT_AMB1=1")
    cmd += list(extra or [])
    cmd += [str(source), "-o", str(work / out)]
    result = subprocess.run(cmd, check=False, cwd=work, capture_output=True, text=True)
    if result.returncode != 0:
        raise AssertionError(
            f"compile failed (rc={result.returncode}): {' '.join(cmd)}\n"
            f"stdout:\n{result.stdout}\nstderr:\n{result.stderr}"
        )
    return work / out


def _build_integration(work: pathlib.Path, amb1: bool):
    work.mkdir(parents=True, exist_ok=True)
    (work / "WiFiPrivate.h").write_text(MOCK_WIFI_PRIVATE)
    (work / "libretiny.h").write_text(MOCK_LIBRETINY)
    # Copy the real sources so their quoted includes resolve to the mocks.
    (work / "WiFiSTA.cpp").write_text(WIFI_STA.read_text())
    (work / "WiFiConnectDiag.h").write_text(HELPER.read_text())
    (work / "test.cpp").write_text(TEST_CPP)
    return _compile(
        work, work / "test.cpp", "connect_diag", amb1, [str(work / "WiFiSTA.cpp")]
    )


def _run(binary: pathlib.Path, scenario: str):
    return subprocess.run(
        [str(binary), scenario], check=False, capture_output=True, text=True
    )


class ConnectDiagTest(unittest.TestCase):
    def test_helper_policy(self):
        with tempfile.TemporaryDirectory(prefix="amb1-helper-") as d:
            work = pathlib.Path(d)
            (work / "WiFiConnectDiag.h").write_text(HELPER.read_text())
            (work / "helper.cpp").write_text(HELPER_TEST)
            binary = _compile(work, work / "helper.cpp", "helper", amb1=True)
            result = subprocess.run(
                [str(binary)], check=False, capture_output=True, text=True
            )
            self.assertEqual(result.returncode, 0, result.stderr)
            self.assertIn("helper OK", result.stdout)

    def test_amb1_ignores_bssid_and_uses_plain_connect(self):
        with tempfile.TemporaryDirectory(prefix="amb1-connect-") as d:
            binary = _build_integration(pathlib.Path(d), amb1=True)
            result = _run(binary, "amb1_bssid_ignored")
            self.assertEqual(result.returncode, 0, f"{result.stderr}\n{result.stdout}")
            self.assertIn("amb1_bssid_ignored OK", result.stdout)

    def test_amb1_has_no_bssid_pscan_or_preclear_fallback(self):
        with tempfile.TemporaryDirectory(prefix="amb1-connect-") as d:
            binary = _build_integration(pathlib.Path(d), amb1=True)
            result = _run(binary, "amb1_fail")
            self.assertEqual(result.returncode, 0, f"{result.stderr}\n{result.stdout}")
            self.assertIn("amb1_fail OK", result.stdout)

    def test_amb1_no_bssid_single_plain_attempt(self):
        with tempfile.TemporaryDirectory(prefix="amb1-connect-") as d:
            binary = _build_integration(pathlib.Path(d), amb1=True)
            result = _run(binary, "amb1_no_bssid")
            self.assertEqual(result.returncode, 0, f"{result.stderr}\n{result.stdout}")
            self.assertIn("amb1_no_bssid OK", result.stdout)

    def test_amb1_reconnects_stay_plain(self):
        with tempfile.TemporaryDirectory(prefix="amb1-connect-") as d:
            binary = _build_integration(pathlib.Path(d), amb1=True)
            result = _run(binary, "amb1_repeat")
            self.assertEqual(result.returncode, 0, f"{result.stderr}\n{result.stdout}")
            self.assertIn("amb1_repeat OK", result.stdout)

    def test_non_amb1_bssid_path_unchanged(self):
        with tempfile.TemporaryDirectory(prefix="amb1-connect-") as d:
            binary = _build_integration(pathlib.Path(d), amb1=False)
            result = _run(binary, "non_amb1_bssid")
            self.assertEqual(result.returncode, 0, f"{result.stderr}\n{result.stdout}")
            self.assertIn("non_amb1_bssid OK", result.stdout)

    def test_non_amb1_plain_path_unchanged(self):
        with tempfile.TemporaryDirectory(prefix="amb1-connect-") as d:
            binary = _build_integration(pathlib.Path(d), amb1=False)
            result = _run(binary, "non_amb1_plain_fail")
            self.assertEqual(result.returncode, 0, f"{result.stderr}\n{result.stdout}")
            self.assertIn("non_amb1_plain_fail OK", result.stdout)


if __name__ == "__main__":
    unittest.main(verbosity=2)
