#!/usr/bin/env python3
"""Regression test for the Ameba-1 scan-result pointer lifetime.

Background (proven from the shipped driver, lib_wlan.a):
  report_scan_result_one (0x3001ba34) keeps the rtw_scan_result_t in its own
  stack frame - a holder at sp+24 holding a pointer to the 62-byte record at
  sp+40 - and calls indicate_wx_scan_result_present(), which forwards
  wrqu->data.pointer (= the holder) and wrqu->data.length (= 62) to
  wifi_indication(WIFI_EVENT_SCAN_RESULT_REPORT, ...). The driver reuses that
  receive-task frame the moment wext_wlan_indicate() returns.

  The SDK handler wifi_scan_each_report_hdl (wifi_conf.c:1608) reads the record
  and then rtw_memset(*result_ptr, 0, sizeof(rtw_scan_result_t)). That immediate
  clear is the "consumed" signal the driver depends on.

  The vendor's wifi_ind.c dispatches the registered handler synchronously
  (CONFIG_WIFI_IND_USE_THREAD is off in the Ameba-1 SDK, and OpenBeken ships
  that file unmodified). The Arduino port instead queued the event and ran the
  handler later, from the wifievent task, after the receive-task frame was
  gone - a stale read plus a stray memset into the reused stack.

The fix (WiFiEvents.cpp, #if LT_AMB1) dispatches pointer-bearing scan events
(SCAN_RESULT_REPORT / SCAN_DONE) synchronously in the calling driver context,
before the queue path, and leaves every other event on the existing queue.

This test compiles the *real* WiFiEvents.cpp against host mocks and drives it
through the exact producer shape (holder -> record on a stack buffer). It needs
no toolchain and no hardware. Run:

    python3 cores/realtek-amb1/tests/test_scan_lifetime.py
"""

import pathlib
import subprocess
import tempfile
import unittest

HERE = pathlib.Path(__file__).resolve()
CORES = HERE.parents[2]
WIFI_EVENTS = CORES / "realtek-amb" / "arduino" / "libraries" / "WiFi" / "WiFiEvents.cpp"

