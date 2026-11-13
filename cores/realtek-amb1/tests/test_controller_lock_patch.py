"""Apply the lock extension to real SDK sources and exercise a host mock.

Run: python3 libretiny/cores/realtek-amb1/tests/test_controller_lock_patch.py -v
AMB1_SDK_ROOT may select another installed framework-realtek-amb1 package.
Only temporary copies are patched; this does not qualify hardware exclusion.
"""

import os
from pathlib import Path
import shutil
import subprocess
import tempfile
import unittest


PATCH = Path(__file__).resolve().parents[1] / "base/ota/image2_controller_lock.patch"
SDK = Path(os.environ.get(
    "AMB1_SDK_ROOT",
    str(Path(os.environ.get("PLATFORMIO_CORE_DIR", "~/.platformio"))
        / "packages/framework-realtek-amb1"),
)).expanduser()
APPROVED_TMP = Path(
    "/private/var/folders/4q/bvwkg_397v102xfwcpxgdymc0000gn/T/opencode"
)
SOURCES = (
    "component/os/os_dep/device_lock.c",
    "component/os/os_dep/include/device_lock.h",
)

OSDEP = r"""
#ifndef MOCK_OSDEP_H
#define MOCK_OSDEP_H
#include <stddef.h>
#include <stdio.h>
typedef unsigned int u32;
typedef struct mock_mutex { int held; int owner; } * _mutex;
typedef _mutex _lock;
typedef unsigned int _irqL;
void rtw_mutex_init(_mutex *);
void rtw_mutex_free(_mutex *);
void rtw_enter_critical(_lock *, _irqL *);
void rtw_exit_critical(_lock *, _irqL *);
int rtw_mutex_get_timeout(_mutex *, u32);
void rtw_mutex_put(_mutex *);
#endif
"""

