"""Host contract for the AMB1 dual-OTA boot source probe.

Compiles the REAL ``lt_ota.c`` against an in-RAM NOR mock and a host
``device_lock`` mock. Proves:

  * slot A/new + slot B/old reports 0, slot B/new + slot A/old reports 1;
  * every ambiguous selector state fails closed to 0 (both new, both old,
    erased, garbage, torn signature, read failure, non-1 SDK return);
  * the result is latched for the boot, so an activation that moves the new
    signature cannot change the reported running slot;
  * ``lt_ota_is_valid()`` follows the same signature rule and rejects
    out-of-range indices;
  * the probe uses raw SDK reads under the shared flash lock, not the AmebaZ
    XIP/address inference, and leaves ``lt_ota_dual_get_stored()`` /
    ``lt_ota_switch()`` as the documented fail-closed stubs.

One scenario per process because the running-slot answer is latched.

Run: python3 -m unittest discover -s libretiny/cores/realtek-amb1/tests \
        -p test_ota_boot_probe.py -v
"""
from pathlib import Path
import shutil
import subprocess
import tempfile
import unittest


ROOT = Path(__file__).resolve().parents[3]
API = ROOT / "cores/realtek-amb1/base/api"
SOURCE = API / "lt_ota.c"

LIBRETINY_H = r"""
#ifndef MOCK_LIBRETINY_H
#define MOCK_LIBRETINY_H
#include <stdbool.h>
#include <stdint.h>
#include <string.h>
typedef enum { OTA_TYPE_SINGLE = 0, OTA_TYPE_DUAL } lt_ota_type_t;
lt_ota_type_t lt_ota_get_type(void);
bool lt_ota_is_valid(uint8_t index);
uint8_t lt_ota_dual_get_current(void);
uint8_t lt_ota_dual_get_stored(void);
bool lt_ota_switch(bool revert);
#endif
"""

DEVICE_LOCK_H = r"""
#ifndef MOCK_DEVICE_LOCK_H
#define MOCK_DEVICE_LOCK_H
typedef enum { RT_DEV_LOCK_FLASH = 2, RT_DEV_LOCK_MAX = 5 } RT_DEV_LOCK_E;
void device_mutex_lock(RT_DEV_LOCK_E device);
void device_mutex_unlock(RT_DEV_LOCK_E device);
#endif
"""

FLASH_API_H = r"""
#ifndef MOCK_FLASH_API_H
#define MOCK_FLASH_API_H
#include <stdint.h>
typedef struct { int unused; } flash_t;
int flash_stream_read(flash_t *obj, uint32_t address, uint32_t len, uint8_t *out);
#endif
"""

