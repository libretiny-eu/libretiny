#!/usr/bin/env python3
"""Host-execute real AMB1 recovery paths; no SDK, device, or credentials needed.

AP and scan tests guard fixes; remaining known-defect assertions characterize
unmodified STA/event behavior. Reuses adjacent mocks with actual shared sources.
"""

import pathlib
import subprocess
import tempfile
import unittest

import test_connect_diag as connect
import test_scan_lifetime as events


def run_cpp(files, source, amb1=1):
    with tempfile.TemporaryDirectory(prefix="amb1-recovery-") as directory:
        work = pathlib.Path(directory)
        for name, content in files.items():
            (work / name).write_text(content)
        (work / "test.cpp").write_text(source)
        binary = work / "test"
        compiled = subprocess.run(
            ["c++", "-std=c++17", "-Wno-c++11-narrowing", f"-DLT_AMB1={amb1}", "-I", str(work),
             str(work / "test.cpp"), "-o", str(binary)],
            check=False, capture_output=True, text=True,
        )
        if compiled.returncode:
            raise AssertionError(compiled.stderr)
        result = subprocess.run([str(binary)], capture_output=True, text=True)
        if result.returncode:
            raise AssertionError(f"rc={result.returncode}\n{result.stdout}\n{result.stderr}")


class WiFiRecoveryTest(unittest.TestCase):
    def test_softap_open_hidden_failures_and_success(self):
        declarations = r'''
    static bool enableAP(bool);
    bool softAP(const char *, const char *, int, bool, int);
    bool softAPConfig(IPAddress, IPAddress, IPAddress);
    bool softAPdisconnect(bool);
    uint8_t softAPgetStationNum();
    IPAddress softAPIP(); IPAddress softAPSubnetMask();
    const char *softAPgetHostname(); bool softAPsetHostname(const char *);
    uint8_t *softAPmacAddress(uint8_t *); String softAPmacAddress();
    const String softAPSSID();
'''
        private = connect.MOCK_WIFI_PRIVATE.replace(
            "char staHostname[64] = {0};", declarations + "char staHostname[64] = {0};"
        ).replace("#define NETIF_RTW_AP &mock_netif", "#define NETIF_RTW_AP (&mock_netif)")
        prelude = connect.TEST_CPP.split("int main(", 1)[0].replace(
            "void wifi_indication(int event, char *buf, int len, int flags) { (void)event; (void)buf; (void)len; (void)flags; }",
            "void wifi_indication(int, char *, int, int) { recovery_ap_events++; }",
        )
        source = "int recovery_ap_events = 0;\n" + prelude + r'''
#include <cassert>
#define AP_STA_NUM 4
#define NETNAME_AP "wlan1"
#define ARDUINO_EVENT_WIFI_AP_START 6
#define configTICK_RATE_HZ 1000
struct rtw_mac_t { uint8_t octet[6]; };
bool ap_enable = true, ap_ready = true;
int ap_result = 0, ap_calls = 0, dhcp_starts = 0, dhcp_stops = 0, dns_stops = 0;
bool called_hidden = false;
const char *expected_pass = nullptr;
const char *expected_ssid = "mock-ap";
bool WiFiClass::enableAP(bool) { return ap_enable; }
void vTaskDelay(int) {}
void dhcps_deinit() { dhcp_stops++; }
void dhcps_init(netif *) { dhcp_starts++; }
void dns_server_deinit() { dns_stops++; }
int start_ap(char *ssid, rtw_security_t auth, char *pass, int ssid_len, int pass_len, int channel, bool hidden) {
    ap_calls++; called_hidden = hidden;
    assert(strcmp(ssid, expected_ssid) == 0 && ssid_len == (int)strlen(expected_ssid));
    assert(channel == 6);
    if (expected_pass) {
        assert(pass && strcmp(pass, expected_pass) == 0);
        assert(pass_len == (int)strlen(expected_pass) && auth == RTW_SECURITY_WPA2_AES_PSK);
    } else {
        assert(pass == nullptr && pass_len == 0 && auth == RTW_SECURITY_OPEN);
    }
    return ap_result;
}
int wifi_start_ap(char *s, rtw_security_t a, char *p, int sl, int pl, int c) {
    return start_ap(s, a, p, sl, pl, c, false);
}
int wifi_start_ap_with_hidden_ssid(char *s, rtw_security_t a, char *p, int sl, int pl, int c) {
    return start_ap(s, a, p, sl, pl, c, true);
}
int wext_get_ssid(const char *, unsigned char *out) {
    if (!ap_ready) return -1;
    strcpy((char *)out, expected_ssid);
    return strlen(expected_ssid);
}
void wifi_get_associated_client_list(void *, size_t) {}
String macToString(uint8_t *) { return String("mock-mac"); }
#include "WiFiAP.cpp"
int main() {
    g_wifi.data = &g_data;
    ap_enable = false;
    assert(!g_wifi.softAP(expected_ssid, nullptr, 6, false, 4));
    assert(ap_calls == 0 && dhcp_stops == 0);
    ap_enable = true;
    assert(!g_wifi.softAP(nullptr, nullptr, 6, false, 4));
    assert(ap_calls == 0 && dhcp_stops == 0);
    for (bool hidden : {false, true}) {
        for (const char *pass : {static_cast<const char *>(nullptr), "mock-password"}) {
            expected_pass = pass;
            int starts = dhcp_starts, events = recovery_ap_events, dns = dns_stops;
            ap_result = -1;
            assert(!g_wifi.softAP(expected_ssid, pass, 6, hidden, 4));
            assert(called_hidden == hidden);
            assert(dhcp_starts == starts && recovery_ap_events == events && dns_stops == dns);
            ap_result = 0; ap_ready = false;
            assert(!g_wifi.softAP(expected_ssid, pass, 6, hidden, 4));
            assert(dhcp_starts == starts && recovery_ap_events == events && dns_stops == dns);
            ap_ready = true;
            assert(g_wifi.softAP(expected_ssid, pass, 6, hidden, 4));
            assert(called_hidden == hidden);
            assert(dhcp_starts == starts + 1 && recovery_ap_events == events + 1 && dns_stops == dns + 1);
        }
    }
    assert(ap_calls == 12 && dhcp_stops == 12 && dhcp_starts == 4);
    WiFiClass::resetNetworkInfo(g_data.ap);
}
'''
        for amb1 in (1, 0):
            with self.subTest(amb1=amb1):
                run_cpp({"WiFiPrivate.h": private,
                         "WiFiAP.cpp": (connect.WIFI_STA.parent / "WiFiAP.cpp").read_text()},
                        source, amb1=amb1)

    def test_scan_start_failure_and_synchronous_completion(self):
        private = connect.MOCK_WIFI_PRIVATE.replace(
            "char staHostname[64] = {0};",
            "int16_t scanNetworks(bool, bool, bool, uint32_t, uint8_t = 0);\n"
            "void scanDelete(); void scanInit(); uint8_t scanAlloc(uint8_t);\n"
            "char staHostname[64] = {0};",
        ).replace("WiFiNetworkInfo sta;", "int scanSem; WiFiNetworkInfo sta;")
        prelude = connect.TEST_CPP.split("int main(", 1)[0]
        source = prelude + r'''
#include <cassert>
#define cDATA ((WiFiData *)cls->data)
#define RTW_TRUE 1
#define WIFI_SCAN_RUNNING -1
#define WIFI_SCAN_FAILED -2
#define pdMS_TO_TICKS(x) (x)
typedef int rtw_result_t;
struct rtw_scan_result_t {
    struct { uint8_t val[33]; uint8_t len; } SSID;
    int security, signal_strength, channel;
    struct { uint8_t octet[6]; } BSSID;
};
struct rtw_scan_handler_result_t {
    void *user_data;
    int scan_complete;
    rtw_scan_result_t ap_details;
};
bool completion = true; // a stale completion must be drained before startup
int empty_waits = 0;
void xSemaphoreGive(int) { completion = true; }
void xSemaphoreTake(int, unsigned ticks) {
    if (ticks > 1 && !completion) empty_waits++;
    completion = false;
}
int scan_mode = 0;
int scan_calls = 0;
rtw_result_t (*pending_handler)(rtw_scan_handler_result_t *);
void *pending_user;
void finish_scan() {
    rtw_scan_handler_result_t done = {};
    done.user_data = pending_user; done.scan_complete = RTW_TRUE;
    pending_handler(&done);
}
int wifi_scan_networks(rtw_result_t (*handler)(rtw_scan_handler_result_t *), void *user) {
    scan_calls++;
    assert(!completion);
    assert(((WiFiClass *)user)->scan->running);
    if (scan_mode == 0) return -1;
    pending_handler = handler; pending_user = user;
    if (scan_mode == 1) finish_scan(); // completion before SDK returns
    return RTW_SUCCESS;
}
void WiFiClass::scanDelete() { delete scan; scan = nullptr; }
void WiFiClass::scanInit() { scan = new WiFiScanData{}; }
uint8_t WiFiClass::scanAlloc(uint8_t) { return 0; }
#include "WiFiScan.cpp"
int main() {
    g_wifi.data = &g_data;
    assert(g_wifi.scanNetworks(true, true, false, 200) == WIFI_SCAN_FAILED);
    assert(!g_wifi.scan->running);
    scan_mode = 1;
    assert(g_wifi.scanNetworks(true, true, false, 200) == WIFI_SCAN_RUNNING);
    assert(!g_wifi.scan->running);
    // Completed async scans allow a fresh SDK attempt, including failure.
    scan_mode = 0;
    assert(g_wifi.scanNetworks(true, true, false, 200) == WIFI_SCAN_FAILED);
    assert(!g_wifi.scan->running && scan_calls == 3);
    scan_mode = 1;
    assert(g_wifi.scanNetworks(false, true, false, 200) == 0);
    assert(!g_wifi.scan->running && empty_waits == 0);
    // Ordinary delayed completion still prevents overlapping scans.
    scan_mode = 2;
    assert(g_wifi.scanNetworks(true, true, false, 200) == WIFI_SCAN_RUNNING);
    assert(g_wifi.scan->running && scan_calls == 5);
    assert(g_wifi.scanNetworks(true, true, false, 200) == WIFI_SCAN_RUNNING);
    assert(scan_calls == 5);
    finish_scan();
    assert(!g_wifi.scan->running);
    scan_mode = 1;
    assert(g_wifi.scanNetworks(false, true, false, 200) == 0);
    assert(scan_calls == 6 && empty_waits == 0);
    g_wifi.scanDelete();
}
'''
        for amb1 in (1, 0):
            with self.subTest(amb1=amb1):
                run_cpp({"WiFiPrivate.h": private,
                         "WiFiScan.cpp": (connect.WIFI_STA.parent / "WiFiScan.cpp").read_text()},
                        source, amb1=amb1)

    def test_join_failure_dhcp_timeout_and_repeated_begin(self):
        # Retain existing SDK stubs, replacing only configurable observations.
        prelude = connect.TEST_CPP.split("int main(", 1)[0]
        prelude = prelude.replace(
            "int LwIP_DHCP(int idx, int start) { (void)idx; (void)start; g_dhcp_calls++; return DHCP_ADDRESS_ASSIGNED; }",
            "int LwIP_DHCP(int, int) { g_dhcp_calls++; return recovery_dhcp_result; }",
        ).replace(
            "void wifi_indication(int event, char *buf, int len, int flags) { (void)event; (void)buf; (void)len; (void)flags; }",
            "void wifi_indication(int, char *, int, int) { recovery_ip_events++; }",
        )
        # Exercise actual common reset semantics (IP fields intentionally survive).
        prelude = prelude.replace(
            "memset(&info, 0, sizeof(info));",
            "memset(&info, 0, sizeof(info) - 5 * sizeof(uint32_t));",
        )
        source = "int recovery_dhcp_result = 1; int recovery_ip_events = 0;\n" + prelude + r'''
#include <cassert>
#include "WiFiSTA.cpp"
int main() {
    g_wifi.data = &g_data;
    // Auth failure and absent AP both surface as failed synchronous joins.
    for (int failure : {-1, -2}) {
        g_connect_ret = failure;
        int dhcp = g_dhcp_calls;
        assert(g_wifi.begin("mock-network", "mock-password") == WL_CONNECT_FAILED);
        assert(g_dhcp_calls == dhcp && recovery_ip_events == 0);
    }
    g_connect_ret = RTW_SUCCESS;
    recovery_dhcp_result = 99;
    assert(g_wifi.begin("mock-network", "mock-password") == WL_CONNECT_FAILED);
    assert(g_calls.back() == "wifi_disconnect" && recovery_ip_events == 0);
    recovery_dhcp_result = DHCP_ADDRESS_ASSIGNED;
    for (int i = 0; i < 100; i++) {
        assert(g_wifi.begin("mock-network", "mock-password") == WL_CONNECTED);
        assert(g_wifi.disconnect());
        assert(g_data.sta.ssid == nullptr);
    }
    assert(recovery_ip_events == 100);
    assert(g_bssid_attempts == 0 && g_pscan_enables == 0 && g_wext_calls == 0);
    // Known defect: resetNetworkInfo preserves static IP across network changes.
    assert(g_wifi.config(IPAddress(0x01020304), IPAddress(1), IPAddress(2)));
    int dhcp = g_dhcp_calls;
    assert(g_wifi.begin("second-network", "mock-password") == WL_CONNECTED);
    assert(g_data.sta.localIP == 0x01020304 && g_dhcp_calls == dhcp);
    WiFiClass::resetNetworkInfo(g_data.sta);
}
'''
        wifi_dir = connect.WIFI_STA.parent
        run_cpp({
            "WiFiPrivate.h": connect.MOCK_WIFI_PRIVATE,
            "libretiny.h": connect.MOCK_LIBRETINY,
            "WiFiSTA.cpp": connect.WIFI_STA.read_text(),
            "WiFiConnectDiag.h": (wifi_dir / "WiFiConnectDiag.h").read_text(),
        }, source)

    def test_disconnect_translation_and_queued_payload_ownership(self):
        prelude = events.TEST_CPP.split("int main(", 1)[0]
        prelude = prelude.replace("int g_post_calls = 0;", "EventInfo recovery_info = {}; int g_post_calls = 0;")
        prelude = prelude.replace("void WiFiClass::postEvent(EventId id, EventInfo) {",
                                  "void WiFiClass::postEvent(EventId id, EventInfo info) { recovery_info = info;")
        source = prelude + r'''
#include <cassert>
static void drain() {
    for (auto *ev : g_queue) {
        handleRtwEvent(ev->event, ev->buf, ev->buf_len, ev->flags);
        test_free(ev->buf); test_free(ev);
    }
    g_queue.clear();
}
int main() {
    wifiEventQueueHandle = (xQueueHandle)1;
    wifiEventTaskHandle = (xTaskHandle)1;
    char stack_payload[8] = "payload";
    wifi_indication(WIFI_EVENT_DISCONNECT, stack_payload, 8, 0);
    assert(g_post_calls == 0 && g_queue.size() == 1);
    memset(stack_payload, 0, sizeof(stack_payload));
    assert(memcmp(g_queue[0]->buf, "payload", 8) == 0);
    drain();
    assert(g_last_post == ARDUINO_EVENT_WIFI_STA_DISCONNECTED);
    assert(recovery_info.wifi_sta_disconnected.reason == WIFI_REASON_UNSPECIFIED);
    assert(recovery_info.wifi_sta_disconnected.ssid_len == 0);
    wifi_indication(WIFI_EVENT_RECONNECTION_FAIL, nullptr, 0, 0);
    drain();
    assert(recovery_info.wifi_sta_disconnected.reason == WIFI_REASON_CONNECTION_FAIL);
    int posted = g_post_calls;
    wifi_indication(WIFI_EVENT_NO_NETWORK, nullptr, 0, 0);
    drain();
    assert(g_post_calls == posted); // Known diagnostic gap: not translated.
    assert(g_mallocs == 4); // record+payload, then two records
    // Null-buffer frees are counted by the inherited mock too.
    assert(g_frees == 6);
}
'''
        run_cpp({"WiFiPrivate.h": events.MOCK_WIFI_PRIVATE,
                 "WiFiEvents.cpp": events.WIFI_EVENTS.read_text()}, source)


if __name__ == "__main__":
    unittest.main()
