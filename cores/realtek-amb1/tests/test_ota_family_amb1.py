#!/usr/bin/env python3
"""Host tests for the AMB1 family OTA lifecycle hook.

The REAL image2 transaction/session/activation/backend/integrity modules and the
REAL ``lt_ota_family_amb1.c`` binding are compiled against an in-RAM NOR mock.
No target compiler, hardware, network, credential or flash is used, and the
production SDK flash adapter is not linked.

The family owns one-shot per-boot state, so every scenario runs in a separate
process (``argv`` mode). Covered contract:

  * happy-path staging into the reviewed inactive slot, readback verification
    and signature activation for both running slots;
  * every program/erase stays inside the inactive target slot; the only write to
    the running slot is the single source-demote byte;
  * end() verifies + activates but never calls lt_ota_switch() or reboots;
  * abort() releases the session/integrity state exactly once and never
    activates, commits or reboots;
  * UF2 sequence/address/block-count contiguity, exact 256-byte payloads and
    family-owned main-flash tags are enforced before flash I/O;
  * the inactive-slot capacity boundary is the only admitted target size;
  * the running slot is frozen at begin(), even if the boot source changes;
  * short uploads and corrupted readbacks fail closed without publishing a
    signature.
"""

import hashlib
import os
from pathlib import Path
import re
import shutil
import subprocess
import tempfile
import unittest


ROOT = Path(__file__).resolve().parents[3]
OTA = ROOT / "cores/realtek-amb1/base/ota"
COMMON = ROOT / "cores/common/base/api"
FAMILY_C = OTA / "lt_ota_family_amb1.c"
SDK_COMPONENT = Path("component/common/network/ssl/mbedtls-2.4.0")

HANDLED = 0x1000
WRITE_FAILED = 13
WRITE_LENGTH = 14
SEQ_MISMATCH = 11


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