HARNESS = r"""
#include <assert.h>
#include <limits.h>
#include <string.h>
#include "osdep_service.h"
#include "device_lock.h"
#include "FreeRTOS.h"
#include "semphr.h"

/* Include the patched real source to inspect its private initialization bit. */
#include "component/os/os_dep/device_lock.c"

static struct mock_mutex pool[RT_DEV_LOCK_MAX];
static int allocations, allocated, allocation_fails, takes, releases, logs;
static int critical_depth, critical_entries, task = 1;
static unsigned int last_ticks;
static _mutex last_take, last_put;

void rtw_mutex_init(_mutex *m) {
    assert(critical_depth == 1);
    ++allocations;
    if (allocation_fails) { *m = NULL; return; }
    assert(allocated < RT_DEV_LOCK_MAX);
    *m = &pool[allocated++];
}
void rtw_mutex_free(_mutex *m) { (void)m; assert(0); }
void rtw_enter_critical(_lock *m, _irqL *irq) {
    (void)m; (void)irq;
    assert(critical_depth == 0);
    ++critical_depth; ++critical_entries;
}
void rtw_exit_critical(_lock *m, _irqL *irq) {
    (void)m; (void)irq;
    assert(critical_depth == 1);
    --critical_depth;
}
int xSemaphoreTake(_mutex m, unsigned int ticks) {
    assert(m && critical_depth == 0);
    ++takes; last_ticks = ticks; last_take = m;
    if (m->held) return pdFALSE;
    m->held = 1; m->owner = task;
    return pdTRUE;
}
int rtw_mutex_get_timeout(_mutex *m, u32 ticks) {
    return xSemaphoreTake(*m, ticks) == pdTRUE ? 0 : -1;
}
void rtw_mutex_put(_mutex *m) {
    assert(*m && (*m)->held && (*m)->owner == task);
    ++releases; last_put = *m;
    (*m)->held = 0; (*m)->owner = 0;
}
int mock_printf(const char *fmt, ...) { (void)fmt; ++logs; return 0; }

static void try_once(RT_DEV_LOCK_E device, int expected, int native_calls) {
    int before_alloc = allocations, before_take = takes;
    int before_put = releases, before_log = logs, before_critical = critical_entries;
    assert(device_mutex_try_lock(device) == expected);
    assert(allocations == before_alloc && releases == before_put);
    assert(logs == before_log && critical_entries == before_critical);
    assert(takes == before_take + native_calls);
    if (native_calls) {
        assert(last_ticks == 0);
        assert(last_take == device_mutex[device]);
    }
}

static void unprepared_and_invalid(void) {
    RT_DEV_LOCK_E invalid[] = {
        (RT_DEV_LOCK_E)-1, RT_DEV_LOCK_MAX, (RT_DEV_LOCK_E)INT_MAX
    };
    for (unsigned int i = 0; i < sizeof(invalid) / sizeof(invalid[0]); ++i) {
        assert(device_mutex_prepare(invalid[i]) == 0);
        try_once(invalid[i], 0, 0);
    }
    for (int i = 0; i < RT_DEV_LOCK_MAX; ++i)
        try_once((RT_DEV_LOCK_E)i, 0, 0);
    assert(!mutex_init && !allocations && !critical_entries && !logs);
}

static void prepare_failure_and_recovery(void) {
    allocation_fails = 1;
    assert(device_mutex_prepare(RT_DEV_LOCK_FLASH) == 0);
    assert(!DEVICE_MUTEX_IS_INIT(RT_DEV_LOCK_FLASH));
    assert(device_mutex[RT_DEV_LOCK_FLASH] == NULL);
    assert(allocations == 1 && !allocated && !critical_depth);
    try_once(RT_DEV_LOCK_FLASH, 0, 0);
    assert(device_mutex_prepare(RT_DEV_LOCK_FLASH) == 0);
    assert(allocations == 2 && !DEVICE_MUTEX_IS_INIT(RT_DEV_LOCK_FLASH));
    allocation_fails = 0;
    assert(device_mutex_prepare(RT_DEV_LOCK_FLASH) == 1);
    _mutex shared = device_mutex[RT_DEV_LOCK_FLASH];
    assert(shared && DEVICE_MUTEX_IS_INIT(RT_DEV_LOCK_FLASH));
    assert(device_mutex_prepare(RT_DEV_LOCK_FLASH) == 1);
    assert(allocations == 3 && allocated == 1 && !takes && !releases);
    assert(device_mutex[RT_DEV_LOCK_FLASH] == shared);
    try_once(RT_DEV_LOCK_FLASH, 1, 1);
    device_mutex_unlock(RT_DEV_LOCK_FLASH);
    assert(last_put == shared && releases == 1 && !logs);
}

static void sdk_holder_contention(void) {
    /* The existing SDK API may create the shared lock before startup prepare. */
    device_mutex_lock(RT_DEV_LOCK_FLASH);
    _mutex shared = last_take;
    assert(shared == device_mutex[RT_DEV_LOCK_FLASH] && last_ticks == 10000);
    assert(device_mutex_prepare(RT_DEV_LOCK_FLASH) == 1 && allocations == 1);
    task = 2;
    for (int i = 0; i < 3; ++i) {
        try_once(RT_DEV_LOCK_FLASH, 0, 1);
        assert(shared->held && shared->owner == 1);
    }
    assert(!releases);
    task = 1;
    device_mutex_unlock(RT_DEV_LOCK_FLASH);
    assert(last_put == shared && !shared->held);
    task = 2;
    try_once(RT_DEV_LOCK_FLASH, 1, 1);
    device_mutex_unlock(RT_DEV_LOCK_FLASH);
    assert(last_put == shared && allocations == 1 && releases == 2);
}

static void try_holder_and_other_sdk_lock(void) {
    assert(device_mutex_prepare(RT_DEV_LOCK_FLASH) == 1);
    assert(device_mutex_prepare(RT_DEV_LOCK_CRYPTO) == 1);
    _mutex shared = device_mutex[RT_DEV_LOCK_FLASH];
    _mutex other = device_mutex[RT_DEV_LOCK_CRYPTO];
    assert(shared != other);
    try_once(RT_DEV_LOCK_FLASH, 1, 1);
    try_once(RT_DEV_LOCK_FLASH, 0, 1); /* No recursive acquisition. */
    task = 2;
    try_once(RT_DEV_LOCK_FLASH, 0, 1);
    /* Probe the ordinary SDK primitive without entering its blocking retry loop. */
    assert(rtw_mutex_get_timeout(&device_mutex[RT_DEV_LOCK_FLASH], 0) == -1);
    assert(shared->owner == 1 && !releases);
    device_mutex_lock(RT_DEV_LOCK_CRYPTO);
    assert(last_take == other && last_ticks == 10000);
    device_mutex_unlock(RT_DEV_LOCK_CRYPTO);
    assert(last_put == other && shared->held && shared->owner == 1);
    try_once(RT_DEV_LOCK_FLASH, 0, 1);
    task = 1;
    device_mutex_unlock(RT_DEV_LOCK_FLASH);
    assert(last_put == shared && !shared->held);
    task = 2;
    device_mutex_lock(RT_DEV_LOCK_FLASH);
    assert(last_take == shared && shared->owner == 2 && last_ticks == 10000);
    device_mutex_unlock(RT_DEV_LOCK_FLASH);
    assert(last_put == shared && allocations == 2 && releases == 3);
}

int main(int argc, char **argv) {
    assert(argc == 2);
    if (!strcmp(argv[1], "invalid")) unprepared_and_invalid();
    else if (!strcmp(argv[1], "allocation")) prepare_failure_and_recovery();
    else if (!strcmp(argv[1], "sdk-holder")) sdk_holder_contention();
    else if (!strcmp(argv[1], "try-holder")) try_holder_and_other_sdk_lock();
    else assert(0);
    assert(!logs && !critical_depth);
    return 0;
}
"""