MOCK_WIFI_PRIVATE = r"""
#pragma once
/* Host-test mock of the real WiFiPrivate.h: only the surface that
 * WiFiEvents.cpp uses, so the real wifi_indication()/handleRtwEvent() logic can
 * be compiled and exercised on the host. Never part of a firmware build.
 *
 * The system headers are pulled in by test.cpp before this file is seen, so the
 * allocation-counting malloc/free macros do not rewrite any declarations here.
 */

/* ---- Realtek SDK event ids (subset used by WiFiEvents.cpp) ---- */
enum {
	WIFI_EVENT_CONNECT = 0,
	WIFI_EVENT_DISCONNECT,
	WIFI_EVENT_FOURWAY_HANDSHAKE_DONE,
	WIFI_EVENT_SCAN_RESULT_REPORT,
	WIFI_EVENT_SCAN_DONE,
	WIFI_EVENT_RECONNECTION_FAIL,
	WIFI_EVENT_NO_NETWORK,
	WIFI_EVENT_SEND_ACTION_DONE,
	WIFI_EVENT_RX_MGNT,
	WIFI_EVENT_STA_ASSOC,
	WIFI_EVENT_STA_DISASSOC,
	WIFI_EVENT_STA_WPS_START,
	WIFI_EVENT_WPS_FINISH,
	WIFI_EVENT_EAPOL_START,
	WIFI_EVENT_EAPOL_RECVD,
	WIFI_EVENT_BEACON_AFTER_DHCP,
	WIFI_EVENT_IP_CHANGED,
	WIFI_EVENT_ICV_ERROR,
	WIFI_EVENT_CHALLENGE_FAIL,
	WIFI_EVENT_MAX
};
typedef int rtw_event_indicate_t;

typedef void (*rtw_event_handler_t)(char *buf, int buf_len, int flags, void *handler_user_data);
typedef struct {
	rtw_event_handler_t handler;
	void *handler_user_data;
} event_list_elem_t;

/* ---- FreeRTOS ---- */
typedef void *xQueueHandle;
typedef void *xTaskHandle;
typedef int BaseType_t;
#define pdTRUE 1
#define portMAX_DELAY 0xffffffffu

extern "C" {
xQueueHandle xQueueCreate(unsigned len, unsigned size);
BaseType_t xQueueSend(xQueueHandle q, const void *item, uint32_t ticks);
BaseType_t xQueueReceive(xQueueHandle q, void *item, uint32_t ticks);
BaseType_t xTaskCreate(void (*fn)(void *), const char *name, unsigned stack, void *arg, unsigned prio,
                       xTaskHandle *out);
void *test_malloc(size_t n);
void test_free(void *p);
}

/* ---- Arduino event types (field-compatible subset) ---- */
typedef enum {
	ARDUINO_EVENT_WIFI_READY = 0,
	ARDUINO_EVENT_WIFI_SCAN_DONE,
	ARDUINO_EVENT_WIFI_STA_START,
	ARDUINO_EVENT_WIFI_STA_STOP,
	ARDUINO_EVENT_WIFI_STA_CONNECTED,
	ARDUINO_EVENT_WIFI_STA_DISCONNECTED,
	ARDUINO_EVENT_WIFI_AP_START,
	ARDUINO_EVENT_WIFI_AP_STOP,
	ARDUINO_EVENT_WIFI_AP_STACONNECTED,
	ARDUINO_EVENT_WIFI_AP_STADISCONNECTED,
	ARDUINO_EVENT_MAX
} EventId;

typedef struct {
	uint32_t status;
	uint8_t number;
	uint8_t scan_id;
} wifi_event_sta_scan_done_t;
typedef struct {
	uint8_t ssid[33];
	uint8_t ssid_len;
	uint8_t bssid[6];
	uint8_t channel;
	int authmode;
} wifi_event_sta_connected_t;
typedef struct {
	uint8_t ssid[33];
	uint8_t ssid_len;
	uint8_t bssid[6];
	uint8_t reason;
} wifi_event_sta_disconnected_t;
typedef struct {
	uint8_t mac[6];
	uint8_t aid;
} wifi_event_ap_staconnected_t;
typedef struct {
	uint8_t mac[6];
	uint8_t aid;
} wifi_event_ap_stadisconnected_t;
typedef union {
	wifi_event_sta_scan_done_t wifi_scan_done;
	wifi_event_sta_connected_t wifi_sta_connected;
	wifi_event_sta_disconnected_t wifi_sta_disconnected;
	wifi_event_ap_staconnected_t wifi_ap_staconnected;
	wifi_event_ap_stadisconnected_t wifi_ap_stadisconnected;
} EventInfo;

#define WIFI_REASON_UNSPECIFIED 0
#define WIFI_REASON_CONNECTION_FAIL 205

class String {
  public:
	String(const char *s = "") : s_(s ? s : "") {}
	size_t length() const { return s_.size(); }
	const char *c_str() const { return s_.c_str(); }

  private:
	std::string s_;
};

typedef enum { WIFI_AUTH_OPEN = 0, WIFI_AUTH_WEP, WIFI_AUTH_WPA_PSK, WIFI_AUTH_WPA2_PSK } WiFiAuthMode;

typedef struct {
	uint8_t count;
} WiFiScanData;

class WiFiClass {
  public:
	void *data = NULL;
	WiFiScanData *scan = NULL;
	static void postEvent(EventId eventId, EventInfo eventInfo);
	const String SSID();
	int32_t channel();
	WiFiAuthMode getEncryption();
	uint8_t *BSSID();
};
extern WiFiClass *pWiFi;

#define LT_EM(tag, ...) ((void)0)
#define LT_IM(tag, ...) test_log_i(__VA_ARGS__)
#define LT_HEAP_I() ((void)0)

void test_log_i(const char *fmt, ...);
void startWifiTask();
void handleRtwEvent(uint16_t event, const char *data, int len, int flags);
"""

