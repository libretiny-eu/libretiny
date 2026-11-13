#!/usr/bin/env python3
"""End-to-end host contract: real bridge + session + SDK adapter.

Unlike ``test_ota_family_amb1.py`` (which injects a mock sdkops), this test links
the REAL ``lt_ota_family_amb1.c``, the REAL ``lt_ota_family_amb1_sdk.c`` strong
binding, the REAL image2 session/backend stack and the REAL
``image2_sdk_adapter.c``. The only substitutes are the in-RAM NOR, the host
device-lock/task primitives and the boot-source report.

It proves the activation path that staging-only tests cannot:

  * ``begin()`` prepares the shared flash mutex and binds the adapter;
  * staging erases/programs only the observed inactive slot;
  * ``end()`` publishes the target signature and then demotes the running
    signature through the dedicated one-byte adapter capability;
  * no generic program/erase ever touches the running slot, boot or KVS;
  * a missing mutex keeps the seam fail-closed before any flash I/O.

Run: python3 -m unittest discover -s libretiny/cores/realtek-amb1/tests \
        -p test_ota_family_amb1_adapter_e2e.py -v
"""
import os
from pathlib import Path
import shutil
import subprocess
import tempfile
import unittest


ROOT = Path(__file__).resolve().parents[3]
OTA = ROOT / "cores/realtek-amb1/base/ota"
COMMON = ROOT / "cores/common/base/api"
SDK_COMPONENT = Path("component/common/network/ssl/mbedtls-2.4.0")

HANDLED = 0x1000
WRITE_FAILED = 13


def find_sdk():
    candidates = []
    if os.environ.get("AMB1_SDK_DIR"):
        candidates.append(Path(os.environ["AMB1_SDK_DIR"]).expanduser())
    candidates.extend((
        Path.home() / ".platformio/packages/framework-realtek-amb1",
        ROOT.parent / "fridge-local/esphome/.platformio/packages/framework-realtek-amb1",
    ))
    for package in candidates:
        sdk = package / SDK_COMPONENT
        if (sdk / "library/sha256.c").is_file() and (sdk / "include/mbedtls/sha256.h").is_file():
            return sdk
    return None


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

LIBRETINY_H = r"""
#pragma once
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
"""

UF2TYPES_H = r"""
#pragma once
#include <stdbool.h>
#include <stdint.h>
#define UF2_MAGIC_1 0x0A324655
#define UF2_MAGIC_2 0x9E5D5157
#define UF2_MAGIC_3 0x0AB16F30
#define UF2_BLOCK_SIZE sizeof(uf2_block_t)
#define UF2OTA_VERSION 50000
struct fal_partition;
typedef struct __attribute__((packed)) {
    uint32_t magic1;
    uint32_t magic2;
    bool not_main_flash : 1;
    uint16_t dummy1 : 11;
    bool file_container : 1;
    bool has_family_id : 1;
    bool has_md5 : 1;
    bool has_tags : 1;
    uint16_t dummy2 : 16;
    uint32_t addr;
    uint32_t len;
    uint32_t block_seq;
    uint32_t block_count;
    uint32_t file_size;
    uint8_t data[476];
    uint32_t magic3;
} uf2_block_t;
typedef struct { uint32_t start, end; } uf2_wp_t;
typedef struct {
    uint32_t seq, family_id, written;
    bool is_format_ok, is_part_set;
    const uint8_t *binpatch;
    uint8_t binpatch_len;
    uint8_t scheme_byte, scheme_shift;
    bool scheme_binpatch;
    uint32_t erased_offset, erased_length;
    uf2_wp_t *wp;
    uint8_t wp_len;
    struct fal_partition *part_table;
    uint32_t part_table_len;
    bool part_table_copied;
    const struct fal_partition *part;
    const struct fal_flash_dev *flash;
} uf2_ota_t;
typedef struct {
    char *fw_name;
    char *fw_version;
    char *lt_version;
    char *board;
} uf2_info_t;
typedef enum {
    UF2_SCHEME_DEVICE_SINGLE = 0,
    UF2_SCHEME_DEVICE_DUAL_1 = 1,
    UF2_SCHEME_DEVICE_DUAL_2 = 2,
    UF2_SCHEME_FLASHER_SINGLE = 3,
    UF2_SCHEME_FLASHER_DUAL_1 = 4,
    UF2_SCHEME_FLASHER_DUAL_2 = 5,
} uf2_ota_scheme_t;
typedef enum {
    UF2_ERR_OK = 0,
    UF2_ERR_IGNORE = 1,
    UF2_ERR_MAGIC = 2,
    UF2_ERR_FAMILY = 3,
    UF2_ERR_NOT_HEADER = 4,
    UF2_ERR_OTA_VER = 5,
    UF2_ERR_OTA_WRONG = 6,
    UF2_ERR_PART_404 = 7,
    UF2_ERR_PART_INVALID = 8,
    UF2_ERR_PART_UNSET = 9,
    UF2_ERR_DATA_TOO_LONG = 10,
    UF2_ERR_SEQ_MISMATCH = 11,
    UF2_ERR_ERASE_FAILED = 12,
    UF2_ERR_WRITE_FAILED = 13,
    UF2_ERR_WRITE_LENGTH = 14,
    UF2_ERR_WRITE_PROTECT = 15,
    UF2_ERR_ALLOC_FAILED = 16,
} uf2_err_t;
"""

