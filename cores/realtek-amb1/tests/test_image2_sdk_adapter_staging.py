"""Host authorisation contract for the staging window and deny ranges.

The REAL ``image2_sdk_adapter.c`` is compiled behind an in-RAM NOR mock and a
host ``device_lock`` mock. The real shared-mutex primitive is covered separately
by ``test_image2_sdk_lock_support``/``test_image2_sdk_adapter``; this file proves
the additive staging capability cannot reach a denied or out-of-window byte, and
that the single-byte demote refuses unless BOTH the running source and the
inactive staged target publish the new signature.

Authorization failures latch the adapter, so each rejection category runs in its
own process (``argv`` mode). Bind is also one-shot per boot.

Run: python3 -m unittest discover -s libretiny/cores/realtek-amb1/tests \
        -p test_image2_sdk_adapter_staging.py -v
"""
from pathlib import Path
import shutil
import subprocess
import tempfile
import unittest


ROOT = Path(__file__).resolve().parents[3]
BASE = ROOT / "cores/realtek-amb1/base/ota"

DEVICE_LOCK_H = r"""
#ifndef MOCK_DEVICE_LOCK_H
#define MOCK_DEVICE_LOCK_H
typedef enum {
    RT_DEV_LOCK_CPWM = 0,
    RT_DEV_LOCK_FLASH,
    RT_DEV_LOCK_MAX
} RT_DEV_LOCK_E;
int device_mutex_prepare(RT_DEV_LOCK_E);
int device_mutex_try_lock(RT_DEV_LOCK_E);
void device_mutex_lock(RT_DEV_LOCK_E);
void device_mutex_unlock(RT_DEV_LOCK_E);
#endif
"""

AMEDA_SOC_H = r"""
#ifndef MOCK_AMEBA_SOC_H
#define MOCK_AMEBA_SOC_H
int mock_ipsr(void);
#define __get_IPSR mock_ipsr
#endif
"""

FLASH_API_H = r"""
#ifndef MOCK_FLASH_H
#define MOCK_FLASH_H
#include <stdint.h>
typedef uint8_t u8;
typedef struct { int flashtype; } SPIC_INIT_PARA;
typedef struct { SPIC_INIT_PARA SpicInitPara; uint32_t Length; } flash_t;
int flash_stream_read(flash_t *, uint32_t, uint32_t, uint8_t *);
#endif
"""

FREERTOS_H = "#define pdTRUE 1\n#define pdFALSE 0\n"
TASK_H = "typedef void *TaskHandle_t;\nTaskHandle_t xTaskGetCurrentTaskHandle(void);\n"

