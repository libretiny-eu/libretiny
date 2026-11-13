"""Host qualification contracts, not hardware timing/footprint certification.

Compile the adapter, patched actual SDK device_lock.c and verbatim actual SDK
erase/burst function bodies. Mock only RTOS/hardware boundaries and SDK reads.
Run: python3 -m unittest discover -s libretiny/cores/realtek-amb1/tests -p test_image2_sdk_adapter.py -v
"""
from pathlib import Path
import shutil
import subprocess
import tempfile
import unittest

import test_image2_sdk_lock_support as lock

BASE = lock.ROOT / "cores/realtek-amb1/base/ota"


def function(source, signature):
    start = source.index(signature)
    brace = source.index("{", start)
    depth = 1
    end = brace + 1
    while depth:
        depth += (source[end] == "{") - (source[end] == "}")
        end += 1
    return source[start:end]


FLASH_HEADER = r"""
#ifndef MOCK_FLASH_H
#define MOCK_FLASH_H
#include <stdint.h>
typedef uint8_t u8;
typedef struct { int flashtype; } SPIC_INIT_PARA;
typedef struct { SPIC_INIT_PARA SpicInitPara; uint32_t Length; } flash_t;
int flash_stream_read(flash_t *, uint32_t, uint32_t, uint8_t *);
int flash_burst_write(flash_t *, uint32_t, uint32_t, uint8_t *);
void flash_erase_sector(flash_t *, uint32_t);
#endif
"""

HARDWARE = r"""
#include <string.h>
#include <stdlib.h>
#include "flash_api.h"
#include "image2_sdk_adapter.h"
static uint8_t memory[16384];
static int reads, programs, erases, bad_blank, bad_program, read_status = 1;
static uint32_t program_address, program_length;
static int ipsr;
int mock_ipsr(void) { return ipsr; }
void *xTaskGetCurrentTaskHandle(void) { return (void *)(uintptr_t)task; }
static flash_t flashobj;
static int isinit;
#define FLASH_MICRON 1
#define SPI_FLASH_BASE 0
static void flash_turnon(void) { assert(pool[0].held); }
static void flash_init(flash_t *obj) { (void)obj; isinit = 1; }
static void SpicDisableRtl8195A(void) {}
static void SpicWaitBusyDoneRtl8195A(void) {}
static void SpicWaitWipDoneRefinedRtl8195A(SPIC_INIT_PARA p) { (void)p; }
static void SpicWaitOperationDoneRtl8195A(SPIC_INIT_PARA p) { (void)p; }
static void SpicSectorEraseFlashRtl8195A(uint32_t a) {
    assert(pool[0].held && a == 4096); ++erases;
    memset(memory + a, 255, 4096);
    if (bad_blank) memory[a + 4095] = 0;
}
static void SpicUserProgramRtl8195A(uint8_t *data, SPIC_INIT_PARA p,
                                   uint32_t a, uint32_t *n) {
    (void)p; assert(pool[0].held && *n && *n <= 256);
    ++programs; program_address = a; program_length = *n;
    if (!bad_program) for (uint32_t i = 0; i < *n; ++i) memory[a+i] &= data[i];
    *n = 0;
}
int flash_stream_read(flash_t *obj, uint32_t a, uint32_t n, uint8_t *data) {
    (void)obj; assert(pool[0].held && n && n <= 256 && a+n <= sizeof(memory));
    ++reads; memcpy(data, memory+a, n); return read_status;
}
static int denied_writes;
int __wrap_flash_burst_write(flash_t *obj, uint32_t a, uint32_t n, uint8_t *data) {
    (void)obj; (void)a; (void)n; (void)data; ++denied_writes; return 0;
}
void __wrap_flash_erase_sector(flash_t *obj, uint32_t a) {
    (void)obj; (void)a; ++denied_writes;
}
"""

