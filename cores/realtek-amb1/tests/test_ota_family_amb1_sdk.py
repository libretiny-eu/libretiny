"""Host contract for the strong AMB1 family OTA binding.

Compiles the REAL ``lt_ota_family_amb1_sdk.c`` together with the REAL
``image2_sdk_adapter.c`` against an in-RAM NOR mock, a host ``device_lock`` mock
and a mock boot source. Proves:

  * the seam stays fail-closed when the adapter build is off, when the shared SDK
    mutex cannot be prepared, and when selector evidence disagrees;
  * the running slot is derived from ``lt_ota_dual_get_current()`` and is never
    authorized, even if the boot source is misreported;
  * only the inactive slot is writable; boot and KVS stay denied;
  * ``sdkops()`` re-checks the boot source at hand-off;
  * binding is one-shot/latched for the boot;
  * a unique link-extraction anchor exists so GNU ld pulls the strong member.

One scenario per process because the binding latches one result per boot.

Run: python3 -m unittest discover -s libretiny/cores/realtek-amb1/tests \
        -p test_ota_family_amb1_sdk.py -v
"""
from pathlib import Path
import shutil
import subprocess
import tempfile
import unittest


ROOT = Path(__file__).resolve().parents[3]
BASE = ROOT / "cores/realtek-amb1/base/ota"
BUILDER = ROOT / "builder/family/realtek-amb1.py"
SDK_SOURCE = BASE / "lt_ota_family_amb1_sdk.c"

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

AMEBA_SOC_H = r"""
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

LT_OTA_H = r"""
#ifndef MOCK_LT_OTA_H
#define MOCK_LT_OTA_H
#include <stddef.h>
#include <stdint.h>
typedef struct { uint32_t reserved; } lt_ota_ctx_t;
typedef int lt_ota_family_result_t;
typedef struct { uint32_t reserved; } uf2_block_t;
uint8_t lt_ota_dual_get_current(void);
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
#include "lt_ota_family_amb1.h"
#include "lt_ota_family_amb1_sdk.h"

#define FLASH_SIZE   0x00800000u
#define SLOT_A_START 0x0000B000u
#define SLOT_A_END   0x0010B000u
#define SLOT_B_START 0x00177000u
#define SLOT_B_END   0x00275000u
#define KVS_START    0x00275000u
#define KVS_END      0x00277000u

static uint8_t memory[FLASH_SIZE];
static int mutex_held, mutex_init, task = 1, ipsr, prepare_ok = 1;
static unsigned reads, programs, erases;
static unsigned current_slot;
static int marker;

uint8_t lt_ota_dual_get_current(void) { return (uint8_t)current_slot; }
int mock_ipsr(void) { return ipsr; }
void *xTaskGetCurrentTaskHandle(void) { return (void *)(uintptr_t)task; }

