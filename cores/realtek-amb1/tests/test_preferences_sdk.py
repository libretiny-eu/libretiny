"""Host-only tests of the actual KVS bridge with SDK stubs; no hardware I/O."""

import json
import shutil
import subprocess
import tempfile
import unittest
from pathlib import Path

ROOT = Path(__file__).resolve().parents[3]
PORT = ROOT / "cores/realtek-amb1/base/port"
STUB = r"""
#ifndef SDK_STUB_H
#define SDK_STUB_H
#include <stdint.h>
#include <stddef.h>
typedef struct { int unused; } flash_t;
typedef void *TaskHandle_t;
#define taskSCHEDULER_NOT_STARTED 0
#define taskSCHEDULER_SUSPENDED 1
#define taskSCHEDULER_RUNNING 2
int xTaskGetSchedulerState(void);
#define RT_DEV_LOCK_FLASH 2
unsigned __get_IPSR(void);
TaskHandle_t xTaskGetCurrentTaskHandle(void);
int device_mutex_try_lock(int);
void device_mutex_unlock(int);
int flash_stream_read(flash_t *, uint32_t, uint32_t, uint8_t *);
#endif
"""
HARNESS = r"""
#include <assert.h>
#include <limits.h>
#include <stdlib.h>
#include <string.h>
#include "preferences_sdk.h"
#if LT_AMB1_EXPERIMENTAL_KVS
#include "sdk_stub.h"
static unsigned char memory[8192];
static int held, locks, unlocks, writes, reads, erases, attempted;
static int ready = 1, prepared = 1, fail_lock, fail_read, fail_write, bad_erase, bad_write;
static int scheduler = taskSCHEDULER_RUNNING;
int xTaskGetSchedulerState(void) { return scheduler; }
unsigned __get_IPSR(void) { return ready ? 0 : 1; }
TaskHandle_t xTaskGetCurrentTaskHandle(void) { return ready ? (void *)1 : NULL; }
int device_mutex_try_lock(int id) {
    assert(id == RT_DEV_LOCK_FLASH && !held); ++attempted;
    if (!prepared || fail_lock) return 0;
    held = 1; ++locks; return 1;
}
void device_mutex_unlock(int id) {
    assert(id == RT_DEV_LOCK_FLASH && held); held = 0; ++unlocks;
}
static unsigned char *extent(uint32_t a, uint32_t n) {
    assert(held && n && a >= AMB1_PREF_KVS_START && a < AMB1_PREF_KVS_END);
    assert(n <= AMB1_PREF_KVS_END-a);
    return memory + a - AMB1_PREF_KVS_START;
}
int flash_stream_read(flash_t *o, uint32_t a, uint32_t n, uint8_t *p) {
    (void)o; unsigned char *s = extent(a,n); ++reads;
    if (fail_read) return 0;
    memcpy(p,s,n); return 1;
}
int __real_flash_burst_write(flash_t *o, uint32_t a, uint32_t n, uint8_t *p) {
    (void)o; unsigned char *s = extent(a,n); ++writes;
    assert(n <= 256 && n <= 256-a%256);
    assert((uintptr_t)p % sizeof(uint32_t) == 0);
    if (fail_write) return 0;
    for (uint32_t i=0;i<n;++i) { assert((s[i]&p[i]) == p[i]); s[i] &= p[i]; }
    if (bad_write) s[0] ^= 1;
    return 1;
}
void __real_flash_erase_sector(flash_t *o, uint32_t a) {
    (void)o; unsigned char *s = extent(a,4096); ++erases;
    assert(a%4096 == 0); memset(s,255,4096);
    if (bad_erase) s[4095] = 0;
}
static amb1_preferences_sdk_policy_t policy(void) {
    amb1_preferences_sdk_policy_t p = {0};
    p.flash.review = (amb1_preferences_review_t){1,1,1,1,1,1};
    memset(p.flash.expected_snapshot_hash,0x42,32);
    memset(p.flash.fresh_snapshot_hash,0x42,32);
    p.flash.fresh_snapshot_attested = 1;
    p.flash.shared_controller_lock_attested = 1;
    p.runtime_qualification_passed = 1;
    p.current_geometry = (amb1_preferences_sdk_geometry_t){
        0x800000,4096,256,0xb000,0x10b000,0x177000,0x275000,
        0x275000,0x277000,0x2e3000,0x800000,0xfe000
    };
    return p;
}
static void balanced(void) { assert(!held && locks == unlocks); }
#endif
int main(int argc, char **argv) {
    unsigned char b[300], out[300];
    (void)argc; (void)argv; (void)out;
    memset(b,0xaa,sizeof(b));
    assert(amb1_preferences_sdk_read(0x275000,b,1) == -1);
    assert(amb1_preferences_sdk_write(0x275000,b,1) == -1);
    assert(amb1_preferences_sdk_erase(0x275000,4096) == -1);
    assert(!amb1_preferences_sdk_format_allowed());
    assert(amb1_preferences_sdk_bind(NULL) == -1);
#if LT_AMB1_EXPERIMENTAL_KVS
    int mode = argc > 1 ? atoi(argv[1]) : 0;
    amb1_preferences_sdk_policy_t p = policy(), q;
    memset(memory,255,sizeof(memory));
    q=p; q.runtime_qualification_passed=0; assert(amb1_preferences_sdk_bind(&q)==-1);
    q=p; q.runtime_qualification_passed=2; assert(amb1_preferences_sdk_bind(&q)==-1);
    q=p; q.current_geometry.ota2_end++; assert(amb1_preferences_sdk_bind(&q)==-1);
    q=p; q.current_geometry.image_max++; assert(amb1_preferences_sdk_bind(&q)==-1);
    q=p; q.current_geometry.kvs_start--; assert(amb1_preferences_sdk_bind(&q)==-1);
    q=p; q.flash.review.reviewed_reservation=0; assert(amb1_preferences_sdk_bind(&q)==-1);
    q=p; q.flash.initial_format_approved=2; assert(amb1_preferences_sdk_bind(&q)==-1);
    q=p; q.flash.fresh_snapshot_hash[31]^=1; assert(amb1_preferences_sdk_bind(&q)==-1);
    q=p; q.flash.fresh_snapshot_attested=0; assert(amb1_preferences_sdk_bind(&q)==-1);
    q=p; q.flash.shared_controller_lock_attested=0; assert(amb1_preferences_sdk_bind(&q)==-1);
    assert(!attempted && !reads && !writes && !erases);
    ready=0; assert(amb1_preferences_sdk_bind(&p)==-1); ready=1;
    for (scheduler=taskSCHEDULER_NOT_STARTED; scheduler<taskSCHEDULER_RUNNING; ++scheduler) {
        assert(xTaskGetCurrentTaskHandle() != NULL);
        assert(amb1_preferences_sdk_bind(&p)==-1);
        assert(!attempted && !reads && !writes && !erases);
    }
    prepared=0; assert(amb1_preferences_sdk_bind(&p)==-1); prepared=1;
    p.flash.initial_format_approved = mode == 1;
    p.flash.existing_flashdb_attested = mode == 2;
    assert(!amb1_preferences_sdk_bind(&p));
    assert(amb1_preferences_sdk_format_allowed() == (mode==1));
    assert(!reads && !writes && !erases); /* binding cannot touch flash */
    assert(amb1_preferences_sdk_bind(&p)==-1); /* no live rebind */
    int before = attempted;
    for (scheduler=taskSCHEDULER_NOT_STARTED; scheduler<taskSCHEDULER_RUNNING; ++scheduler) {
        assert(xTaskGetCurrentTaskHandle() != NULL);
        assert(amb1_preferences_sdk_read(0x275000,b,1)==-1);
        assert(amb1_preferences_sdk_write(0x275000,b,1)==-1);
        assert(amb1_preferences_sdk_erase(0x275000,4096)==-1);
        assert(attempted == before && !reads && !writes && !erases);
    }
    const long invalid[] = {-1, 0, 0x274fff, 0x277000, LONG_MAX};
    for (unsigned i=0;i<sizeof(invalid)/sizeof(invalid[0]);++i) {
        assert(amb1_preferences_sdk_read(invalid[i],b,1)==-1);
        assert(amb1_preferences_sdk_write(invalid[i],b,1)==-1);
        assert(amb1_preferences_sdk_erase(invalid[i],4096)==-1);
    }
    assert(amb1_preferences_sdk_read(0x276fff,b,2)==-1);
    assert(amb1_preferences_sdk_write(0x276fff,b,2)==-1);
    assert(amb1_preferences_sdk_read(0x275000,b,(size_t)-1)==-1);
    assert(amb1_preferences_sdk_read(0x275000,b,0)==-1);
    assert(amb1_preferences_sdk_write(0x275000,b,0)==-1);
    assert(amb1_preferences_sdk_read(0x275000,NULL,1)==-1);
    assert(amb1_preferences_sdk_write(0x275000,NULL,1)==-1);
    assert(amb1_preferences_sdk_erase(0x275001,4096)==-1);
    assert(amb1_preferences_sdk_erase(0x276000,8192)==-1);
    assert(attempted == before);
    assert(amb1_preferences_sdk_read(0x276fff,b,1)==1);
    fail_lock=1;
    before=reads;
    assert(amb1_preferences_sdk_read(0x275000,b,1)==-1);
    assert(amb1_preferences_sdk_write(0x275000,b,1)==-1);
    assert(amb1_preferences_sdk_erase(0x275000,4096)==-1);
    assert(reads==before && !writes && !erases); fail_lock=0;
    fail_read=1; assert(amb1_preferences_sdk_read(0x275000,b,1)==-1); fail_read=0;
    if (!mode) {
        assert(amb1_preferences_sdk_write(0x275000,b,1)==-1);
        assert(amb1_preferences_sdk_erase(0x275000,4096)==-1);
        assert(!writes && !erases); balanced(); return 0;
    }
    memset(b,0xaa,sizeof(b));
    before=locks;
    assert(amb1_preferences_sdk_write(0x2750fa,b,sizeof(b))==sizeof(b));
    /* Bridge chunks 128/128/44; SDK splits first and last at page edges. */
    assert(writes==5 && locks==before+1);
    assert(!memcmp(memory+0xfa,b,sizeof(b)));
    assert(memory[0xf9]==255 && memory[0xfa+sizeof(b)]==255);
    assert(amb1_preferences_sdk_read(0x2750fa,out,sizeof(out))==sizeof(out));
    assert(!memcmp(b,out,sizeof(b)));
    memset(b,255,sizeof(b)); before=writes;
    assert(amb1_preferences_sdk_write(0x2750fa,b,sizeof(b))==-1 && writes==before);
    fail_write=1; assert(amb1_preferences_sdk_write(0x275800,b,1)==-1); fail_write=0;
    bad_write=1; assert(amb1_preferences_sdk_write(0x275800,b,1)==-1); bad_write=0;
    before=locks;
    assert(amb1_preferences_sdk_erase(0x275000,8192)==8192);
    assert(erases==2 && locks==before+1);
    for (unsigned i=0;i<sizeof(memory);++i) assert(memory[i]==255);
    bad_erase=1; assert(amb1_preferences_sdk_erase(0x276000,4096)==-1); bad_erase=0;
    fail_read=1; assert(amb1_preferences_sdk_erase(0x276000,4096)==-1); fail_read=0;
    balanced();
#else
    amb1_preferences_sdk_policy_t p = {0};
    assert(amb1_preferences_sdk_bind(&p)==-1);
#endif
    return 0;
}
"""