HARNESS = r"""
#include <assert.h>
#include <stdint.h>
#include <string.h>
#include "device_lock.h"
#include "flash_api.h"
#include "image2_sdk_adapter.h"

#define FLASH_SIZE 0x00010000u

static uint8_t memory[FLASH_SIZE];
static int mutex_held, task = 1, ipsr;
static unsigned reads, programs, erases, denied_writes;

int mock_ipsr(void) { return ipsr; }
void *xTaskGetCurrentTaskHandle(void) { return (void *)(uintptr_t)task; }

int device_mutex_prepare(RT_DEV_LOCK_E id) { return id == RT_DEV_LOCK_FLASH; }
int device_mutex_try_lock(RT_DEV_LOCK_E id) {
    if (id != RT_DEV_LOCK_FLASH || mutex_held) return 0;
    mutex_held = 1;
    return 1;
}
void device_mutex_lock(RT_DEV_LOCK_E id) {
    assert(id == RT_DEV_LOCK_FLASH && !mutex_held);
    mutex_held = 1;
}
void device_mutex_unlock(RT_DEV_LOCK_E id) {
    assert(id == RT_DEV_LOCK_FLASH && mutex_held);
    mutex_held = 0;
}

int flash_stream_read(flash_t *obj, uint32_t a, uint32_t n, uint8_t *data) {
    (void)obj;
    assert(mutex_held && n && n <= 256 && a + n <= FLASH_SIZE);
    ++reads;
    memcpy(data, memory + a, n);
    return 1;
}

void __real_flash_erase_sector(flash_t *obj, uint32_t a) {
    (void)obj;
    assert(mutex_held && a % 4096u == 0 && a + 4096u <= FLASH_SIZE);
    ++erases;
    memset(memory + a, 255, 4096);
}

int __real_flash_burst_write(flash_t *obj, uint32_t a, uint32_t n, uint8_t *data) {
    (void)obj;
    assert(mutex_held && n && n <= 256);
    ++programs;
    for (uint32_t i = 0; i < n; ++i) memory[a + i] &= data[i];
    return 1;
}

/* The reviewed --wrap denials must stay untouched and must never be called by
 * the staging path. */
int __wrap_flash_burst_write(flash_t *obj, uint32_t a, uint32_t n, uint8_t *data) {
    (void)obj; (void)a; (void)n; (void)data;
    ++denied_writes;
    return 0;
}
void __wrap_flash_erase_sector(flash_t *obj, uint32_t a) {
    (void)obj; (void)a;
    ++denied_writes;
}

static int bind_full(image2_flash_sdkops *ops, uint32_t staging_start, uint32_t staging_end,
                     uint32_t source_start, uint32_t source_end, uint32_t demote_address,
                     uint32_t target_signature_address,
                     const uint32_t *scratch, size_t scratch_count) {
    image2_sdk_range deny[3];
    image2_sdk_qualification_policy p;
    memset(&p, 0, sizeof(p));
    deny[0] = (image2_sdk_range){ 8192, 12288 };
    deny[1] = (image2_sdk_range){ 0, 4096 };
    deny[2] = (image2_sdk_range){ 12288, FLASH_SIZE };
    p.read_start = 0;
    p.read_size = FLASH_SIZE;
    p.staging = (image2_sdk_range){ staging_start, staging_end };
    p.scratch_sectors = scratch;
    p.scratch_count = scratch_count;
    p.source = (image2_sdk_range){ source_start, source_end };
    p.demote_address = demote_address;
    p.target_signature_address = target_signature_address;
    if (staging_start != staging_end) {
        p.deny_ranges = deny;
        p.deny_count = 3;
    }
    return image2_sdk_adapter_bind(&p, ops);
}

static int bind(image2_flash_sdkops *ops, uint32_t staging_start, uint32_t staging_end) {
    return bind_full(ops, staging_start, staging_end, 0, 0, 0, 0, NULL, 0);
}

int main(int argc, char **argv) {
    image2_flash_sdkops ops;
    uint8_t value = 0x5a, out[300];
    assert(argc == 2);
    memset(memory, 255, sizeof(memory));

    if (!strcmp(argv[1], "invalid")) {
        image2_sdk_range deny[3];
        image2_sdk_qualification_policy p;

        memset(&p, 0, sizeof(p));
        deny[0] = (image2_sdk_range){ 8192, 12288 };
        deny[1] = (image2_sdk_range){ 0, 4096 };
        deny[2] = (image2_sdk_range){ 12288, FLASH_SIZE };
        p.read_start = 0;
        p.read_size = FLASH_SIZE;

        p.staging = (image2_sdk_range){ 4097, 8192 }; /* misaligned */
        assert(!image2_sdk_adapter_bind(&p, &ops) && !ops.ctx);

        p.staging = (image2_sdk_range){ 8192, 4096 }; /* start > end */
        assert(!image2_sdk_adapter_bind(&p, &ops) && !ops.ctx);

        p.staging = (image2_sdk_range){ FLASH_SIZE - 4096, FLASH_SIZE + 4096 };
        assert(!image2_sdk_adapter_bind(&p, &ops) && !ops.ctx);

        p.staging = (image2_sdk_range){ 0, 0 }; /* deny without a window */
        p.deny_ranges = deny;
        p.deny_count = 3;
        assert(!image2_sdk_adapter_bind(&p, &ops) && !ops.ctx);

        p.staging = (image2_sdk_range){ 4096, 8192 }; /* deny overlaps */
        deny[2] = (image2_sdk_range){ 4096, 8192 };
        assert(!image2_sdk_adapter_bind(&p, &ops) && !ops.ctx);

        deny[2] = (image2_sdk_range){ 12288, 12288 }; /* empty deny */
        assert(!image2_sdk_adapter_bind(&p, &ops) && !ops.ctx);

        deny[2] = (image2_sdk_range){ FLASH_SIZE, FLASH_SIZE + 4096 }; /* outside extent */
        assert(!image2_sdk_adapter_bind(&p, &ops) && !ops.ctx);

        deny[2] = (image2_sdk_range){ 12288, FLASH_SIZE };
        p.deny_ranges = NULL; /* count without an array */
        p.deny_count = 1;
        assert(!image2_sdk_adapter_bind(&p, &ops) && !ops.ctx);

        p.deny_ranges = deny; /* too many ranges */
        p.deny_count = IMAGE2_SDK_DENY_MAX + 1;
        assert(!image2_sdk_adapter_bind(&p, &ops) && !ops.ctx);

        /* Source/demote policy: the demote byte must be exactly source.start+8
         * inside a non-empty source, and the source must never overlap staging. */
        memset(&p, 0, sizeof(p));
        p.read_start = 0;
        p.read_size = FLASH_SIZE;

        p.source = (image2_sdk_range){ 8192, 12288 };
        p.demote_address = 8192; /* not source.start + 8 */
        assert(!image2_sdk_adapter_bind(&p, &ops) && !ops.ctx);

        p.source = (image2_sdk_range){ 12288, 12288 }; /* empty source */
        p.demote_address = 12296;
        assert(!image2_sdk_adapter_bind(&p, &ops) && !ops.ctx);

        p.source = (image2_sdk_range){ 8192, 12288 };
        p.demote_address = 0; /* source without demote is allowed... */
        p.staging = (image2_sdk_range){ 8192, 12288 }; /* ...but must be disjoint */
        p.deny_ranges = deny;
        p.deny_count = 3;
        assert(!image2_sdk_adapter_bind(&p, &ops) && !ops.ctx);

        p.staging = (image2_sdk_range){ 0, 0 };
        p.deny_ranges = NULL;
        p.deny_count = 0;
        p.source = (image2_sdk_range){ FLASH_SIZE - 4096, FLASH_SIZE + 4096 };
        assert(!image2_sdk_adapter_bind(&p, &ops) && !ops.ctx);

        p.source = (image2_sdk_range){ 8192, 12288 };
        p.demote_address = 8200;
        p.source = (image2_sdk_range){ 8192, 8196 }; /* too short for a sig */
        assert(!image2_sdk_adapter_bind(&p, &ops) && !ops.ctx);

        /* Target-publication evidence is bound together with demote: it must be
         * present, exactly staging.start + 8 and inside a real staging window. */
        memset(&p, 0, sizeof(p));
        p.read_start = 0;
        p.read_size = FLASH_SIZE;
        p.source = (image2_sdk_range){ 8192, 12288 };
        p.demote_address = 8200;
        p.staging = (image2_sdk_range){ 4096, 8192 };

        p.target_signature_address = 0; /* missing evidence */
        assert(!image2_sdk_adapter_bind(&p, &ops) && !ops.ctx);

        p.target_signature_address = 4105; /* misaligned: not staging.start + 8 */
        assert(!image2_sdk_adapter_bind(&p, &ops) && !ops.ctx);

        p.target_signature_address = 4096; /* window start, not the signature */
        assert(!image2_sdk_adapter_bind(&p, &ops) && !ops.ctx);

        p.target_signature_address = 4104;
        p.staging = (image2_sdk_range){ 0, 0 }; /* evidence without a window */
        assert(!image2_sdk_adapter_bind(&p, &ops) && !ops.ctx);

        /* Evidence without demote is a malformed policy, never an implicit grant. */
        memset(&p, 0, sizeof(p));
        p.read_start = 0;
        p.read_size = FLASH_SIZE;
        p.source = (image2_sdk_range){ 8192, 12288 };
        p.staging = (image2_sdk_range){ 4096, 8192 };
        p.target_signature_address = 4104;
        assert(!image2_sdk_adapter_bind(&p, &ops) && !ops.ctx);

        p.source = (image2_sdk_range){ 0, 0 }; /* evidence without a source */
        assert(!image2_sdk_adapter_bind(&p, &ops) && !ops.ctx);

        /* The matching tuple is accepted last: bind is one-shot, so success must
         * not be followed by further rejects in this process. It is exercised
         * end-to-end by the demote modes. */
        p.source = (image2_sdk_range){ 8192, 12288 };
        p.demote_address = 8200;
        assert(image2_sdk_adapter_bind(&p, &ops) == 1);

        assert(!programs && !erases && !denied_writes && !reads);
        return 0;
    }

    if (!strcmp(argv[1], "readonly")) {
        assert(bind(&ops, 0, 0) == 1);
    } else if (!strcmp(argv[1], "scratch_in_source_denied")) {
        static const uint32_t scratch_source[1] = { 8192 };
        assert(bind_full(&ops, 4096, 8192, 8192, 12288, 8200, 4104,
                         scratch_source, 1) == 1);
    } else if (!strncmp(argv[1], "demote", 6)) {
        assert(bind_full(&ops, 4096, 8192, 8192, 12288, 8200, 4104, NULL, 0) == 1);
    } else {
        assert(bind(&ops, 4096, 8192) == 1);
    }
    assert(ops.try_lock(ops.ctx, &ops) == 1);

    if (!strcmp(argv[1], "success")) {
        assert(ops.erase_sector(ops.ctx, 4096) == 1 && erases == 1);
        assert(ops.program(ops.ctx, 4104, &value, 1) == 1 && programs == 1);
        assert(memory[4104] == 0x5a && memory[4103] == 255 && memory[4105] == 255);
        /* Whole-flash reads stay allowed and never authorize a mutation. */
        assert(ops.read(ops.ctx, 0, out, 1) == 1);
        assert(ops.read(ops.ctx, 8192, out, 1) == 1);
        assert(ops.read(ops.ctx, 12288, out, 1) == 1);
        assert(ops.read(ops.ctx, 4096, out, 300) == 1);
        assert(ops.program(ops.ctx, 0, &value, 1) == 0);
        assert(programs == 1 && erases == 1 && !denied_writes);
    } else if (!strcmp(argv[1], "erase_deny")) {
        assert(ops.erase_sector(ops.ctx, 8192) == 0);
        assert(!erases && !denied_writes);
    } else if (!strcmp(argv[1], "program_deny")) {
        assert(ops.program(ops.ctx, 8200, &value, 1) == 0);
        assert(!programs && !denied_writes);
    } else if (!strcmp(argv[1], "erase_outside")) {
        assert(ops.erase_sector(ops.ctx, 12288) == 0);
        assert(!erases && !denied_writes);
    } else if (!strcmp(argv[1], "program_outside")) {
        assert(ops.program(ops.ctx, 12288, &value, 1) == 0);
        assert(!programs && !denied_writes);
    } else if (!strcmp(argv[1], "straddle")) {
        assert(ops.program(ops.ctx, 8191, &value, 2) == 0);
        assert(!programs && !denied_writes);
    } else if (!strcmp(argv[1], "read_extent")) {
        unsigned before = reads;
        assert(ops.read(ops.ctx, FLASH_SIZE, out, 1) == 0);
        assert(ops.read(ops.ctx, FLASH_SIZE - 1, out, 2) == 0);
        assert(reads == before); /* rejected before any SDK I/O */
    } else if (!strcmp(argv[1], "readonly")) {
        assert(ops.program(ops.ctx, 4096, &value, 1) == 0);
        assert(!programs && !erases);
    } else if (!strcmp(argv[1], "demote")) {
        /* Both the running source and the inactive target must publish the new
         * signature; the target write is also the required staged write. */
        assert(ops.erase_sector(ops.ctx, 4096) == 1);
        assert(ops.program(ops.ctx, 4104, "81958711", 8) == 1);
        memcpy(memory + 8200, "81958711", 8);
        assert(ops.demote(ops.ctx) == 1);
        assert(memcmp(memory + 8200, "01958711", 8) == 0);
        assert(programs == 2 && erases == 1);
        /* The running slot is STILL hard-denied for generic writes. */
        assert(ops.program(ops.ctx, 8200, &value, 1) == 0);
    } else if (!strcmp(argv[1], "demote_twice")) {
        assert(ops.program(ops.ctx, 4104, "81958711", 8) == 1);
        memcpy(memory + 8200, "81958711", 8);
        assert(ops.demote(ops.ctx) == 1);
        assert(ops.demote(ops.ctx) == 0); /* one-shot per bound session */
        assert(memcmp(memory + 8200, "01958711", 8) == 0);
    } else if (!strcmp(argv[1], "demote_no_stage")) {
        memcpy(memory + 4104, "81958711", 8);
        memcpy(memory + 8200, "81958711", 8);
        assert(ops.demote(ops.ctx) == 0);
        assert(memcmp(memory + 8200, "81958711", 8) == 0);
        assert(!programs);
    } else if (!strcmp(argv[1], "demote_target_erased")) {
        /* Valid source and staged write, but the inactive slot is still blank. */
        assert(ops.program(ops.ctx, 5000, &value, 1) == 1);
        memcpy(memory + 8200, "81958711", 8);
        assert(ops.demote(ops.ctx) == 0);
        assert(memcmp(memory + 8200, "81958711", 8) == 0);
        assert(programs == 1);
    } else if (!strcmp(argv[1], "demote_target_old")) {
        assert(ops.program(ops.ctx, 5000, &value, 1) == 1);
        memcpy(memory + 4104, "01958711", 8);
        memcpy(memory + 8200, "81958711", 8);
        assert(ops.demote(ops.ctx) == 0);
        assert(memcmp(memory + 8200, "81958711", 8) == 0);
        assert(programs == 1);
    } else if (!strcmp(argv[1], "demote_target_partial")) {
        assert(ops.program(ops.ctx, 5000, &value, 1) == 1);
        /* Seven new bytes then one erased byte: not a published signature. */
        memcpy(memory + 4104, "8195871", 7);
        memcpy(memory + 8200, "81958711", 8);
        assert(ops.demote(ops.ctx) == 0);
        assert(memcmp(memory + 8200, "81958711", 8) == 0);
        assert(programs == 1);
    } else if (!strcmp(argv[1], "demote_source_old")) {
        /* Target evidence cannot rescue a source that is not still new. */
        assert(ops.program(ops.ctx, 5000, &value, 1) == 1);
        memcpy(memory + 4104, "81958711", 8);
        memcpy(memory + 8200, "01958711", 8);
        assert(ops.demote(ops.ctx) == 0);
        assert(memcmp(memory + 8200, "01958711", 8) == 0);
        assert(programs == 1);
    } else if (!strcmp(argv[1], "demote_program_denied")) {
        memcpy(memory + 8200, "81958711", 8);
        assert(ops.program(ops.ctx, 8200, &value, 1) == 0);
        assert(!programs);
    } else if (!strcmp(argv[1], "scratch_in_source_denied")) {
        /* A scratch sector that overlaps the running slot must NOT widen the
         * source capability: generic program/erase stay denied there. */
        memcpy(memory + 8200, "81958711", 8);
        assert(ops.program(ops.ctx, 8200, &value, 1) == 0);
        assert(ops.erase_sector(ops.ctx, 8192) == 0);
        assert(!programs && !erases);
    } else {
        return 2;
    }

    ops.unlock(ops.ctx, &ops);
    assert(!mutex_held && !denied_writes);
    return 0;
}
"""