int device_mutex_prepare(RT_DEV_LOCK_E id) {
    if (id != RT_DEV_LOCK_FLASH || !prepare_ok) return 0;
    mutex_init = 1;
    return 1;
}
int device_mutex_try_lock(RT_DEV_LOCK_E id) {
    if (id != RT_DEV_LOCK_FLASH || !mutex_init || mutex_held) return 0;
    mutex_held = 1;
    return 1;
}
void device_mutex_lock(RT_DEV_LOCK_E id) {
    assert(id == RT_DEV_LOCK_FLASH && mutex_init && !mutex_held);
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

static const uint8_t new_sig[8] = { '8', '1', '9', '5', '8', '7', '1', '1' };
static const uint8_t old_sig[8] = { '0', '1', '9', '5', '8', '7', '1', '1' };

static uint32_t slot_start(unsigned slot) { return slot ? SLOT_B_START : SLOT_A_START; }

/* Blank flash, a new signature in the running slot and an erased/old inactive
 * slot: the reviewed selector state the binding requires. */
static void setup(unsigned running, int inactive_old) {
    current_slot = running;
    memset(memory, 255, sizeof(memory));
    memcpy(memory + slot_start(running) + 8, new_sig, 8);
    if (inactive_old)
        memcpy(memory + slot_start(1u - running) + 8, old_sig, 8);
}

int main(int argc, char **argv) {
    const image2_flash_sdkops *ops;
    uint8_t value = 0x5a, out[8];
    assert(argc == 2);
    memset(memory, 255, sizeof(memory));

    if (!strcmp(argv[1], "prepare")) {
        assert(mutex_init == 0);
        assert(lt_ota_family_amb1_sdk_prepare() == 1);
        assert(lt_ota_family_amb1_sdk_prepare() == 1); /* idempotent */
        assert(mutex_init == 1 && reads == 0);
        return 0;
    }
    if (!strcmp(argv[1], "prepare_fail")) {
        prepare_ok = 0;
        assert(lt_ota_family_amb1_sdk_prepare() == 0);
        prepare_ok = 1;
        /* Latched failure: a later prepare must not enable the seam. */
        assert(lt_ota_family_amb1_sdk_prepare() == 0);
        assert(lt_ota_family_amb1_sdkops() == NULL);
        assert(reads == 0 && !programs && !erases);
        return 0;
    }
    if (!strcmp(argv[1], "isr")) {
        setup(0, 0);
        ipsr = 1;
        assert(lt_ota_family_amb1_sdkops() == NULL);
        ipsr = 0;
        assert(lt_ota_family_amb1_sdkops() == NULL); /* latched */
        return 0;
    }
    if (!strcmp(argv[1], "boot_oob")) {
        setup(0, 0);
        current_slot = 2;
        assert(lt_ota_family_amb1_sdkops() == NULL);
        assert(reads == 0);
        return 0;
    }
    if (!strcmp(argv[1], "evidence_running")) {
        /* Reported running slot does not carry the new signature. */
        setup(0, 0);
        memset(memory + SLOT_A_START + 8, 255, 8);
        assert(lt_ota_family_amb1_sdkops() == NULL);
        memset(memory + SLOT_A_START + 8, 0, 8);
        assert(lt_ota_family_amb1_sdkops() == NULL);
        return 0;
    }
    if (!strcmp(argv[1], "evidence_running_old")) {
        setup(0, 0);
        memcpy(memory + SLOT_A_START + 8, old_sig, 8);
        assert(lt_ota_family_amb1_sdkops() == NULL);
        return 0;
    }
    if (!strcmp(argv[1], "evidence_target")) {
        setup(0, 0);
        memcpy(memory + SLOT_B_START + 8, "12345678", 8);
        assert(lt_ota_family_amb1_sdkops() == NULL);
        return 0;
    }
    if (!strcmp(argv[1], "evidence_both_new")) {
        setup(0, 0);
        memcpy(memory + SLOT_B_START + 8, new_sig, 8);
        assert(lt_ota_family_amb1_sdkops() == NULL);
        return 0;
    }
    if (!strcmp(argv[1], "misdetected_running")) {
        /* The stub reports B as running, but B is blank and A is the real
         * image. Evidence must refuse instead of authorizing B's sibling. */
        setup(0, 0);
        current_slot = 1;
        assert(lt_ota_family_amb1_sdkops() == NULL);
        assert(!programs && !erases);
        return 0;
    }
    if (!strcmp(argv[1], "latch")) {
        setup(0, 0);
        memset(memory + SLOT_A_START + 8, 255, 8);
        assert(lt_ota_family_amb1_sdkops() == NULL);
        memcpy(memory + SLOT_A_START + 8, new_sig, 8); /* repair is too late */
        assert(lt_ota_family_amb1_sdkops() == NULL);
        return 0;
    }
    if (!strcmp(argv[1], "recheck")) {
        setup(0, 0);
        ops = lt_ota_family_amb1_sdkops();
        assert(ops && lt_ota_family_amb1_sdkops() == ops);
        current_slot = 1; /* boot source changed after hand-off */
        assert(lt_ota_family_amb1_sdkops() == NULL);
        current_slot = 0;
        assert(lt_ota_family_amb1_sdkops() == ops);
        return 0;
    }

    /* Remaining modes bind successfully and then probe one authorization. */
    if (!strcmp(argv[1], "happy") || !strcmp(argv[1], "deny_running") ||
        !strcmp(argv[1], "deny_program_running") || !strcmp(argv[1], "deny_boot") ||
        !strcmp(argv[1], "deny_kvs") || !strcmp(argv[1], "read_extent")) {
        setup(0, 0);
        ops = lt_ota_family_amb1_sdkops();
        assert(ops != NULL);
        assert(ops->try_lock(ops->ctx, &marker) == 1);
        if (!strcmp(argv[1], "happy")) {
            assert(ops->erase_sector(ops->ctx, SLOT_B_START) == 1 && erases == 1);
            assert(ops->program(ops->ctx, SLOT_B_START + 8, &value, 1) == 1 &&
                   programs == 1);
            assert(memory[SLOT_B_START + 8] == 0x5a);
            assert(memcmp(memory + SLOT_A_START + 8, new_sig, 8) == 0);
            assert(ops->read(ops->ctx, 0, out, 1) == 1);
            assert(ops->read(ops->ctx, KVS_START, out, 1) == 1);
            assert(ops->read(ops->ctx, SLOT_A_START, out, 8) == 1);
            assert(ops->program(ops->ctx, 0, &value, 1) == 0); /* boot deny */
        } else if (!strcmp(argv[1], "deny_running")) {
            assert(ops->erase_sector(ops->ctx, SLOT_A_START) == 0);
            assert(!erases && !programs);
        } else if (!strcmp(argv[1], "deny_program_running")) {
            assert(ops->program(ops->ctx, SLOT_A_START + 8, &value, 1) == 0);
            assert(!programs && !erases);
        } else if (!strcmp(argv[1], "deny_boot")) {
            assert(ops->erase_sector(ops->ctx, 0) == 0);
            assert(!erases && !programs);
        } else if (!strcmp(argv[1], "deny_kvs")) {
            assert(ops->erase_sector(ops->ctx, KVS_START) == 0);
            assert(ops->program(ops->ctx, KVS_END - 1, &value, 1) == 0);
            assert(!erases && !programs);
        } else {
            unsigned before = reads;
            assert(ops->read(ops->ctx, FLASH_SIZE, out, 1) == 0);
            assert(reads == before);
        }
        ops->unlock(ops->ctx, &marker);
        assert(!mutex_held);
        return 0;
    }

    if (!strcmp(argv[1], "slot_a_target")) {
        /* Slot B running: the inactive slot becomes A. */
        setup(1, 1);
        ops = lt_ota_family_amb1_sdkops();
        assert(ops != NULL);
        assert(ops->try_lock(ops->ctx, &marker) == 1);
        assert(ops->erase_sector(ops->ctx, SLOT_A_START) == 1 && erases == 1);
        assert(ops->program(ops->ctx, SLOT_A_START + 8, &value, 1) == 1);
        assert(memcmp(memory + SLOT_B_START + 8, new_sig, 8) == 0);
        ops->unlock(ops->ctx, &marker);
        return 0;
    }
    if (!strcmp(argv[1], "slot_a_deny_running")) {
        setup(1, 1);
        ops = lt_ota_family_amb1_sdkops();
        assert(ops != NULL);
        assert(ops->try_lock(ops->ctx, &marker) == 1);
        assert(ops->erase_sector(ops->ctx, SLOT_B_START) == 0);
        assert(!erases);
        ops->unlock(ops->ctx, &marker);
        return 0;
    }
    if (!strcmp(argv[1], "slot_a_deny_boot")) {
        setup(1, 1);
        ops = lt_ota_family_amb1_sdkops();
        assert(ops != NULL);
        assert(ops->try_lock(ops->ctx, &marker) == 1);
        assert(ops->program(ops->ctx, 8, &value, 1) == 0);
        assert(!programs);
        ops->unlock(ops->ctx, &marker);
        return 0;
    }
    if (!strcmp(argv[1], "demote") || !strcmp(argv[1], "demote_no_stage") ||
        !strcmp(argv[1], "demote_target_missing")) {
        setup(0, 0);
        ops = lt_ota_family_amb1_sdkops();
        assert(ops != NULL);
        assert(ops->demote != NULL);
        assert(ops->try_lock(ops->ctx, &marker) == 1);
        if (!strcmp(argv[1], "demote_no_stage")) {
            /* The dedicated demote is refused until this session staged data. */
            assert(ops->demote(ops->ctx) == 0);
            assert(!programs && memcmp(memory + SLOT_A_START + 8, new_sig, 8) == 0);
        } else if (!strcmp(argv[1], "demote_target_missing")) {
            /* Staged data alone is not enough: the inactive slot must publish the
             * new signature before the source may be demoted. */
            assert(ops->erase_sector(ops->ctx, SLOT_B_START) == 1);
            assert(ops->program(ops->ctx, SLOT_B_START + 16, &value, 1) == 1);
            assert(ops->demote(ops->ctx) == 0);
            assert(memcmp(memory + SLOT_A_START + 8, new_sig, 8) == 0);
        } else {
            assert(ops->erase_sector(ops->ctx, SLOT_B_START) == 1);
            /* Publishing the target signature at the bound offset is also the
             * required staged write for this session. */
            assert(ops->program(ops->ctx, SLOT_B_START + 8, new_sig, 8) == 1);
            assert(ops->demote(ops->ctx) == 1);
            assert(memcmp(memory + SLOT_A_START + 8, old_sig, 8) == 0);
            /* Generic writes to the running slot stay denied after demote. */
            assert(ops->program(ops->ctx, SLOT_A_START + 8, &value, 1) == 0);
        }
        ops->unlock(ops->ctx, &marker);
        assert(!mutex_held);
        return 0;
    }
    if (!strcmp(argv[1], "slot_a_demote")) {
        setup(1, 1);
        ops = lt_ota_family_amb1_sdkops();
        assert(ops != NULL);
        assert(ops->try_lock(ops->ctx, &marker) == 1);
        assert(ops->erase_sector(ops->ctx, SLOT_A_START) == 1);
        assert(ops->program(ops->ctx, SLOT_A_START + 8, new_sig, 8) == 1);
        assert(ops->demote(ops->ctx) == 1);
        assert(memcmp(memory + SLOT_B_START + 8, old_sig, 8) == 0);
        assert(ops->erase_sector(ops->ctx, SLOT_B_START) == 0);
        ops->unlock(ops->ctx, &marker);
        return 0;
    }
    return 2;
}
"""


class OtaFamilyAmb1SDKTests(unittest.TestCase):
    @classmethod
    def setUpClass(cls):
        cc = shutil.which("cc")
        if not cc:
            raise unittest.SkipTest("host C compiler unavailable")
        cls.tmp = tempfile.TemporaryDirectory(prefix="ota-family-amb1-sdk-")
        cls.addClassCleanup(cls.tmp.cleanup)
        work = Path(cls.tmp.name)
        (work / "device_lock.h").write_text(DEVICE_LOCK_H)
        (work / "ameba_soc.h").write_text(AMEBA_SOC_H)
        (work / "flash_api.h").write_text(FLASH_API_H)
        (work / "lt_ota.h").write_text(LT_OTA_H)
        (work / "FreeRTOS.h").write_text(FREERTOS_H)
        (work / "task.h").write_text(TASK_H)
        (work / "harness.c").write_text(HARNESS)

        common = [
            cc, "-std=c99", "-Wall", "-Wextra", "-Werror",
            "-Wno-unused-function", "-Wno-unused-parameter",
            "-I", str(work), "-I", str(BASE),
        ]
        enabled = [
            "-DLT_AMB1_EXPERIMENTAL_IMAGE2_SDK_ADAPTER=1",
            str(BASE / "image2_sdk_adapter.c"),
            str(SDK_SOURCE), str(work / "harness.c"),
            "-o", str(work / "test"),
        ]
        result = subprocess.run([*common, *enabled], text=True, capture_output=True)
        if result.returncode:
            raise AssertionError(result.stdout + result.stderr)
        cls.binary = work / "test"
        cls.work = work

        # Off build: the seam must not require any SDK symbol.
        off_main = (
            '#include "lt_ota_family_amb1.h"\n'
            '#include "lt_ota_family_amb1_sdk.h"\n'
            '#include "image2_sdk_adapter.h"\n'
            '#include <assert.h>\n'
            'int main(void) { image2_flash_sdkops ops; '
            'assert(lt_ota_family_amb1_sdk_prepare() == 0); '
            'assert(lt_ota_family_amb1_sdkops() == NULL); '
            'assert(!image2_sdk_adapter_bind(0, &ops)); return 0; }\n'
        )
        (work / "off.c").write_text(off_main)
        result = subprocess.run(
            [*common, str(BASE / "image2_sdk_adapter.c"), str(SDK_SOURCE),
             str(work / "off.c"), "-o", str(work / "test_off")],
            text=True, capture_output=True,
        )
        if result.returncode:
            raise AssertionError(result.stdout + result.stderr)

    def run_mode(self, mode):
        result = subprocess.run([str(self.binary), mode], text=True, capture_output=True)
        self.assertEqual(result.returncode, 0, result.stderr.strip())

    # ------------------------------------------------------- fail-closed gates
    def test_off_build_returns_null_without_sdk_symbols(self):
        result = subprocess.run([str(self.work / "test_off")], text=True,
                                capture_output=True)
        self.assertEqual(result.returncode, 0, result.stderr.strip())

    def test_prepare_is_idempotent(self):
        self.run_mode("prepare")

    def test_prepare_failure_latches_the_seam_closed(self):
        self.run_mode("prepare_fail")

    def test_isr_and_out_of_range_boot_source_fail_closed(self):
        self.run_mode("isr")
        self.run_mode("boot_oob")

    def test_selector_evidence_must_match(self):
        for mode in ("evidence_running", "evidence_running_old",
                     "evidence_target", "evidence_both_new"):
            with self.subTest(mode=mode):
                self.run_mode(mode)

    def test_misdetected_running_slot_authorizes_nothing(self):
        self.run_mode("misdetected_running")

    def test_binding_failure_latches_for_the_boot(self):
        self.run_mode("latch")

    # ------------------------------------------------------------- hand-off
    def test_hand_off_rechecks_the_boot_source(self):
        self.run_mode("recheck")

    # ------------------------------------------------------------- authority
    def test_only_the_inactive_slot_is_writable(self):
        self.run_mode("happy")
        self.run_mode("slot_a_target")

    def test_running_slot_boot_and_kvs_are_denied(self):
        for mode in ("deny_running", "deny_program_running", "deny_boot",
                     "deny_kvs", "slot_a_deny_running", "slot_a_deny_boot"):
            with self.subTest(mode=mode):
                self.run_mode(mode)

    def test_demote_is_a_separate_after_staging_capability(self):
        for mode in ("demote", "demote_no_stage", "demote_target_missing",
                     "slot_a_demote"):
            with self.subTest(mode=mode):
                self.run_mode(mode)

    def test_read_is_limited_to_the_declared_extent(self):
        self.run_mode("read_extent")

    # --------------------------------------------------------- link contract
    def test_unique_anchor_and_forced_archive_extraction(self):
        source = SDK_SOURCE.read_text()
        self.assertIn("lt_ota_family_amb1_sdk_anchor", source)
        self.assertEqual(source.count("lt_ota_family_amb1_sdk_anchor ="), 1)
        builder = BUILDER.read_text()
        self.assertIn("-Wl,--undefined=lt_ota_family_amb1_sdk_anchor", builder)
        # The family hooks also have weak defaults in lt_ota.c, so the bridge
        # archive member needs its own forced-extraction anchor.
        bridge = (BASE / "lt_ota_family_amb1.c").read_text()
        self.assertIn("lt_ota_family_amb1_anchor", bridge)
        self.assertEqual(bridge.count("lt_ota_family_amb1_anchor ="), 1)
        self.assertIn("-Wl,--undefined=lt_ota_family_amb1_anchor", builder)

        nm = shutil.which("nm")
        if not nm:
            self.skipTest("nm unavailable")
        obj = self.work / "sdk.o"
        cc = shutil.which("cc")
        result = subprocess.run([
            cc, "-std=c99", "-Wall", "-Wextra", "-Werror",
            "-Wno-unused-function", "-Wno-unused-parameter",
            "-DLT_AMB1_EXPERIMENTAL_IMAGE2_SDK_ADAPTER=1",
            "-I", str(self.work), "-I", str(BASE),
            "-c", str(SDK_SOURCE), "-o", str(obj),
        ], text=True, capture_output=True)
        self.assertEqual(result.returncode, 0, result.stdout + result.stderr)
        symbols = subprocess.run([nm, "-g", str(obj)], text=True,
                                 capture_output=True, check=True).stdout
        self.assertIn("lt_ota_family_amb1_sdk_anchor", symbols)
        self.assertIn("lt_ota_family_amb1_sdkops", symbols)


if __name__ == "__main__":
    unittest.main()