HARNESS = r"""
#include <assert.h>
#include <stdint.h>
#include <string.h>
#include "device_lock.h"
#include "flash_api.h"
#include "libretiny.h"

#define FLASH_SIZE   0x00800000u
#define SLOT_A_START 0x0000B000u
#define SLOT_B_START 0x00177000u
#define SIGNATURE_OFFSET 8u

static uint8_t memory[FLASH_SIZE];
static int locked, locks, unlocks;
static unsigned reads;
static int read_result = 1;

static const uint8_t new_sig[8] = { '8', '1', '9', '5', '8', '7', '1', '1' };
static const uint8_t old_sig[8] = { '0', '1', '9', '5', '8', '7', '1', '1' };

static uint32_t slot_start(unsigned slot) {
    return slot ? SLOT_B_START : SLOT_A_START;
}

static void blank(void) {
    memset(memory, 255, sizeof(memory));
}

static void set_sig(unsigned slot, const uint8_t sig[8]) {
    memcpy(memory + slot_start(slot) + SIGNATURE_OFFSET, sig, 8);
}

static void balanced(void) {
    assert(!locked);
    assert(locks == unlocks);
}

void device_mutex_lock(RT_DEV_LOCK_E device) {
    assert(device == RT_DEV_LOCK_FLASH && !locked);
    locked = 1;
    ++locks;
}

void device_mutex_unlock(RT_DEV_LOCK_E device) {
    assert(device == RT_DEV_LOCK_FLASH && locked);
    locked = 0;
    ++unlocks;
}

/* The probe may only read the two fixed signature marks, and only while it
 * holds the shared flash lock. Success is exactly 1, matching the SDK. */
int flash_stream_read(flash_t *obj, uint32_t address, uint32_t len,
                      uint8_t *out) {
    assert(locked && obj && len == 8);
    assert(address == SLOT_A_START + SIGNATURE_OFFSET ||
           address == SLOT_B_START + SIGNATURE_OFFSET);
    ++reads;
    if (read_result != 1) return read_result;
    memcpy(out, memory + address, len);
    return 1;
}

int main(int argc, char **argv) {
    assert(argc == 2);
    blank();

    if (!strcmp(argv[1], "slot_a")) {
        set_sig(0, new_sig);
        set_sig(1, old_sig);
        assert(lt_ota_dual_get_current() == 0);
        assert(reads == 2);
    } else if (!strcmp(argv[1], "slot_b")) {
        set_sig(1, new_sig);
        set_sig(0, old_sig);
        assert(lt_ota_dual_get_current() == 1);
        assert(reads == 2);
    } else if (!strcmp(argv[1], "slot_b_erased_a")) {
        set_sig(1, new_sig);
        assert(lt_ota_dual_get_current() == 1);
    } else if (!strcmp(argv[1], "slot_a_erased_b")) {
        set_sig(0, new_sig);
        assert(lt_ota_dual_get_current() == 0);
    } else if (!strcmp(argv[1], "both_new")) {
        set_sig(0, new_sig);
        set_sig(1, new_sig);
        assert(lt_ota_dual_get_current() == 0);
    } else if (!strcmp(argv[1], "both_old")) {
        set_sig(0, old_sig);
        set_sig(1, old_sig);
        assert(lt_ota_dual_get_current() == 0);
    } else if (!strcmp(argv[1], "erased")) {
        assert(lt_ota_dual_get_current() == 0);
    } else if (!strcmp(argv[1], "garbage")) {
        memcpy(memory + SLOT_A_START + SIGNATURE_OFFSET, "12345678", 8);
        memcpy(memory + SLOT_B_START + SIGNATURE_OFFSET, "abcdefgh", 8);
        assert(lt_ota_dual_get_current() == 0);
    } else if (!strcmp(argv[1], "torn")) {
        /* A near-miss new signature in the only candidate, B old: no slot is
         * new, so the running slot is unconfirmed and must stay slot A. */
        memcpy(memory + SLOT_A_START + SIGNATURE_OFFSET, "8195871X", 8);
        set_sig(1, old_sig);
        assert(lt_ota_dual_get_current() == 0);
    } else if (!strcmp(argv[1], "read_fail_a")) {
        /* A is unreadable, B looks new: unconfirmed, so fail closed. */
        set_sig(0, new_sig);
        set_sig(1, new_sig);
        memcpy(memory + SLOT_A_START + SIGNATURE_OFFSET, "XXXXXXXX", 8);
        read_result = 0;
        assert(lt_ota_dual_get_current() == 0);
    } else if (!strcmp(argv[1], "read_short")) {
        set_sig(1, new_sig);
        read_result = 4; /* not the SDK's exactly-1 success */
        assert(lt_ota_dual_get_current() == 0);
    } else if (!strcmp(argv[1], "read_error")) {
        set_sig(1, new_sig);
        read_result = -1;
        assert(lt_ota_dual_get_current() == 0);
    } else if (!strcmp(argv[1], "latch")) {
        /* Running slot is A. Activation later publishes B and demotes A; the
         * reported running slot must not follow because the image is still A
         * until the reboot. */
        set_sig(0, new_sig);
        set_sig(1, old_sig);
        assert(lt_ota_dual_get_current() == 0);
        set_sig(0, old_sig);
        set_sig(1, new_sig);
        assert(lt_ota_dual_get_current() == 0);
        assert(reads == 2);
    } else if (!strcmp(argv[1], "latch_ambiguous")) {
        assert(lt_ota_dual_get_current() == 0);
        set_sig(0, new_sig); /* a later repair is too late for this boot */
        assert(lt_ota_dual_get_current() == 0);
        assert(reads == 2);
    } else if (!strcmp(argv[1], "is_valid")) {
        set_sig(0, old_sig);
        set_sig(1, new_sig);
        assert(!lt_ota_is_valid(0));
        assert(lt_ota_is_valid(1));
        assert(!lt_ota_is_valid(2));
        assert(!lt_ota_is_valid(255));
        assert(reads == 2);
    } else if (!strcmp(argv[1], "is_valid_fail")) {
        set_sig(0, new_sig);
        read_result = 0;
        assert(!lt_ota_is_valid(0));
    } else if (!strcmp(argv[1], "stubs")) {
        assert(lt_ota_get_type() == OTA_TYPE_DUAL);
        assert(lt_ota_dual_get_stored() == 0);
        assert(!lt_ota_switch(false));
        assert(!lt_ota_switch(true));
        assert(reads == 0);
    } else {
        return 2;
    }

    balanced();
    return 0;
}
"""


