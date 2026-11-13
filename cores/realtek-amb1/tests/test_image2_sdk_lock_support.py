"""Compile the actual SDK device_lock.c after applying the unapplied extension.

Run: python3 -m unittest discover -s libretiny/cores/realtek-amb1/tests -p test_image2_sdk_lock_support.py -v
AMB1_SDK_ROOT overrides the installed package location. No SDK files are edited.
These are source-contract tests, not FreeRTOS scheduling or hardware qualification.
"""

import os
from pathlib import Path
import shutil
import subprocess
import tempfile
import unittest


ROOT = Path(__file__).resolve().parents[3]
PATCH = ROOT / "cores/realtek-amb1/base/ota/image2_controller_lock.patch"
SDK = Path(os.environ.get(
    "AMB1_SDK_ROOT",
    str(Path(os.environ.get("PLATFORMIO_CORE_DIR", "~/.platformio"))
        / "packages/framework-realtek-amb1"),
)).expanduser()

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
int rtw_mutex_get_timeout(_lock *, u32);
void rtw_mutex_put(_lock *);
#endif
"""

HARNESS = r"""
#include <assert.h>
#include <stdarg.h>
#include "osdep_service.h"
#include "device_lock.h"
#include "FreeRTOS.h"
#include "semphr.h"
static struct mock_mutex pool[RT_DEV_LOCK_MAX];
static int allocations, alloc_fail, takes, put_count, logs, critical, task = 1;
static unsigned int ticks;
static _mutex last_take, last_put;
void rtw_mutex_init(_mutex *m) {
    ++allocations;
    *m = alloc_fail ? NULL : &pool[allocations - 1];
}
void rtw_mutex_free(_mutex *m) { (void)m; assert(0); }
void rtw_enter_critical(_lock *m, _irqL *irq) {
    (void)m; (void)irq; ++critical;
}
void rtw_exit_critical(_lock *m, _irqL *irq) {
    (void)m; (void)irq; --critical;
}
int xSemaphoreTake(_mutex m, unsigned int timeout) {
    assert(m && !critical);
    ++takes; ticks = timeout; last_take = m;
    if (m->held) return pdFALSE;
    m->held = 1; m->owner = task;
    return pdTRUE;
}
int rtw_mutex_get_timeout(_lock *m, u32 timeout) {
    return xSemaphoreTake(*m, timeout) == pdTRUE ? 0 : -1;
}
void rtw_mutex_put(_lock *m) {
    assert((*m)->held && (*m)->owner == task);
    ++put_count; last_put = *m; (*m)->held = 0;
}
int mock_printf(const char *fmt, ...) { (void)fmt; ++logs; return 0; }

int main(int argc, char **argv) {
    (void)argv;
    assert(argc == 1);
    assert(device_mutex_try_lock(RT_DEV_LOCK_FLASH) == 0);
    assert(device_mutex_try_lock((RT_DEV_LOCK_E)-1) == 0);
    assert(device_mutex_try_lock(RT_DEV_LOCK_MAX) == 0);
    assert(device_mutex_prepare((RT_DEV_LOCK_E)-1) == 0);
    assert(device_mutex_prepare(RT_DEV_LOCK_MAX) == 0);
    assert(!allocations && !takes && !logs && !put_count);

    alloc_fail = 1;
    assert(device_mutex_prepare(RT_DEV_LOCK_FLASH) == 0);
    assert(device_mutex_try_lock(RT_DEV_LOCK_FLASH) == 0);
    assert(allocations == 1 && !takes && !critical);
    alloc_fail = 0;
    assert(device_mutex_prepare(RT_DEV_LOCK_FLASH) == 1);
    assert(device_mutex_prepare(RT_DEV_LOCK_FLASH) == 1);
    assert(allocations == 2);

    /* A normal SDK holder excludes try-lock, with zero-tick failure. */
    device_mutex_lock(RT_DEV_LOCK_FLASH);
    _mutex shared = last_take;
    assert(ticks == 10000);
    task = 2;
    assert(device_mutex_try_lock(RT_DEV_LOCK_FLASH) == 0);
    assert(last_take == shared && ticks == 0 && !put_count && !logs);
    task = 1;
    device_mutex_unlock(RT_DEV_LOCK_FLASH);
    assert(put_count == 1 && last_put == shared);

    assert(device_mutex_try_lock(RT_DEV_LOCK_FLASH) == 1);
    assert(last_take == shared && ticks == 0);
    /* Native nonrecursive mutex: even the same task cannot reenter. */
    assert(device_mutex_try_lock(RT_DEV_LOCK_FLASH) == 0);
    assert(shared->held && shared->owner == 1 && put_count == 1);
    task = 2;
    /* Probe the same primitive used by ordinary blocking SDK callers. */
    assert(rtw_mutex_get_timeout(&shared, 0) == -1);
    assert(device_mutex_try_lock(RT_DEV_LOCK_FLASH) == 0);
    assert(shared->owner == 1 && put_count == 1);
    task = 1;
    device_mutex_unlock(RT_DEV_LOCK_FLASH);
    assert(put_count == 2 && !shared->held && last_put == shared);
    task = 2;
    device_mutex_lock(RT_DEV_LOCK_FLASH);
    assert(last_take == shared && shared->owner == 2);
    device_mutex_unlock(RT_DEV_LOCK_FLASH);
    assert(put_count == 3 && allocations == 2 && !logs && !critical);
    return 0;
}
"""


class SharedSDKLockTest(unittest.TestCase):
    def test_patched_sdk_shared_zero_wait_primitive(self):
        if not (SDK / "component/os/os_dep/device_lock.c").is_file():
            self.skipTest("installed AMB1 SDK unavailable; set AMB1_SDK_ROOT")
        compiler = shutil.which("cc")
        if not compiler or not shutil.which("patch"):
            self.skipTest("host cc and patch required")
        with tempfile.TemporaryDirectory(prefix="amb1-lock-") as directory:
            work = Path(directory)
            for relative in ("component/os/os_dep/device_lock.c",
                             "component/os/os_dep/include/device_lock.h"):
                target = work / relative
                target.parent.mkdir(parents=True, exist_ok=True)
                # Normalize CRLF because the installed public header uses it.
                target.write_text((SDK / relative).read_text())
            result = subprocess.run(
                ["patch", "--batch", "--fuzz=0", "-p1", "-i", str(PATCH)],
                cwd=work, text=True, capture_output=True,
            )
            self.assertEqual(result.returncode, 0, result.stdout + result.stderr)
            (work / "osdep_service.h").write_text(OSDEP)
            (work / "FreeRTOS.h").write_text("#define pdTRUE 1\n#define pdFALSE 0\n")
            (work / "semphr.h").write_text(
                '#include "osdep_service.h"\nint xSemaphoreTake(_mutex, unsigned int);\n'
            )
            (work / "harness.c").write_text(HARNESS)
            executable = work / "test_lock"
            result = subprocess.run([
                compiler, "-std=c99", "-Wall", "-Wextra", "-Werror",
                "-Wno-unused-function", "-Dprintf=mock_printf", "-I", str(work),
                "-I", str(work / "component/os/os_dep/include"),
                str(work / "component/os/os_dep/device_lock.c"),
                str(work / "harness.c"), "-o", str(executable),
            ], text=True, capture_output=True)
            self.assertEqual(result.returncode, 0, result.stdout + result.stderr)
            subprocess.run([str(executable)], check=True)


if __name__ == "__main__":
    unittest.main()