HARNESS_C = r"""
/* Per-process host harness for the AMB1 family hook. One scenario per process
 * because the family latches one session, one boot object and one integrity
 * context for the whole boot. */
#include <lt_ota.h>
#include <image2_flash_backend.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define FLASH_SIZE   0x00800000u
#define SECTOR_SIZE  0x00001000u
#define SLOT_A_START 0x0000B000u
#define SLOT_A_END   0x0010B000u
#define SLOT_B_START 0x00177000u
#define SLOT_B_END   0x00275000u
#define IMAGE_SIZE   1024u
#define DATA_SIZE    256u
#define UF2_TOTAL    ((IMAGE_SIZE / DATA_SIZE + 1u) * 512u)

static uint8_t flash[FLASH_SIZE];
static uint8_t image[IMAGE_SIZE];

static unsigned current_slot;
static int sdkops_enabled = 1;

/* Geometry captured once per scenario so later boot-source changes cannot hide
 * a write outside the intended target. */
static uint32_t tgt, tgt_end, act, act_end;

static unsigned lock_calls, unlock_calls, program_calls, erase_calls, read_calls;
static unsigned demote_calls, programs_outside, programs_running;
static unsigned erases_outside;

static void put32(uint8_t *p, uint32_t v) {
    p[0] = (uint8_t)v;
    p[1] = (uint8_t)(v >> 8);
    p[2] = (uint8_t)(v >> 16);
    p[3] = (uint8_t)(v >> 24);
}

/* A minimal but structurally exact AMB1 image2 payload (see
 * fridge-local/tools/amb1/package_elf.py):
 *   [0..4)   SRAM length (10)
 *   [4..8)   SRAM base (0x10006000)
 *   [8..16)  boot signature (written to the transaction, never to flash)
 *   [16..20) Thumb entry point (first word of the SRAM payload)
 *   [20..26) "RTKWin"
 *   [26..30) SDRAM length (dram)
 *   [30..34) SDRAM base
 *   [34..42) reserved erased bytes
 *   [42..)   SDRAM content
 */
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
    act_end = running ? SLOT_B_END : SLOT_A_END;
    memset(flash, 0xFF, sizeof(flash));
    build_image();
    /* The running slot must carry a valid new signature; the target stays blank. */
    memcpy(flash + act + 8, "81958711", 8);
}

static int sdk_try_lock(void *ctx, const void *owner) {
    (void)ctx;
    (void)owner;
    lock_calls++;
    return 1;
}

static void sdk_unlock(void *ctx, const void *owner) {
    (void)ctx;
    (void)owner;
    unlock_calls++;
}

static int sdk_read(void *ctx, uint32_t address, void *out, size_t n) {
    (void)ctx;
    read_calls++;
    if (address > FLASH_SIZE || n > FLASH_SIZE - address) return 0;
    memcpy(out, flash + address, n);
    return 1;
}

static int sdk_program(void *ctx, uint32_t address, const void *in, size_t n) {
    const uint8_t *p = in;
    size_t i;
    int inside, demote;
    (void)ctx;
    program_calls++;
    if (address > FLASH_SIZE || n > FLASH_SIZE - address) return 0;
    inside = address >= tgt && n <= tgt_end - address;
    demote = address == act + 8 && n == 1;
    if (!inside && !demote) programs_outside++;
    if (demote) demote_calls++;
    if (address >= act && address < act_end && !demote) programs_running++;
    for (i = 0; i < n; i++) {
        if ((flash[address + i] & p[i]) != p[i]) return 0; /* NOR 0 -> 1 */
        flash[address + i] &= p[i];
    }
    return 1;
}

static int sdk_erase(void *ctx, uint32_t address) {
    (void)ctx;
    erase_calls++;
    if (address % SECTOR_SIZE || address > FLASH_SIZE - SECTOR_SIZE) return 0;
    if (address < tgt || address + SECTOR_SIZE > tgt_end) erases_outside++;
    memset(flash + address, 0xFF, SECTOR_SIZE);
    return 1;
}

static const image2_flash_sdkops sdk_ops = {
    NULL, sdk_try_lock, sdk_unlock, sdk_read, sdk_program, sdk_erase, NULL
};

const image2_flash_sdkops *lt_ota_family_amb1_sdkops(void) {
    return sdkops_enabled ? &sdk_ops : NULL;
}

uint8_t lt_ota_dual_get_current(void) {
    return (uint8_t)current_slot;
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

static uf2_block_t write_at(uint32_t seq, uint32_t address) {
    return make_block(seq, UF2_TOTAL / 512u, address, DATA_SIZE);
}

static int do_begin(size_t size) {
    lt_ota_ctx_t ctx;
    memset(&ctx, 0, sizeof(ctx));
    return lt_ota_family_begin(&ctx, size);
}

static int do_write(const uf2_block_t *block) {
    lt_ota_ctx_t ctx;
    memset(&ctx, 0, sizeof(ctx));
    return lt_ota_family_write_block(&ctx, block);
}

static int do_write_at(uint32_t seq, uint32_t address) {
    uf2_block_t block = write_at(seq, address);
    return do_write(&block);
}

static int do_write_raw(uint32_t seq, uint32_t count,
                        uint32_t address, uint32_t length) {
    uf2_block_t block = make_block(seq, count, address, length);
    return do_write(&block);
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
    printf(" programs=%u erases=%u locks=%u unlocks=%u demotes=%u reads=%u"
           " outside=%u running=%u erases_outside=%u"
           " target_new=%d target_erased=%d source_new=%d source_old=%d data=%d\n",
           program_calls, erase_calls, lock_calls, unlock_calls, demote_calls,
           read_calls, programs_outside, programs_running, erases_outside,
           target_new(), target_erased(), source_new(), source_old(), data_ok());
}

static int mode_happy(unsigned running, int after) {
    int begin, end, i;
    int w[4];
    int after_end = -1, after_write = -1;
    setup(running);
    begin = do_begin(UF2_TOTAL);
    for (i = 0; i < 4; i++)
        w[i] = do_write_at((uint32_t)i + 1u, (uint32_t)i * DATA_SIZE);
    end = do_end();
    if (after) {
        do_abort(); /* must be a no-op after a successful end */
        after_end = do_end();
        after_write = do_write_at(5, 4u * DATA_SIZE);
    }
    printf("mode=happy begin=%d w0=%d w1=%d w2=%d w3=%d end=%d after_end=%d after_write=%d",
           begin, w[0], w[1], w[2], w[3], end, after_end, after_write);
    status();
    return 0;
}

static int mode_freeze(void) {
    int begin, end, i;
    int w[4];
    setup(0);
    begin = do_begin(UF2_TOTAL);
    /* A boot-source change must not move the target captured at begin(). */
    current_slot = 1;
    for (i = 0; i < 4; i++)
        w[i] = do_write_at((uint32_t)i + 1u, (uint32_t)i * DATA_SIZE);
    end = do_end();
    printf("mode=freeze begin=%d w0=%d w1=%d w2=%d w3=%d end=%d",
           begin, w[0], w[1], w[2], w[3], end);
    status();
    return 0;
}

static int mode_latch(const char *variant) {
    int begin, w0 = -1, bad = -1, after, end;
    setup(0);
    begin = do_begin(UF2_TOTAL);
    if (!strcmp(variant, "seq_zero")) {
        bad = do_write_raw(0, 5, 0, DATA_SIZE);
    } else {
        w0 = do_write_at(1, 0);
        if (!strcmp(variant, "seq_skip"))
            bad = do_write_at(3, 2u * DATA_SIZE);
        else if (!strcmp(variant, "seq_dup"))
            bad = do_write_at(1, 0);
        else if (!strcmp(variant, "addr_wrong"))
            bad = do_write_raw(2, 5, 2u * DATA_SIZE, DATA_SIZE);
        else if (!strcmp(variant, "count_wrong"))
            bad = do_write_raw(2, 6, DATA_SIZE, DATA_SIZE);
        else if (!strcmp(variant, "addr_huge"))
            bad = do_write_raw(2, 5, 0x1000, DATA_SIZE);
        else
            return 2;
    }
    after = do_write_at(2, DATA_SIZE); /* latched: must not reach flash */
    end = do_end();
    do_abort();
    printf("mode=latch begin=%d w0=%d bad=%d w_after=%d end=%d",
           begin, w0, bad, after, end);
    status();
    return 0;
}

static int mode_length(const char *variant) {
    uf2_block_t block;
    int begin, bad, after, end;
    setup(0);
    begin = do_begin(UF2_TOTAL);
    if (!strcmp(variant, "short")) {
        block = make_block(1, 5, 0, 128);
        bad = do_write(&block);
    } else if (!strcmp(variant, "zero")) {
        block = make_block(1, 5, 0, 0);
        bad = do_write(&block);
    } else if (!strcmp(variant, "notmain")) {
        block = make_block(1, 5, 0, DATA_SIZE);
        block.not_main_flash = 1;
        bad = do_write(&block);
    } else if (!strcmp(variant, "container")) {
        block = make_block(1, 5, 0, DATA_SIZE);
        block.file_container = 1;
        bad = do_write(&block);
    } else if (!strcmp(variant, "null")) {
        bad = do_write(NULL);
    } else {
        return 2;
    }
    after = do_write_at(1, 0);
    end = do_end();
    do_abort();
    printf("mode=length begin=%d bad=%d w_after=%d end=%d", begin, bad, after, end);
    status();
    return 0;
}

static int mode_begin(const char *variant) {
    size_t size;
    unsigned running = 0;
    int begin;
    if (!strcmp(variant, "slot_limit_a") || !strcmp(variant, "slot_over_a"))
        running = 1;
    setup(running);
    if (!strcmp(variant, "sdkops")) {
        sdkops_enabled = 0;
        size = UF2_TOTAL;
    } else if (!strcmp(variant, "zero")) {
        size = 0;
    } else if (!strcmp(variant, "small")) {
        size = 511;
    } else if (!strcmp(variant, "misaligned")) {
        size = UF2_TOTAL + 1u;
    } else if (!strcmp(variant, "huge")) {
        size = (size_t)0x100000000ull;
    } else if (!strcmp(variant, "slot_limit")) {
        size = 4065u * 512u; /* image_size == 0xFE000, exactly slot B */
    } else if (!strcmp(variant, "slot_over")) {
        size = 4066u * 512u;
    } else if (!strcmp(variant, "slot_limit_a")) {
        size = 4097u * 512u; /* image_size == 0x100000, exactly slot A */
    } else if (!strcmp(variant, "slot_over_a")) {
        size = 4098u * 512u;
    } else {
        return 2;
    }
    begin = do_begin(size);
    do_abort();
    printf("mode=begin begin=%d", begin);
    status();
    return 0;
}

static int mode_abort(void) {
    int begin, w0, w1, write_after, end;
    unsigned programs_before, unlocks_before, programs_after, unlocks_after;
    unsigned unlocks_extra;
    setup(0);
    begin = do_begin(UF2_TOTAL);
    w0 = do_write_at(1, 0);
    w1 = do_write_at(2, DATA_SIZE);
    programs_before = program_calls;
    unlocks_before = unlock_calls;
    do_abort();
    programs_after = program_calls;
    unlocks_after = unlock_calls;
    write_after = do_write_at(3, 2u * DATA_SIZE);
    end = do_end();
    do_abort(); /* idempotent, no second release */
    unlocks_extra = unlock_calls - unlocks_after;
    printf("mode=abort begin=%d w0=%d w1=%d abort_programs=%d abort_unlocks=%u"
           " write_after=%d end=%d unlocks_extra=%u",
           begin, w0, w1, programs_after == programs_before,
           unlocks_after - unlocks_before, write_after, end, unlocks_extra);
    status();
    return 0;
}

static int mode_short(const char *variant) {
    int begin, w0 = -1, w1 = -1, w2 = -1, end;
    setup(0);
    begin = do_begin(UF2_TOTAL);
    if (strcmp(variant, "none")) {
        w0 = do_write_at(1, 0);
        w1 = do_write_at(2, DATA_SIZE);
        w2 = do_write_at(3, 2u * DATA_SIZE);
    }
    end = do_end();
    do_abort();
    printf("mode=short begin=%d w0=%d w1=%d w2=%d end=%d",
           begin, w0, w1, w2, end);
    status();
    return 0;
}

static int mode_corrupt(unsigned offset) {
    int begin, end, i;
    int w[4];
    setup(0);
    begin = do_begin(UF2_TOTAL);
    for (i = 0; i < 4; i++)
        w[i] = do_write_at((uint32_t)i + 1u, (uint32_t)i * DATA_SIZE);
    flash[tgt + offset] ^= 1u;
    end = do_end();
    do_abort();
    printf("mode=corrupt begin=%d w0=%d w1=%d w2=%d w3=%d end=%d offset=%u",
           begin, w[0], w[1], w[2], w[3], end, offset);
    status();
    return 0;
}

static int mode_double(void) {
    int first, second;
    setup(0);
    first = do_begin(UF2_TOTAL);
    second = do_begin(UF2_TOTAL);
    do_abort();
    printf("mode=double first=%d second=%d", first, second);
    status();
    return 0;
}

static int mode_early(void) {
    int end, write;
    setup(0);
    end = do_end();
    write = do_write_at(1, 0);
    do_abort();
    printf("mode=early end=%d write=%d", end, write);
    status();
    return 0;
}

int main(int argc, char **argv) {
    if (argc < 2) return 2;
    if (!strcmp(argv[1], "happy"))
        return mode_happy(argc > 2 ? (unsigned)atoi(argv[2]) : 0u, 0);
    if (!strcmp(argv[1], "after_end"))
        return mode_happy(argc > 2 ? (unsigned)atoi(argv[2]) : 0u, 1);
    if (!strcmp(argv[1], "freeze")) return mode_freeze();
    if (!strcmp(argv[1], "abort")) return mode_abort();
    if (!strcmp(argv[1], "double")) return mode_double();
    if (!strcmp(argv[1], "early")) return mode_early();
    if (argc < 3) return 2;
    if (!strcmp(argv[1], "latch")) return mode_latch(argv[2]);
    if (!strcmp(argv[1], "length")) return mode_length(argv[2]);
    if (!strcmp(argv[1], "begin")) return mode_begin(argv[2]);
    if (!strcmp(argv[1], "short")) return mode_short(argv[2]);
    if (!strcmp(argv[1], "corrupt")) return mode_corrupt((unsigned)strtoul(argv[2], NULL, 0));
    return 2;
}
"""