HARNESS = r"""
#include <lt_ota.h>
#include <lt_ota_family_amb1_sdk.h>

#include <assert.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "device_lock.h"
#include "flash_api.h"

#define FLASH_SIZE   0x00800000u
#define SECTOR_SIZE  0x00001000u
#define SLOT_A_START 0x0000B000u
#define SLOT_A_END   0x0010B000u
#define SLOT_B_START 0x00177000u
#define SLOT_B_END   0x00275000u
#define KVS_START    0x00275000u
#define IMAGE_SIZE   1024u
#define DATA_SIZE    256u
#define UF2_TOTAL    ((IMAGE_SIZE / DATA_SIZE + 1u) * 512u)

static uint8_t flash[FLASH_SIZE];
static uint8_t image[IMAGE_SIZE];

static unsigned current_slot;
static int mutex_held, mutex_init, prepare_ok = 1, ipsr;
static unsigned reads, programs, erases, demotes, writes_outside;
static unsigned erases_outside, running_writes;

static uint32_t tgt, tgt_end, act;

static void put32(uint8_t *p, uint32_t v) {
    p[0] = (uint8_t)v;
    p[1] = (uint8_t)(v >> 8);
    p[2] = (uint8_t)(v >> 16);
    p[3] = (uint8_t)(v >> 24);
}

static void build_image(void) {
    unsigned i;
    memset(image, 0, sizeof(image));
    put32(image + 0, 10);
    put32(image + 4, 0x10006000u);
    memcpy(image + 8, "81958711", 8);
    put32(image + 16, 0x10006001u);
    memcpy(image + 20, "RTKWin", 6);
    put32(image + 26, IMAGE_SIZE - 26u - 16u);
    put32(image + 30, 0x30000000u);
    memset(image + 34, 0xFF, 8);
    for (i = 42; i < IMAGE_SIZE; i++) image[i] = (uint8_t)(i * 7u + 3u);
}

static void setup(unsigned running) {
    current_slot = running;
    tgt = running ? SLOT_A_START : SLOT_B_START;
    tgt_end = running ? SLOT_A_END : SLOT_B_END;
    act = running ? SLOT_B_START : SLOT_A_START;
    memset(flash, 0xFF, sizeof(flash));
    build_image();
    memcpy(flash + act + 8, "81958711", 8);
}

uint8_t lt_ota_dual_get_current(void) { return (uint8_t)current_slot; }
int mock_ipsr(void) { return ipsr; }
void *xTaskGetCurrentTaskHandle(void) { return (void *)(uintptr_t)1; }

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
    memcpy(data, flash + a, n);
    return 1;
}

void __real_flash_erase_sector(flash_t *obj, uint32_t a) {
    (void)obj;
    assert(mutex_held && a % SECTOR_SIZE == 0 && a + SECTOR_SIZE <= FLASH_SIZE);
    ++erases;
    if (a < tgt || a + SECTOR_SIZE > tgt_end) ++erases_outside;
    memset(flash + a, 0xFF, SECTOR_SIZE);
}

int __real_flash_burst_write(flash_t *obj, uint32_t a, uint32_t n, uint8_t *data) {
    int demote;
    (void)obj;
    assert(mutex_held && n && n <= 256);
    ++programs;
    demote = a == act + 8u && n == 1u && data[0] == '0';
    if (demote) ++demotes;
    if (a < tgt || a + n > tgt_end) {
        if (!demote) ++writes_outside;
    }
    if (a >= act && a < act + 16u && !demote) ++running_writes;
    for (uint32_t i = 0; i < n; ++i) {
        if ((flash[a + i] & data[i]) != data[i]) return 0; /* NOR 0 -> 1 */
        flash[a + i] &= data[i];
    }
    return 1;
}

static uf2_block_t make_block(uint32_t seq, uint32_t count,
                              uint32_t address, uint32_t length) {
    uf2_block_t b;
    memset(&b, 0, sizeof(b));
    b.magic1 = UF2_MAGIC_1;
    b.magic2 = UF2_MAGIC_2;
    b.magic3 = UF2_MAGIC_3;
    b.has_family_id = true;
    b.block_seq = seq;
    b.block_count = count;
    b.addr = address;
    b.len = length;
    if (address <= IMAGE_SIZE && length <= IMAGE_SIZE - address &&
        length <= sizeof(b.data))
        memcpy(b.data, image + address, length);
    return b;
}

static int do_begin(size_t size) {
    lt_ota_ctx_t ctx;
    memset(&ctx, 0, sizeof(ctx));
    return lt_ota_family_begin(&ctx, size);
}

static int do_write_at(uint32_t seq, uint32_t address) {
    lt_ota_ctx_t ctx;
    uf2_block_t b = make_block(seq, UF2_TOTAL / 512u, address, DATA_SIZE);
    memset(&ctx, 0, sizeof(ctx));
    return lt_ota_family_write_block(&ctx, &b);
}

static int do_end(void) {
    lt_ota_ctx_t ctx;
    memset(&ctx, 0, sizeof(ctx));
    return lt_ota_family_end(&ctx);
}

static void do_abort(void) {
    lt_ota_ctx_t ctx;
    memset(&ctx, 0, sizeof(ctx));
    lt_ota_family_abort(&ctx);
}

static int target_new(void) {
    return memcmp(flash + tgt + 8, "81958711", 8) == 0;
}

static int target_erased(void) {
    static const uint8_t erased[8] = {255, 255, 255, 255, 255, 255, 255, 255};
    return memcmp(flash + tgt + 8, erased, 8) == 0;
}

static int source_new(void) {
    return memcmp(flash + act + 8, "81958711", 8) == 0;
}

static int source_old(void) {
    return memcmp(flash + act + 8, "01958711", 8) == 0;
}

static int data_ok(void) {
    return memcmp(flash + tgt, image, 8) == 0 &&
           memcmp(flash + tgt + 16, image + 16, IMAGE_SIZE - 16) == 0;
}

static void status(void) {
    printf(" programs=%u erases=%u demotes=%u reads=%u outside=%u"
           " running_writes=%u erases_outside=%u"
           " target_new=%d target_erased=%d source_new=%d source_old=%d data=%d\n",
           programs, erases, demotes, reads, writes_outside,
           running_writes, erases_outside,
           target_new(), target_erased(), source_new(), source_old(), data_ok());
}

int main(int argc, char **argv) {
    int begin, end, i, w[4];
    if (argc < 2) return 2;

    if (!strcmp(argv[1], "happy") && argc == 3) {
        setup((unsigned)atoi(argv[2]));
        begin = do_begin(UF2_TOTAL);
        for (i = 0; i < 4; i++)
            w[i] = do_write_at((uint32_t)i + 1u, (uint32_t)i * DATA_SIZE);
        end = do_end();
        printf("mode=happy begin=%d w0=%d w1=%d w2=%d w3=%d end=%d",
               begin, w[0], w[1], w[2], w[3], end);
        status();
        return 0;
    }
    if (!strcmp(argv[1], "abort")) {
        setup(0);
        begin = do_begin(UF2_TOTAL);
        for (i = 0; i < 3; i++)
            w[i] = do_write_at((uint32_t)i + 1u, (uint32_t)i * DATA_SIZE);
        w[3] = 0;
        do_abort();
        end = do_end();
        printf("mode=abort begin=%d w0=%d w1=%d w2=%d end=%d",
               begin, w[0], w[1], w[2], end);
        status();
        return 0;
    }
    if (!strcmp(argv[1], "no_prepare")) {
        setup(0);
        prepare_ok = 0;
        begin = do_begin(UF2_TOTAL);
        w[0] = do_write_at(1, 0);
        end = do_end();
        printf("mode=no_prepare begin=%d w0=%d end=%d mutex_init=%d",
               begin, w[0], end, mutex_init);
        status();
        return 0;
    }
    return 2;
}
"""


