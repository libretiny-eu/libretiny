"""Host-only binding contract checks; no hardware qualification or flash I/O."""
import pathlib
import shutil
import subprocess
import tempfile
import unittest

ROOT = pathlib.Path(__file__).resolve().parents[3]
PORT = ROOT / "cores/realtek-amb1/base/port"
SOURCE = PORT / "preferences_profile.c"
FACTS = (
    "HARDWARE_QUALIFIED", "REVIEWED_RESERVATION", "MIGRATED_NATIVE_LOADER",
    "NORMAL_SELECTOR", "INACTIVE_B_RESERVATION", "ADMITTED_IMAGES_EXCLUDE_KVS",
    "BASELINE_MIGRATION_ATTESTED", "SHARED_LOCK_ATTESTED", "PROFILE_TABLE_REVIEWED",
)


@unittest.skipUnless(shutil.which("cc"), "C compiler required")
class ProfileContract(unittest.TestCase):
    def compile(self, *, selected=True, missing=None, value=None, prepare=1,
                existing=1, formatting=0, flash_length="0x800000", hash_size=32,
                experimental=1, profile=1, attestation_header=True):
        with tempfile.TemporaryDirectory() as tmp:
            work = pathlib.Path(tmp)
            (work / "libretiny.h").write_text(
                f"#define FLASH_LENGTH {flash_length}\n#define LT_E(...) ((void)0)\n")
            (work / "device_lock.h").write_text(
                "#define RT_DEV_LOCK_FLASH 7\nint device_mutex_prepare(int);\n")
            (work / "FreeRTOS.h").write_text(
                "#include <stddef.h>\ntypedef void *TaskHandle_t;\n"
                "unsigned __get_IPSR(void);\n")
            (work / "task.h").write_text(
                "#define taskSCHEDULER_NOT_STARTED 0\n#define taskSCHEDULER_SUSPENDED 1\n"
                "#define taskSCHEDULER_RUNNING 2\nint xTaskGetSchedulerState(void);\n"
                "TaskHandle_t xTaskGetCurrentTaskHandle(void);\n")
            facts = {f"LT_AMB1_KVS_{name}": 1 for name in FACTS}
            facts.update(LT_AMB1_KVS_EXISTING_FLASHDB_ATTESTED=existing,
                         LT_AMB1_KVS_INITIAL_FORMAT_APPROVED=formatting)
            if missing:
                if value is None:
                    facts.pop(f"LT_AMB1_KVS_{missing}")
                else:
                    facts[f"LT_AMB1_KVS_{missing}"] = value
            # Synthetic identifier, deliberately unrelated to any device evidence.
            header = "".join(f"#define {key} {val}\n" for key, val in facts.items())
            header += "#define LT_AMB1_KVS_REVIEWED_BASELINE_SHA256_BYTES "
            header += ",".join(str(i + 1) for i in range(hash_size)) + "\n"
            (work / "attest.h").write_text(header)
            harness = r'''
#include <assert.h>
#include <string.h>
#include "preferences_sdk.h"
#include <FreeRTOS.h>
#include <task.h>
static int calls, prepared;
static int scheduler = taskSCHEDULER_NOT_STARTED, isr;
static TaskHandle_t current = (void *)1;
unsigned __get_IPSR(void) { return isr; }
int xTaskGetSchedulerState(void) { return scheduler; }
TaskHandle_t xTaskGetCurrentTaskHandle(void) { return current; }
int device_mutex_prepare(int id) { assert(id == 7); prepared++; return PREPARE; }
int amb1_preferences_sdk_bind(const amb1_preferences_sdk_policy_t *p) {
    assert(prepared == 1);
    assert(p->runtime_qualification_passed == 1);
    assert(amb1_preferences_layout_reviewed(&p->flash.review));
    assert(p->flash.fresh_snapshot_attested == 1);
    assert(p->flash.shared_controller_lock_attested == 1);
    assert(p->flash.existing_flashdb_attested == EXISTING);
    assert(p->flash.initial_format_approved == FORMATTING);
    assert(!memcmp(p->flash.expected_snapshot_hash, p->flash.fresh_snapshot_hash, 32));
    for (int i = 0; i < 32; i++) assert(p->flash.expected_snapshot_hash[i] == i + 1);
    assert(p->current_geometry.flash_size == 0x800000);
    assert(p->current_geometry.kvs_start == 0x275000);
    assert(p->current_geometry.kvs_end == 0x277000);
    assert(p->current_geometry.system_start == 0x2e3000);
    assert(p->current_geometry.image_max == 0xfe000);
    calls++;
    return 0;
}
void lt_amb1_preferences_configure(void);
int main(void) {
    for (; scheduler < taskSCHEDULER_RUNNING; ++scheduler) {
        assert(current != NULL);
        lt_amb1_preferences_configure();
        assert(!prepared && !calls);
    }
    isr = 1; lt_amb1_preferences_configure(); isr = 0;
    current = NULL; lt_amb1_preferences_configure(); current = (void *)1;
    assert(!prepared && !calls);
    lt_amb1_preferences_configure();
    assert(calls == (PREPARE == 1));
    return 0;
}
'''
            (work / "harness.c").write_text(harness)
            cmd = ["cc", "-std=c11", "-Wall", "-Wextra", "-Werror", "-I", str(work),
                   "-I", str(PORT), str(SOURCE)]
            if selected:
                cmd += [f"-DLT_AMB1_EXPERIMENTAL_KVS={experimental}",
                        f"-DLT_AMB1_WCATA007_KVS_PROFILE={profile}",
                        f"-DPREPARE={prepare}", f"-DEXISTING={existing}",
                        f"-DFORMATTING={formatting}"]
                if attestation_header:
                    cmd += ['-DLT_AMB1_KVS_ATTESTATION_HEADER="attest.h"']
                if experimental and profile:
                    cmd += [str(work / "harness.c")]
                else:
                    cmd += ["-c"]
            else:
                cmd += ["-c"]
            cmd += ["-o", str(work / "result")]
            result = subprocess.run(cmd, capture_output=True, text=True)
            enabled = selected and experimental and profile
            if result.returncode == 0 and enabled:
                subprocess.run([str(work / "result")], check=True)
            if result.returncode == 0 and not enabled:
                symbols = subprocess.run(["nm", str(work / "result")], capture_output=True,
                                         text=True, check=True).stdout
                self.assertNotIn("lt_amb1_preferences_configure", symbols)
            return result

    def test_default_off(self):
        self.assertEqual(self.compile(selected=False).returncode, 0)
        self.assertEqual(self.compile(experimental=0).returncode, 0)
        self.assertEqual(self.compile(profile=0).returncode, 0)

    def test_external_header_required(self):
        self.assertNotEqual(self.compile(attestation_header=False).returncode, 0)

    def test_each_missing_or_false_attestation_fails_closed(self):
        for name in FACTS:
            for value in (None, 0, 2):
                with self.subTest(name=name, value=value):
                    self.assertNotEqual(self.compile(missing=name, value=value).returncode, 0)

    def test_explicit_mock_policy_and_mutex_failure(self):
        for prepare in (0, 1, -1):
            with self.subTest(prepare=prepare):
                self.assertEqual(self.compile(prepare=prepare).returncode, 0)

    def test_first_format_and_subsequent_no_format_are_distinct(self):
        self.assertEqual(self.compile(existing=0, formatting=1).returncode, 0)
        self.assertEqual(self.compile(existing=1, formatting=0).returncode, 0)
        self.assertNotEqual(self.compile(existing=0, formatting=0).returncode, 0)
        self.assertNotEqual(self.compile(existing=1, formatting=2).returncode, 0)
        for name in ("EXISTING_FLASHDB_ATTESTED", "INITIAL_FORMAT_APPROVED"):
            self.assertNotEqual(self.compile(missing=name).returncode, 0)

    def test_compile_time_geometry_and_identifier_size(self):
        self.assertNotEqual(self.compile(flash_length="0x400000").returncode, 0)
        self.assertNotEqual(self.compile(hash_size=31).returncode, 0)
        self.assertNotEqual(self.compile(hash_size=33).returncode, 0)


if __name__ == "__main__":
    unittest.main()