MAIN = r"""
int main(int argc, char **argv) {
    assert(argc == 2);
    image2_flash_sdkops ops;
    uint32_t scratch = 4096;
    image2_sdk_qualification_policy policy = { 0, sizeof(memory), &scratch, 1, { 0, 0 }, NULL, 0, { 0, 0 }, 0 };
    uint8_t value = 0xf0, out[300];
    memset(memory, 255, sizeof(memory));
    if (!strcmp(argv[1], "invalidpolicy")) {
        policy.read_start = UINT32_MAX - 8;
        assert(!image2_sdk_adapter_bind(&policy, &ops) && !ops.ctx);
        policy.read_start = 0;
        scratch = 4097;
        assert(!image2_sdk_adapter_bind(&policy, &ops) && !ops.ctx);
        scratch = 4096;
        policy.scratch_count = IMAGE2_SDK_SCRATCH_MAX + 1;
        assert(!image2_sdk_adapter_bind(&policy, &ops) && !ops.ctx);
        policy.scratch_count = 1; policy.scratch_sectors = NULL;
        assert(!image2_sdk_adapter_bind(&policy, &ops) && !ops.ctx);
        assert(!allocations && !programs && !erases);
        return 0;
    }
    if (!strcmp(argv[1], "readonly")) { policy.scratch_count = 0; policy.scratch_sectors = NULL; }
    assert(image2_sdk_adapter_bind(&policy, &ops) == 1);
    scratch = 8192; /* Copied authorization, not a mutable policy pointer. */
    assert(ops.try_lock(ops.ctx, &ops) == 0); /* No lazy lock allocation. */
    assert(!allocations);
    assert(device_mutex_prepare(RT_DEV_LOCK_FLASH) == 1);
    device_mutex_lock(RT_DEV_LOCK_FLASH);
    assert(ops.try_lock(ops.ctx, &ops) == 0 && ticks == 0);
    device_mutex_unlock(RT_DEV_LOCK_FLASH);
    assert(ops.try_lock(ops.ctx, &ops) == 1 && ticks == 0);
    assert(ops.try_lock(ops.ctx, &ops) == 0);
    task = 2;
    assert(ops.read(ops.ctx, 4096, out, 1) == 0);
    ops.unlock(ops.ctx, &ops); assert(pool[0].held);
    task = 1;
    ipsr = 1; assert(ops.read(ops.ctx, 4096, out, 1) == 0); ipsr = 0;
    assert(ops.read(ops.ctx, 4096, out+1, 299) == 1 && reads == 2);
    if (!strcmp(argv[1], "readonly")) {
        assert(ops.program(ops.ctx, 4096, &value, 1) == 0 && !programs);
    } else if (!strcmp(argv[1], "blankfail")) {
        bad_blank = 1;
        assert(ops.erase_sector(ops.ctx, 4096) == 0 && erases == 1 && reads == 18);
    } else if (!strcmp(argv[1], "verifyfail")) {
        bad_program = 1;
        assert(ops.program(ops.ctx, 4104, &value, 1) == 0 && programs == 1);
    } else if (!strcmp(argv[1], "readfail")) {
        read_status = 2;
        assert(ops.read(ops.ctx, 4096, out, 1) == 0);
    } else if (!strcmp(argv[1], "bounds")) {
        assert(ops.program(ops.ctx, 4351, out, 2) == 0 && !programs);
    } else if (!strcmp(argv[1], "erasebounds")) {
        assert(ops.erase_sector(ops.ctx, 4097) == 0 && !erases);
    } else if (!strcmp(argv[1], "unapproved")) {
        assert(ops.erase_sector(ops.ctx, 8192) == 0 && !erases);
    } else if (!strcmp(argv[1], "zero")) {
        assert(ops.program(ops.ctx, 4096, &value, 0) == 0 && !programs);
    } else {
        assert(ops.erase_sector(ops.ctx, 4096) == 1 && erases == 1 && reads == 18);
        for (unsigned offset = 0; offset < 4; ++offset) {
            assert(ops.program(ops.ctx, 4104+offset, &value, 1) == 1);
            assert(program_address == 4104+offset && program_length == 1);
        }
        assert(ops.program(ops.ctx, 4351, &value, 1) == 1);
        assert(memory[4350] == 255 && memory[4352] == 255);
        value = 255;
        assert(ops.program(ops.ctx, 4104, &value, 1) == 0 && programs == 5);
    }
    int prior = reads;
    assert(ops.read(ops.ctx, 4096, out, 1) == 0 && reads == prior); /* Fail-stop. */
    ops.unlock(ops.ctx, &policy); assert(pool[0].held);
    ops.unlock(ops.ctx, &ops); assert(!pool[0].held);
    ops.unlock(ops.ctx, &ops);
    assert(ops.try_lock(ops.ctx, &ops) == 0);
    assert(!denied_writes && !logs);
    return 0;
}
"""