TEST_CPP = r"""
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <string>
#include <vector>

/* Allocation accounting, installed before the real source is included so that
 * every malloc()/free() inside WiFiEvents.cpp is observed. */
int g_mallocs = 0;
int g_frees = 0;
extern "C" void *test_malloc(size_t n) {
	g_mallocs++;
	return malloc(n);
}
extern "C" void test_free(void *p) {
	g_frees++;
	free(p);
}

#define malloc test_malloc
#define free test_free
#include "WiFiEvents.cpp"
#undef malloc
#undef free

/* ---- FreeRTOS mock: captures the real rtw_event_t records ---- */
std::vector<rtw_event_t *> g_queue;
int g_queue_sends = 0;
extern "C" xQueueHandle xQueueCreate(unsigned, unsigned) {
	return (xQueueHandle)1;
}
extern "C" BaseType_t xQueueSend(xQueueHandle, const void *item, uint32_t) {
	g_queue_sends++;
	g_queue.push_back(*(rtw_event_t *const *)item);
	return pdTRUE;
}
extern "C" BaseType_t xQueueReceive(xQueueHandle, void *item, uint32_t) {
	if (g_queue.empty())
		return 0;
	*(rtw_event_t **)item = g_queue.front();
	g_queue.erase(g_queue.begin());
	return pdTRUE;
}
extern "C" BaseType_t xTaskCreate(void (*)(void *), const char *, unsigned, void *, unsigned, xTaskHandle *) {
	return pdTRUE;
}

/* ---- WiFiClass mock ---- */
WiFiClass g_wifi;
WiFiClass *pWiFi = &g_wifi;
static WiFiScanData g_scan = {7};
int g_post_calls = 0;
EventId g_last_post = ARDUINO_EVENT_MAX;
void WiFiClass::postEvent(EventId id, EventInfo) {
	g_post_calls++;
	g_last_post = id;
}
const String WiFiClass::SSID() {
	return String("test");
}
int32_t WiFiClass::channel() {
	return 1;
}
WiFiAuthMode WiFiClass::getEncryption() {
	return WIFI_AUTH_OPEN;
}
uint8_t *WiFiClass::BSSID() {
	static uint8_t b[6] = {1, 2, 3, 4, 5, 6};
	return b;
}

/* ---- SDK handler probes ---- */
static const int SCAN_REC_LEN = 62;
static uint8_t g_rx_stack[256];
static int g_inside_call = 0;
static int g_report_calls = 0;
static int g_done_calls = 0;
static int g_report_during_call = 0;
static int g_done_during_call = 0;
static void *g_seen_inner = NULL;
static uint8_t g_seen_record[SCAN_REC_LEN];
static int g_seq = 0;
static int g_report_seq = -1;
static int g_done_seq = -1;

/* Mirrors wifi_scan_each_report_hdl: dereference the holder, copy the record,
 * then clear it in place before returning. */
static void report_handler(char *buf, int len, int flags, void *ud) {
	(void)len;
	(void)flags;
	(void)ud;
	g_report_calls++;
	g_report_during_call = g_inside_call;
	g_report_seq = g_seq++;
	void *inner = *(void **)buf;
	g_seen_inner = inner;
	memcpy(g_seen_record, inner, SCAN_REC_LEN);
	memset(inner, 0, SCAN_REC_LEN);
}

static void done_handler(char *buf, int len, int flags, void *ud) {
	(void)buf;
	(void)len;
	(void)flags;
	(void)ud;
	g_done_calls++;
	g_done_during_call = g_inside_call;
	g_done_seq = g_seq++;
}

static int all_zero(const void *p, size_t n) {
	const uint8_t *b = (const uint8_t *)p;
	for (size_t i = 0; i < n; i++)
		if (b[i])
			return 0;
	return 1;
}

static void reset(void) {
	g_mallocs = g_frees = g_queue_sends = 0;
	g_queue.clear();
	g_report_calls = g_done_calls = g_post_calls = 0;
	g_report_during_call = g_done_during_call = 0;
	g_report_seq = g_done_seq = -1;
	g_seq = 0;
	g_seen_inner = NULL;
	memset(g_seen_record, 0, sizeof(g_seen_record));
}

int main(void) {
	(void)all_zero;
	wifiEventQueueHandle = (xQueueHandle)1;
	wifiEventTaskHandle = (xTaskHandle)1;
	g_wifi.scan = &g_scan;
	wifi_reg_event_handler(WIFI_EVENT_SCAN_RESULT_REPORT, report_handler, NULL);
	wifi_reg_event_handler(WIFI_EVENT_SCAN_DONE, done_handler, NULL);

	uint8_t *holder = g_rx_stack + 24; /* report_scan_result_one: holder at sp+24 */
	uint8_t *record = g_rx_stack + 40; /* record at sp+40 */
	uint8_t expected[SCAN_REC_LEN];
	for (int i = 0; i < SCAN_REC_LEN; i++)
		expected[i] = (uint8_t)(0x10 + i);
	*(void **)holder = record;

#if defined(LT_AMB1)
	/* --- scan report: synchronous, no allocation, no queue --- */
	reset();
	memset(record, 0, SCAN_REC_LEN);
	memcpy(record, expected, SCAN_REC_LEN);
	g_inside_call = 1;
	wifi_indication(WIFI_EVENT_SCAN_RESULT_REPORT, (char *)holder, SCAN_REC_LEN, 0);
	g_inside_call = 0;
	if (g_report_calls != 1)
		return 10;
	if (!g_report_during_call)
		return 11; /* handler ran inside the call */
	if (g_seen_inner != record)
		return 12; /* original pointer, not a heap copy */
	if (memcmp(g_seen_record, expected, SCAN_REC_LEN) != 0)
		return 13;
	if (!all_zero(record, SCAN_REC_LEN))
		return 14; /* cleared during the call */
	if (g_mallocs != 0 || g_queue_sends != 0)
		return 15; /* no event record / payload allocation, no queue */
	if (g_frees != 0)
		return 16;

	/* Reusing the frame afterwards must be irrelevant: already consumed. */
	memset(g_rx_stack, 0xAA, sizeof(g_rx_stack));
	if (g_report_calls != 1)
		return 17;
	/* re-arm the holder -> record link for the remaining checks */
	*(void **)holder = record;
	memset(record, 0, SCAN_REC_LEN);

	/* --- malformed scan reports are dropped, never dispatched --- */
	reset();
	wifi_indication(WIFI_EVENT_SCAN_RESULT_REPORT, NULL, 0, 0);
	wifi_indication(WIFI_EVENT_SCAN_RESULT_REPORT, (char *)holder, 0, 0);
	if (g_report_calls != 0 || g_queue_sends != 0 || g_mallocs != 0)
		return 18;

	/* --- scan done: synchronous SDK handler + synchronous Arduino event --- */
	reset();
	g_inside_call = 1;
	wifi_indication(WIFI_EVENT_SCAN_DONE, NULL, 0, 0);
	g_inside_call = 0;
	if (g_done_calls != 1)
		return 20;
	if (!g_done_during_call)
		return 21;
	if (g_post_calls != 1 || g_last_post != ARDUINO_EVENT_WIFI_SCAN_DONE)
		return 22;
	if (g_queue_sends != 0 || g_mallocs != 0)
		return 23;

	/* --- ordering: report handlers run before the done handler --- */
	reset();
	g_inside_call = 1;
	wifi_indication(WIFI_EVENT_SCAN_RESULT_REPORT, (char *)holder, SCAN_REC_LEN, 0);
	wifi_indication(WIFI_EVENT_SCAN_RESULT_REPORT, (char *)holder, SCAN_REC_LEN, 0);
	wifi_indication(WIFI_EVENT_SCAN_DONE, NULL, 0, 0);
	g_inside_call = 0;
	if (g_report_calls != 2 || g_done_calls != 1)
		return 30;
	if (!(g_report_seq < g_done_seq))
		return 31;

	/* --- non-scan and Arduino events still go through the queue --- */
	reset();
	wifi_indication(WIFI_EVENT_DISCONNECT, NULL, 0, 0);
	wifi_indication(WIFI_EVENT_CONNECT, NULL, ARDUINO_EVENT_WIFI_STA_START, -2);
	if (g_queue_sends != 2)
		return 40;
	if (g_report_calls != 0 || g_done_calls != 0)
		return 41;

	puts("amb1 scan lifetime: synchronous dispatch OK");
#else
	/* --- non-AMB1 keeps the deferred queue behaviour --- */
	reset();
	memset(record, 0, SCAN_REC_LEN);
	memcpy(record, expected, SCAN_REC_LEN);
	g_inside_call = 1;
	wifi_indication(WIFI_EVENT_SCAN_RESULT_REPORT, (char *)holder, SCAN_REC_LEN, 0);
	g_inside_call = 0;
	if (g_report_calls != 0)
		return 50; /* deferred, not synchronous */
	if (g_queue_sends != 1)
		return 51; /* queued */
	if (g_mallocs != 2)
		return 52; /* event record + payload copy */

	/* The queued entry still points at the receive-task stack. Once the frame is
	 * reused the deferred handler reads garbage and memsets it - the exact
	 * hazard the AMB1 synchronous path avoids. */
	rtw_event_t *ev = g_queue.front();
	memset(g_rx_stack, 0xAA, sizeof(g_rx_stack));
	g_report_calls = 0;
	handleRtwEvent(ev->event, ev->buf, ev->buf_len, ev->flags);
	if (g_report_calls != 1)
		return 53;
	if (g_seen_record[0] != 0xAA)
		return 54; /* stale frame observed */

	/* Scan done is likewise queued, not dispatched synchronously. */
	reset();
	wifi_indication(WIFI_EVENT_SCAN_DONE, NULL, 0, 0);
	if (g_done_calls != 0 || g_queue_sends != 1)
		return 55;
	if (g_post_calls != 0)
		return 56;

	puts("non-amb1 scan lifetime: deferred queue behaviour preserved");
#endif
	return 0;
}
"""