class PreferencesSDKTests(unittest.TestCase):
    def test_actual_bridge_default_off_and_enabled(self):
        cc = shutil.which("cc")
        if not cc:
            self.skipTest("C compiler unavailable")
        with tempfile.TemporaryDirectory(prefix="preferences-sdk-") as temp:
            temp = Path(temp)
            (temp / "sdk_stub.h").write_text(STUB)
            for name in (
                "ameba_soc.h",
                "flash_api.h",
                "device_lock.h",
                "FreeRTOS.h",
                "task.h",
            ):
                (temp / name).write_text('#include "sdk_stub.h"\n')
            (temp / "main.c").write_text(HARNESS)
            for enabled in (False, True):
                output = temp / ("enabled" if enabled else "default")
                command = [
                    cc,
                    "-std=c99",
                    "-Wall",
                    "-Wextra",
                    "-Werror",
                    "-I",
                    str(PORT),
                    "-I",
                    str(temp),
                ]
                if enabled:
                    command += ["-DLT_AMB1_EXPERIMENTAL_KVS=1"]
                command += [
                    str(PORT / "preferences_sdk.c"),
                    str(temp / "main.c"),
                    "-o",
                    str(output),
                ]
                # Default-off must link without even the bridge or SDK symbols.
                if enabled:
                    command += [str(PORT / "preferences_flash.c")]
                subprocess.run(command, check=True, capture_output=True, text=True)
                for mode in (0, 1, 2) if enabled else (0,):
                    subprocess.run(
                        [str(output), str(mode)],
                        check=True,
                        capture_output=True,
                        text=True,
                    )

    def test_reserved_board_geometry(self):
        board = json.loads((ROOT / "boards/wcata007-rtl8711am-8mb.json").read_text())
        self.assertEqual(board["build"]["variant"], "generic-rtl8711am-8mb")
        self.assertEqual(board["upload"]["maximum_size"], 0xFE000)
        expected = {
            "boot": (0, 0xB000),
            "ota1": (0xB000, 0x10B000),
            "ota2": (0x177000, 0x275000),
            "kvs": (0x275000, 0x277000),
            "system": (0x2E3000, 0x800000),
        }
        for name, extent in expected.items():
            start, length = (int(x, 16) for x in board["flash"][name].split("+"))
            self.assertEqual((start, start + length), extent)
        ranges = sorted(expected.values())
        self.assertTrue(all(a[1] <= b[0] for a, b in zip(ranges, ranges[1:])))
        self.assertNotIn("flags", board["build"])


if __name__ == "__main__":
    unittest.main()