class SDKAdapterTest(unittest.TestCase):
    def test_default_off_has_no_sdk_dependencies(self):
        with tempfile.TemporaryDirectory(prefix="amb1-adapter-off-") as directory:
            work = Path(directory)
            (work / "main.c").write_text(
                '#include "image2_sdk_adapter.h"\n#include <assert.h>\n'
                'int main(void) { image2_flash_sdkops ops; '
                'assert(!image2_sdk_adapter_bind(0, &ops)); '
                'assert(!ops.ctx && !ops.program && !ops.erase_sector); return 0; }\n')
            self.build(work, [BASE / "image2_sdk_adapter.c", work / "main.c"])
            subprocess.run([str(work / "test")], check=True)

    def build(self, work, sources, extra=()):
        result = subprocess.run([
            shutil.which("cc") or "cc", "-std=c99", "-Wall", "-Wextra", "-Werror",
            "-Wno-unused-function", "-Wno-unused-parameter", "-I", str(work),
            "-I", str(BASE), "-I", str(work / "component/os/os_dep/include"),
            *extra, *map(str, sources), "-o", str(work / "test"),
        ], text=True, capture_output=True)
        self.assertEqual(result.returncode, 0, result.stdout + result.stderr)

    def test_real_sdk_lock_erase_burst_with_private_wrapped_binding(self):
        flash = lock.SDK / "component/common/mbed/targets/hal/rtl8195a/flash_api.c"
        if not flash.is_file():
            self.skipTest("installed SDK unavailable; set AMB1_SDK_ROOT")
        with tempfile.TemporaryDirectory(prefix="amb1-adapter-") as directory:
            work = Path(directory)
            for relative in ("component/os/os_dep/device_lock.c",
                             "component/os/os_dep/include/device_lock.h"):
                target = work / relative
                target.parent.mkdir(parents=True, exist_ok=True)
                target.write_text((lock.SDK / relative).read_text())
            result = subprocess.run(["patch", "--batch", "--fuzz=0", "-p1", "-i", str(lock.PATCH)],
                                    cwd=work, text=True, capture_output=True)
            self.assertEqual(result.returncode, 0, result.stdout + result.stderr)
            (work / "osdep_service.h").write_text(lock.OSDEP)
            (work / "FreeRTOS.h").write_text("#define pdTRUE 1\n#define pdFALSE 0\n")
            (work / "semphr.h").write_text('#include "osdep_service.h"\nint xSemaphoreTake(_mutex, unsigned int);\n')
            (work / "task.h").write_text('typedef void *TaskHandle_t;\nTaskHandle_t xTaskGetCurrentTaskHandle(void);\n')
            (work / "ameba_soc.h").write_text('int mock_ipsr(void);\n#define __get_IPSR mock_ipsr\n')
            (work / "flash_api.h").write_text(FLASH_HEADER)
            source = flash.read_text()
            actual = function(source, "void flash_erase_sector(") + "\n" + function(source, "int flash_burst_write(")
            # Mach-O ld has no GNU --wrap. Rename only the copied SDK definitions
            # to the linker-provided real-symbol names; adapter stays unchanged.
            actual = actual.replace("void flash_erase_sector(", "void __real_flash_erase_sector(")
            actual = actual.replace("int flash_burst_write(", "int __real_flash_burst_write(")
            mock_lock = lock.HARNESS.split("int main(")[0]
            (work / "main.c").write_text(mock_lock + HARDWARE + actual + MAIN)
            self.build(work, [BASE / "image2_sdk_adapter.c", work / "component/os/os_dep/device_lock.c", work / "main.c"],
                       ["-DLT_AMB1_EXPERIMENTAL_IMAGE2_SDK_ADAPTER=1", "-Dprintf=mock_printf"])
            for case in ("success", "readonly", "blankfail", "verifyfail", "readfail", "bounds", "erasebounds", "unapproved", "zero", "invalidpolicy"):
                with self.subTest(case=case):
                    subprocess.run([str(work / "test"), case], check=True)


if __name__ == "__main__":
    unittest.main()