class ControllerLockPatchTest(unittest.TestCase):
    @classmethod
    def setUpClass(cls):
        if not all((SDK / source).is_file() for source in SOURCES):
            raise unittest.SkipTest("AMB1 SDK unavailable; set AMB1_SDK_ROOT")
        compiler = shutil.which("cc")
        patch = shutil.which("patch")
        if not compiler or not patch:
            raise unittest.SkipTest("host cc and patch are required")
        cls.temp = tempfile.TemporaryDirectory(
            prefix="amb1-controller-lock-",
            dir=APPROVED_TMP if APPROVED_TMP.is_dir() else None,
        )
        cls.addClassCleanup(cls.temp.cleanup)
        work = Path(cls.temp.name)
        originals = {source: (SDK / source).read_bytes() for source in SOURCES}
        for source in SOURCES:
            target = work / source
            target.parent.mkdir(parents=True, exist_ok=True)
            # The SDK header uses CRLF; patch matching needs normalized copies.
            target.write_text(originals[source].decode().replace("\r\n", "\n"))
        result = subprocess.run(
            [patch, "--batch", "--fuzz=0", "-p1", "-i", str(PATCH)],
            cwd=work, text=True, capture_output=True, timeout=30,
        )
        if result.returncode:
            raise AssertionError("SDK patch does not apply:\n" + result.stdout + result.stderr)
        for source in SOURCES:
            if (SDK / source).read_bytes() != originals[source]:
                raise AssertionError("Installed SDK changed: " + source)
        (work / "osdep_service.h").write_text(OSDEP)
        (work / "FreeRTOS.h").write_text("#define pdTRUE 1\n#define pdFALSE 0\n")
        (work / "semphr.h").write_text(
            '#include "osdep_service.h"\nint xSemaphoreTake(_mutex, unsigned int);\n'
        )
        (work / "harness.c").write_text(HARNESS)
        cls.executable = work / "controller_lock_test"
        result = subprocess.run(
            [compiler, "-std=c99", "-Wall", "-Wextra", "-Werror",
             "-Wno-unused-function", "-Dprintf=mock_printf", "-I", str(work),
             "-I", str(work / "component/os/os_dep/include"),
             str(work / "harness.c"), "-o", str(cls.executable)],
            text=True, capture_output=True, timeout=30,
        )
        if result.returncode:
            raise AssertionError("Patched SDK compile failed:\n" + result.stdout + result.stderr)

    def run_scenario(self, scenario):
        result = subprocess.run(
            [str(self.executable), scenario], text=True, capture_output=True, timeout=5,
        )
        self.assertEqual(result.returncode, 0, result.stdout + result.stderr)

    def test_invalid_enums_and_unprepared_try_do_not_allocate(self):
        self.run_scenario("invalid")

    def test_prepare_exact_one_and_allocation_failure_not_initialized(self):
        self.run_scenario("allocation")

    def test_existing_sdk_holder_excludes_zero_wait_try(self):
        self.run_scenario("sdk-holder")

    def test_try_holder_shares_sdk_unlock_and_keeps_other_lock_independent(self):
        self.run_scenario("try-holder")


if __name__ == "__main__":
    unittest.main()