def _compile_and_run(work: pathlib.Path, amb1: bool) -> subprocess.CompletedProcess:
    work.mkdir(parents=True, exist_ok=True)
    (work / "WiFiPrivate.h").write_text(MOCK_WIFI_PRIVATE)
    # Copy the real source so its quoted #include "WiFiPrivate.h" resolves to the
    # mock sitting next to it, and so the test compiles the shipped file verbatim.
    (work / "WiFiEvents.cpp").write_text(WIFI_EVENTS.read_text())
    (work / "test.cpp").write_text(TEST_CPP)
    binary = work / "scan_lifetime"
    cmd = [
        "c++",
        "-std=c++17",
        "-Wall",
        "-Wextra",
        "-Wno-unused-parameter",
        "-I",
        str(work),
    ]
    if amb1:
        cmd.append("-DLT_AMB1=1")
    cmd += [str(work / "test.cpp"), "-o", str(binary)]
    subprocess.run(cmd, check=True, cwd=work)
    return subprocess.run([str(binary)], check=False, capture_output=True, text=True)


class ScanLifetimeTest(unittest.TestCase):
    def test_amb1_dispatches_scan_events_synchronously(self):
        with tempfile.TemporaryDirectory(prefix="amb1-scan-") as d:
            result = _compile_and_run(pathlib.Path(d) / "amb1", amb1=True)
            self.assertEqual(
                result.returncode,
                0,
                f"AMB1 scan-lifetime harness failed (rc={result.returncode})\n"
                f"stdout:\n{result.stdout}\nstderr:\n{result.stderr}",
            )
            self.assertIn("synchronous dispatch OK", result.stdout)

    def test_non_amb1_keeps_the_deferred_queue_behaviour(self):
        with tempfile.TemporaryDirectory(prefix="amb1-scan-") as d:
            result = _compile_and_run(pathlib.Path(d) / "other", amb1=False)
            self.assertEqual(
                result.returncode,
                0,
                f"non-AMB1 scan-lifetime harness failed (rc={result.returncode})\n"
                f"stdout:\n{result.stdout}\nstderr:\n{result.stderr}",
            )
            self.assertIn("deferred queue behaviour preserved", result.stdout)


if __name__ == "__main__":
    unittest.main(verbosity=2)