class SDKAdapterStagingTest(unittest.TestCase):
    @classmethod
    def setUpClass(cls):
        cc = shutil.which("cc")
        if not cc:
            raise unittest.SkipTest("host C compiler unavailable")
        cls.tmp = tempfile.TemporaryDirectory(prefix="amb1-adapter-staging-")
        cls.addClassCleanup(cls.tmp.cleanup)
        work = Path(cls.tmp.name)
        (work / "device_lock.h").write_text(DEVICE_LOCK_H)
        (work / "ameba_soc.h").write_text(AMEDA_SOC_H)
        (work / "flash_api.h").write_text(FLASH_API_H)
        (work / "FreeRTOS.h").write_text(FREERTOS_H)
        (work / "task.h").write_text(TASK_H)
        (work / "harness.c").write_text(HARNESS)
        result = subprocess.run([
            cc, "-std=c99", "-Wall", "-Wextra", "-Werror",
            "-Wno-unused-function", "-Wno-unused-parameter",
            "-DLT_AMB1_EXPERIMENTAL_IMAGE2_SDK_ADAPTER=1",
            "-I", str(work), "-I", str(BASE),
            str(BASE / "image2_sdk_adapter.c"), str(work / "harness.c"),
            "-o", str(work / "test"),
        ], text=True, capture_output=True)
        if result.returncode:
            raise AssertionError(result.stdout + result.stderr)
        cls.binary = work / "test"

    def run_mode(self, mode):
        result = subprocess.run([str(self.binary), mode], text=True, capture_output=True)
        self.assertEqual(result.returncode, 0, result.stderr.strip())

    def test_staging_window_and_reads(self):
        self.run_mode("success")

    def test_denied_and_out_of_window_mutations_reach_no_sdk_call(self):
        for mode in ("erase_deny", "program_deny", "erase_outside",
                     "program_outside", "straddle"):
            with self.subTest(mode=mode):
                self.run_mode(mode)

    def test_read_is_limited_to_the_declared_extent(self):
        self.run_mode("read_extent")

    def test_scratch_only_sentinel_remains_read_only(self):
        self.run_mode("readonly")

    def test_demote_is_the_only_running_slot_mutation(self):
        for mode in ("demote", "demote_twice", "demote_program_denied"):
            with self.subTest(mode=mode):
                self.run_mode(mode)

    def test_demote_requires_a_prior_staged_write(self):
        self.run_mode("demote_no_stage")

    def test_demote_requires_published_target_evidence(self):
        """A valid source signature is not enough: the inactive slot must have
        published the new signature. Erased, old or partial targets fail closed
        with no source mutation."""
        for mode in ("demote_target_erased", "demote_target_old",
                     "demote_target_partial"):
            with self.subTest(mode=mode):
                self.run_mode(mode)

    def test_demote_still_requires_the_new_source_signature(self):
        self.run_mode("demote_source_old")

    def test_scratch_never_widens_the_running_slot_source(self):
        self.run_mode("scratch_in_source_denied")

    def test_malformed_policy_is_rejected_before_binding(self):
        self.run_mode("invalid")


if __name__ == "__main__":
    unittest.main()