class OtaBootProbeTests(unittest.TestCase):
    @classmethod
    def setUpClass(cls):
        cc = shutil.which("cc")
        if not cc:
            raise unittest.SkipTest("host C compiler unavailable")
        cls.tmp = tempfile.TemporaryDirectory(prefix="ota-boot-probe-")
        cls.addClassCleanup(cls.tmp.cleanup)
        cls.work = Path(cls.tmp.name)
        (cls.work / "libretiny.h").write_text(LIBRETINY_H)
        (cls.work / "device_lock.h").write_text(DEVICE_LOCK_H)
        (cls.work / "flash_api.h").write_text(FLASH_API_H)
        (cls.work / "harness.c").write_text(HARNESS)
        cls.binary = cls.work / "test"
        result = subprocess.run([
            cc, "-std=c11", "-Wall", "-Wextra", "-Werror",
            "-I", str(cls.work), "-I", str(API),
            str(SOURCE), str(cls.work / "harness.c"),
            "-o", str(cls.binary),
        ], text=True, capture_output=True)
        if result.returncode:
            raise AssertionError(result.stdout + result.stderr)

    def run_mode(self, mode):
        result = subprocess.run([str(self.binary), mode], text=True,
                                capture_output=True)
        self.assertEqual(result.returncode, 0,
                         f"{mode}: {result.stderr.strip()}")

    # ------------------------------------------------------- slot selection
    def test_running_slot_follows_the_single_new_signature(self):
        for mode in ("slot_a", "slot_b", "slot_a_erased_b",
                     "slot_b_erased_a"):
            with self.subTest(mode=mode):
                self.run_mode(mode)

    def test_ambiguous_selector_states_fail_closed_to_slot_a(self):
        for mode in ("both_new", "both_old", "erased", "garbage", "torn",
                     "read_fail_a", "read_short", "read_error"):
            with self.subTest(mode=mode):
                self.run_mode(mode)

    def test_running_slot_is_latched_for_the_boot(self):
        for mode in ("latch", "latch_ambiguous"):
            with self.subTest(mode=mode):
                self.run_mode(mode)

    # -------------------------------------------------------------- validity
    def test_is_valid_uses_the_same_signature_rule(self):
        self.run_mode("is_valid")
        self.run_mode("is_valid_fail")

    # ----------------------------------------------------------- stubs/source
    def test_switch_and_stored_remain_fail_closed(self):
        self.run_mode("stubs")

    def test_probe_does_not_use_xip_address_inference(self):
        source = SOURCE.read_text()
        self.assertIn("flash_stream_read", source)
        self.assertIn("RT_DEV_LOCK_FLASH", source)
        self.assertNotIn("lt_log", source)
        self.assertNotIn("SPI_FLASH_BASE", source)
        self.assertIn("AMB1_SLOT_A_START", source)
        self.assertIn("0x0000B000u", source)
        self.assertIn("AMB1_SLOT_B_START", source)
        self.assertIn("0x00177000u", source)


if __name__ == "__main__":
    unittest.main()