class OtaFamilyAmb1AdapterE2ETests(unittest.TestCase):
    @classmethod
    def setUpClass(cls):
        cc = shutil.which("cc")
        if not cc:
            raise unittest.SkipTest("host C compiler unavailable")
        sdk = find_sdk()
        if sdk is None:
            raise unittest.SkipTest(
                "framework-realtek-amb1 mbedTLS sources not installed; set "
                "AMB1_SDK_DIR to the framework package directory"
            )

        cls.tmp = tempfile.TemporaryDirectory(prefix="amb1-ota-adapter-e2e-")
        cls.addClassCleanup(cls.tmp.cleanup)
        tmp = Path(cls.tmp.name)
        (tmp / "uf2ota").mkdir()
        (tmp / "libretiny.h").write_text(LIBRETINY_H)
        (tmp / "uf2ota" / "uf2types.h").write_text(UF2TYPES_H)
        (tmp / "mbedtls_config.h").write_text("#define MBEDTLS_SHA256_C\n")
        (tmp / "device_lock.h").write_text(DEVICE_LOCK_H)
        (tmp / "ameba_soc.h").write_text(AMEDA_SOC_H)
        (tmp / "flash_api.h").write_text(FLASH_API_H)
        (tmp / "FreeRTOS.h").write_text(FREERTOS_H)
        (tmp / "task.h").write_text(TASK_H)
        (tmp / "harness.c").write_text(HARNESS)

        sources = [
            OTA / "image2_transaction.c",
            OTA / "image2_flash_backend.c",
            OTA / "image2_activation.c",
            OTA / "image2_session.c",
            OTA / "image2_integrity.c",
            OTA / "lt_ota_family_amb1.c",
            OTA / "lt_ota_family_amb1_sdk.c",
            OTA / "image2_sdk_adapter.c",
            sdk / "library/sha256.c",
            tmp / "harness.c",
        ]
        objects = []
        for source in sources:
            obj = tmp / (source.stem + ".o")
            subprocess.run(
                [cc, "-std=c11", "-O2", "-Wall", "-Wextra", "-Werror",
                 "-Wno-unused-function", "-Wno-unused-parameter",
                 "-DLT_AMB1_EXPERIMENTAL_IMAGE2_SDK_ADAPTER=1",
                 '-DMBEDTLS_CONFIG_FILE="mbedtls_config.h"',
                 "-I" + str(tmp), "-I" + str(OTA), "-I" + str(COMMON),
                 "-I" + str(sdk / "include"),
                 "-c", str(source), "-o", str(obj)],
                check=True, capture_output=True, text=True,
            )
            objects.append(str(obj))

        cls.binary = tmp / "ota_adapter_e2e"
        subprocess.run(
            [cc, *objects, "-o", str(cls.binary)],
            check=True, capture_output=True, text=True,
        )

    def run_mode(self, *args):
        result = subprocess.run(
            [str(self.binary), *args], capture_output=True, text=True
        )
        if result.returncode != 0:
            self.fail(
                f"harness {args} exited {result.returncode}: {result.stderr.strip()}"
            )
        fields = {}
        for field in result.stdout.split():
            key, value = field.split("=", 1)
            fields[key] = int(value) if value.lstrip("-").isdigit() else value
        return fields

    def test_end_to_end_activation_for_both_running_slots(self):
        for running in ("0", "1"):
            with self.subTest(running=running):
                r = self.run_mode("happy", running)
                self.assertEqual(r["begin"], HANDLED)
                for key in ("w0", "w1", "w2", "w3", "end"):
                    self.assertEqual(r[key], HANDLED, key)
                self.assertEqual(r["demotes"], 1)
                self.assertEqual(r["outside"], 0)
                self.assertEqual(r["running_writes"], 0)
                self.assertEqual(r["erases_outside"], 0)
                self.assertGreater(r["programs"], 0)
                self.assertEqual(r["target_new"], 1)
                self.assertEqual(r["target_erased"], 0)
                self.assertEqual(r["source_old"], 1)
                self.assertEqual(r["data"], 1)

    def test_abort_leaves_the_running_slot_untouched(self):
        r = self.run_mode("abort")
        self.assertEqual(r["begin"], HANDLED)
        for key in ("w0", "w1", "w2"):
            self.assertEqual(r[key], HANDLED, key)
        self.assertEqual(r["end"], WRITE_FAILED)
        self.assertEqual(r["demotes"], 0)
        self.assertEqual(r["outside"], 0)
        self.assertEqual(r["running_writes"], 0)
        self.assertEqual(r["target_new"], 0)
        self.assertEqual(r["target_erased"], 1)
        self.assertEqual(r["source_new"], 1)

    def test_mutex_gate_refuses_every_operation_before_flash_io(self):
        r = self.run_mode("no_prepare")
        self.assertEqual(r["begin"], WRITE_FAILED)
        self.assertEqual(r["w0"], WRITE_FAILED)
        self.assertEqual(r["end"], WRITE_FAILED)
        self.assertEqual(r["reads"], 0)
        self.assertEqual(r["programs"], 0)
        self.assertEqual(r["erases"], 0)
        self.assertEqual(r["demotes"], 0)


if __name__ == "__main__":
    unittest.main(verbosity=2)