class OtaFamilyAmb1Tests(unittest.TestCase):
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

        cls.tmp = tempfile.TemporaryDirectory(prefix="ota-family-amb1-")
        cls.addClassCleanup(cls.tmp.cleanup)
        tmp = Path(cls.tmp.name)
        (tmp / "uf2ota").mkdir()
        (tmp / "libretiny.h").write_text(LIBRETINY_H)
        (tmp / "uf2ota" / "uf2types.h").write_text(UF2TYPES_H)
        (tmp / "mbedtls_config.h").write_text("#define MBEDTLS_SHA256_C\n")
        (tmp / "harness.c").write_text(HARNESS_C)

        sources = [
            OTA / "image2_transaction.c",
            OTA / "image2_flash_backend.c",
            OTA / "image2_activation.c",
            OTA / "image2_session.c",
            OTA / "image2_integrity.c",
            FAMILY_C,
            sdk / "library/sha256.c",
            tmp / "harness.c",
        ]
        objects = []
        for source in sources:
            obj = tmp / (source.stem + ".o")
            subprocess.run(
                [cc, "-std=c11", "-O2", "-Wall", "-Wextra", "-Werror",
                 '-DMBEDTLS_CONFIG_FILE="mbedtls_config.h"',
                 "-I" + str(tmp), "-I" + str(OTA), "-I" + str(COMMON),
                 "-I" + str(sdk / "include"),
                 "-c", str(source), "-o", str(obj)],
                check=True, capture_output=True, text=True,
            )
            objects.append(str(obj))

        cls.binary = tmp / "ota_family_amb1"
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

    # --------------------------------------------------------------- happy path
    def test_happy_path_activates_inactive_slot(self):
        for running in (0, 1):
            with self.subTest(running=running):
                r = self.run_mode("happy", str(running))
                self.assertEqual(r["begin"], HANDLED)
                for key in ("w0", "w1", "w2", "w3", "end"):
                    self.assertEqual(r[key], HANDLED, key)
                self.assertEqual(r["locks"], 1)
                self.assertEqual(r["unlocks"], 1)
                self.assertEqual(r["demotes"], 1)
                self.assertEqual(r["outside"], 0)
                self.assertEqual(r["running"], 0)
                self.assertEqual(r["erases_outside"], 0)
                self.assertEqual(r["erases"], 1)
                self.assertGreater(r["programs"], 0)
                self.assertEqual(r["target_new"], 1)
                self.assertEqual(r["target_erased"], 0)
                self.assertEqual(r["source_old"], 1)
                self.assertEqual(r["data"], 1)

    def test_end_is_not_a_reboot_and_finishes_the_boot(self):
        r = self.run_mode("after_end", "0")
        self.assertEqual(r["end"], HANDLED)
        self.assertEqual(r["unlocks"], 1)
        self.assertEqual(r["demotes"], 1)
        self.assertEqual(r["after_end"], WRITE_FAILED)
        self.assertEqual(r["after_write"], WRITE_FAILED)
        self.assertEqual(r["unlocks"], 1)  # abort after end released nothing
        self.assertEqual(r["target_new"], 1)
        self.assertEqual(r["source_old"], 1)

    def test_running_slot_is_frozen_at_begin(self):
        r = self.run_mode("freeze")
        self.assertEqual(r["begin"], HANDLED)
        self.assertEqual(r["end"], HANDLED)
        self.assertEqual(r["target_new"], 1)
        self.assertEqual(r["source_old"], 1)
        self.assertEqual(r["data"], 1)
        self.assertEqual(r["demotes"], 1)
        self.assertEqual(r["outside"], 0)
        self.assertEqual(r["running"], 0)

    # -------------------------------------------------------------- concurrency
    def test_contiguity_violations_latch(self):
        variants = ("seq_zero", "seq_skip", "seq_dup", "addr_wrong",
                    "count_wrong", "addr_huge")
        for variant in variants:
            with self.subTest(variant=variant):
                r = self.run_mode("latch", variant)
                self.assertEqual(r["begin"], HANDLED)
                if variant == "seq_zero":
                    self.assertEqual(r["w0"], -1)
                else:
                    self.assertEqual(r["w0"], HANDLED)
                self.assertEqual(r["bad"], SEQ_MISMATCH)
                self.assertEqual(r["w_after"], WRITE_FAILED)
                self.assertEqual(r["end"], WRITE_FAILED)
                self.assertEqual(r["target_new"], 0)
                self.assertEqual(r["target_erased"], 1)
                self.assertEqual(r["source_new"], 1)
                self.assertEqual(r["unlocks"], 1)

    def test_block_length_and_tag_violations(self):
        expected = {
            "short": WRITE_LENGTH,
            "zero": WRITE_FAILED,
            "notmain": WRITE_FAILED,
            "container": WRITE_FAILED,
            "null": WRITE_FAILED,
        }
        for variant, code in expected.items():
            with self.subTest(variant=variant):
                r = self.run_mode("length", variant)
                self.assertEqual(r["begin"], HANDLED)
                self.assertEqual(r["bad"], code)
                self.assertEqual(r["w_after"], WRITE_FAILED)
                self.assertEqual(r["end"], WRITE_FAILED)
                self.assertEqual(r["target_new"], 0)
                self.assertEqual(r["target_erased"], 1)
                self.assertEqual(r["unlocks"], 1)

    # -------------------------------------------------------------- begin gates
    def test_begin_rejects_bad_sizes_and_missing_sdkops(self):
        for variant in ("sdkops", "zero", "small", "misaligned", "huge",
                        "slot_over", "slot_over_a"):
            with self.subTest(variant=variant):
                r = self.run_mode("begin", variant)
                self.assertEqual(r["begin"], WRITE_FAILED)
                self.assertEqual(r["locks"], 0)
                self.assertEqual(r["unlocks"], 0)
                self.assertEqual(r["programs"], 0)
                self.assertEqual(r["erases"], 0)
                self.assertEqual(r["outside"], 0)
                self.assertEqual(r["target_new"], 0)

    def test_inactive_slot_capacity_is_the_admitted_boundary(self):
        for variant, sectors in (("slot_limit", 0xFE000 // 0x1000),
                                 ("slot_limit_a", 0x100000 // 0x1000)):
            with self.subTest(variant=variant):
                r = self.run_mode("begin", variant)
                self.assertEqual(r["begin"], HANDLED)
                self.assertEqual(r["locks"], 1)
                self.assertEqual(r["unlocks"], 1)
                self.assertEqual(r["erases"], sectors)
                self.assertEqual(r["erases_outside"], 0)
                self.assertEqual(r["outside"], 0)
                self.assertEqual(r["target_erased"], 1)
                self.assertEqual(r["source_new"], 1)
                self.assertEqual(r["data"], 0)

    # -------------------------------------------------------------------- abort
    def test_abort_never_activates_and_is_idempotent(self):
        r = self.run_mode("abort")
        self.assertEqual(r["begin"], HANDLED)
        self.assertEqual(r["w0"], HANDLED)
        self.assertEqual(r["w1"], HANDLED)
        self.assertEqual(r["abort_programs"], 1)
        self.assertEqual(r["abort_unlocks"], 1)
        self.assertEqual(r["unlocks_extra"], 0)
        self.assertEqual(r["write_after"], WRITE_FAILED)
        self.assertEqual(r["end"], WRITE_FAILED)
        self.assertEqual(r["target_new"], 0)
        self.assertEqual(r["target_erased"], 1)
        self.assertEqual(r["source_new"], 1)
        self.assertEqual(r["demotes"], 0)
        self.assertEqual(r["unlocks"], 1)

    # ------------------------------------------------------------- short upload
    def test_short_upload_never_activates(self):
        for variant in ("partial", "none"):
            with self.subTest(variant=variant):
                r = self.run_mode("short", variant)
                self.assertEqual(r["begin"], HANDLED)
                if variant == "partial":
                    for key in ("w0", "w1", "w2"):
                        self.assertEqual(r[key], HANDLED, key)
                self.assertEqual(r["end"], WRITE_FAILED)
                self.assertEqual(r["target_new"], 0)
                self.assertEqual(r["target_erased"], 1)
                self.assertEqual(r["source_new"], 1)
                self.assertEqual(r["demotes"], 0)
                self.assertEqual(r["unlocks"], 1)

    # --------------------------------------------------------- corrupted readback
    def test_corrupted_readback_fails_closed(self):
        for offset in ("0", "16", "42", "600", "1023"):
            with self.subTest(offset=offset):
                r = self.run_mode("corrupt", offset)
                self.assertEqual(r["begin"], HANDLED)
                for key in ("w0", "w1", "w2", "w3"):
                    self.assertEqual(r[key], HANDLED, key)
                self.assertEqual(r["end"], WRITE_FAILED)
                self.assertEqual(r["target_new"], 0)
                self.assertEqual(r["target_erased"], 1)
                self.assertEqual(r["source_new"], 1)
                self.assertEqual(r["demotes"], 0)
                self.assertEqual(r["unlocks"], 1)

    # --------------------------------------------------------- one-shot per boot
    def test_boot_latch_is_one_shot(self):
        r = self.run_mode("double")
        self.assertEqual(r["first"], HANDLED)
        self.assertEqual(r["second"], WRITE_FAILED)
        self.assertEqual(r["locks"], 1)
        self.assertEqual(r["unlocks"], 1)
        self.assertEqual(r["target_new"], 0)

        r = self.run_mode("early")
        self.assertEqual(r["end"], WRITE_FAILED)
        self.assertEqual(r["write"], WRITE_FAILED)
        self.assertEqual(r["locks"], 0)
        self.assertEqual(r["unlocks"], 0)
        self.assertEqual(r["programs"], 0)
        self.assertEqual(r["erases"], 0)

    # ----------------------------------------------------------- source contract
    def test_family_declares_one_boot_and_one_integrity_object(self):
        source = FAMILY_C.read_text()
        code = re.sub(r"/\*.*?\*/", "", source, flags=re.S)
        code = re.sub(r"//[^\n]*", "", code)
        self.assertEqual(source.count("static image2_session_boot amb1_boot;"), 1)
        self.assertEqual(source.count("static image2_integrity amb1_integrity;"), 1)
        self.assertEqual(source.count("static image2_session amb1_session;"), 1)
        self.assertEqual(source.count("lt_ota_family_amb1_sdkops(void)"), 1)
        self.assertIn("__attribute__((weak))", source)
        self.assertNotIn("lt_ota_switch", code)
        self.assertNotIn("sys_reboot", code)
        # The size contract must be UF2-block based, not a literal size - 512.
        self.assertNotIn("size - AMB1_UF2_BLOCK_SIZE", source)
        self.assertIn("(size / AMB1_UF2_BLOCK_SIZE) - 1u", source)

    def test_integrity_provider_is_documented_as_not_authentication(self):
        header = (OTA / "image2_integrity.h").read_text().lower()
        self.assertIn("not authentication", header)
        self.assertIn("integrity only", header)
        # No secret material or MAC API is referenced by the provider itself.
        source = (OTA / "image2_integrity.c").read_text().lower()
        self.assertNotIn("hmac", source)
        self.assertNotIn("wcfota", source)
        self.assertNotIn("secret", source)


if __name__ == "__main__":
    unittest.main(verbosity=2)
