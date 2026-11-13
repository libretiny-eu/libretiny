"""Host tests for the common lt_ota family lifecycle hooks.

The real cores/common/base/api/lt_ota.c is compiled against local UF2/FAL
doubles. No target compiler, hardware, network, or flash is used. The doubles
record stock-writer calls, image2-style staging, activation attempts, and
resource release so the hook contract can be checked without a second parser.

Covered contract:
  * default (no strong family hook) keeps today's uf2_write/lt_ota_switch path;
  * a strong family hook can accept/stage blocks and short-circuit the FAL writer;
  * abort never commits/finalizes/activates, and is idempotent;
  * end runs only after the Update digest gate and never activates;
  * family errors latch, are not cleared by later blocks, and stay inside the
    Update error-map range.
"""

import os
from pathlib import Path
import shutil
import subprocess
import tempfile
import unittest


ROOT = Path(__file__).resolve().parents[3]
COMMON = ROOT / "cores/common/base/api"
LT_OTA_C = COMMON / "lt_ota.c"
UPDATE = ROOT / "cores/common/arduino/libraries/common/Update"

FAMILY_ID = "0x12345678"

LIBRETINY_H = r"""
#pragma once
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#define LT_HAS_OTA 1
#define LT_DEBUG_OTA 0
#define LT_DM(...)
#define LT_VM(...)
#define LT_IM(...)
#define LT_EM(...)
#define LT_E(...)
#define LT_W(...)
#define MIN(a,b) ((a)<(b)?(a):(b))
uint32_t lt_cpu_get_family(void);
char *lt_btox(const uint8_t *src, int len, char *dest);
uint8_t *lt_xtob(const char *src, int len, uint8_t *dest);
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

UF2OTA_H = r"""
#pragma once
#include "uf2types.h"
void uf2_ctx_init(uf2_ota_t *ctx, uf2_ota_scheme_t scheme, uint32_t family_id);
void uf2_info_init(uf2_info_t *info);
void uf2_ctx_free(uf2_ota_t *ctx);
void uf2_info_free(uf2_info_t *info);
uf2_err_t uf2_check_block(uf2_ota_t *ctx, const uf2_block_t *block);
uf2_err_t uf2_parse_header(uf2_ota_t *ctx, const uf2_block_t *block, uf2_info_t *info);
uf2_err_t uf2_write(uf2_ota_t *ctx, uf2_block_t *block);
"""

ARDUINO_H = r"""
#pragma once
#include <cstdint>
#include <cstring>
#include <cstdio>
#include <cstdlib>
#include <string>
extern "C" {
#include <lt_ota.h>
}
#define LOW 0
using String = std::string;
class Print { public: void println(const char *) {} };
class Stream {
  public:
    virtual int available() = 0;
    virtual size_t readBytes(uint8_t *, size_t) = 0;
};
extern "C" uint32_t millis(void);
"""

MD5_H = r"""
#pragma once
#include <stddef.h>
#include <stdint.h>
#ifdef __cplusplus
extern "C" {
#endif
typedef struct { uint8_t value; } LT_MD5_CTX_T;
void MD5Init(LT_MD5_CTX_T *);
void MD5Update(LT_MD5_CTX_T *, const uint8_t *, size_t);
void MD5Final(uint8_t *, LT_MD5_CTX_T *);
#ifdef __cplusplus
}
#endif
"""

FAL_H = r"""
#pragma once
#include <stddef.h>
#include <stdint.h>
struct fal_partition { char name[16]; uint32_t offset; uint32_t len; };
struct fal_flash_dev {
    struct {
        int (*erase)(long, size_t);
        int (*write)(long, const uint8_t *, size_t);
    } ops;
};
"""

STUB_C = r"""
#include <uf2ota/uf2ota.h>
#include <libretiny.h>
#include <lt_ota.h>
#include <md5stub.h>
#include <stdio.h>

int uf2_write_calls, uf2_header_calls, uf2_ctx_frees, uf2_info_frees;
int uf2_write_fail_on_call;
int switch_calls, switch_ok = 1;
uint32_t uf2_written_total;
uint8_t uf2_written_bytes[4096];

uint32_t lt_cpu_get_family(void) { return 0x12345678u; }
lt_ota_type_t lt_ota_get_type(void) { return OTA_TYPE_DUAL; }
uint8_t lt_ota_dual_get_current(void) { return 1; }
bool lt_ota_is_valid(uint8_t index) { (void)index; return true; }
bool lt_ota_switch(bool revert) { (void)revert; switch_calls++; return switch_ok != 0; }

void uf2_ctx_init(uf2_ota_t *ctx, uf2_ota_scheme_t scheme, uint32_t family_id) {
    memset(ctx, 0, sizeof(*ctx));
    ctx->family_id = family_id;
    ctx->scheme_byte = scheme >> 1;
    ctx->scheme_shift = ((scheme & 1) ^ 1) * 4;
    ctx->scheme_binpatch = scheme == UF2_SCHEME_DEVICE_DUAL_2 ||
                           scheme == UF2_SCHEME_FLASHER_DUAL_2;
}
void uf2_info_init(uf2_info_t *info) { memset(info, 0, sizeof(*info)); }
void uf2_ctx_free(uf2_ota_t *ctx) { if (!ctx) return; uf2_ctx_frees++; memset(ctx, 0, sizeof(*ctx)); }
void uf2_info_free(uf2_info_t *info) { if (!info) return; uf2_info_frees++; memset(info, 0, sizeof(*info)); }

uf2_err_t uf2_check_block(uf2_ota_t *ctx, const uf2_block_t *block) {
    if (block->magic1 != UF2_MAGIC_1) return UF2_ERR_MAGIC;
    if (block->magic2 != UF2_MAGIC_2) return UF2_ERR_MAGIC;
    if (block->magic3 != UF2_MAGIC_3) return UF2_ERR_MAGIC;
    if (block->file_container) return UF2_ERR_IGNORE;
    if (!block->has_family_id || block->file_size != ctx->family_id) return UF2_ERR_FAMILY;
    return UF2_ERR_OK;
}
uf2_err_t uf2_parse_header(uf2_ota_t *ctx, const uf2_block_t *block, uf2_info_t *info) {
    (void)block; (void)info;
    uf2_header_calls++;
    ctx->is_format_ok = true;
    return UF2_ERR_OK;
}
uf2_err_t uf2_write(uf2_ota_t *ctx, uf2_block_t *block) {
    (void)ctx;
    uf2_write_calls++;
    if (uf2_write_fail_on_call && uf2_write_calls == uf2_write_fail_on_call)
        return UF2_ERR_WRITE_FAILED;
    if (uf2_written_total + block->len <= sizeof(uf2_written_bytes)) {
        memcpy(uf2_written_bytes + uf2_written_total, block->data, block->len);
        uf2_written_total += block->len;
    }
    return UF2_ERR_OK;
}

uint8_t *lt_xtob(const char *src, int len, uint8_t *dest) {
    for (int i = 0; i < len / 2; ++i) {
        unsigned x = 0;
        sscanf(src + 2 * i, "%2x", &x);
        dest[i] = (uint8_t)x;
    }
    return dest;
}
char *lt_btox(const uint8_t *src, int len, char *dest) {
    for (int i = 0; i < len; ++i) sprintf(dest + 2 * i, "%02x", src[i]);
    return dest;
}
uint32_t millis(void) { return 0; }
void MD5Init(LT_MD5_CTX_T *p) { p->value = 0; }
void MD5Update(LT_MD5_CTX_T *p, const uint8_t *d, size_t n) { (void)p; (void)d; (void)n; }
void MD5Final(uint8_t *out, LT_MD5_CTX_T *p) { (void)p; memset(out, 0x11, 16); }
"""

MD5STUB_H = r"""
#pragma once
#include <stddef.h>
#include <stdint.h>
typedef struct { uint8_t value; } LT_MD5_CTX_T;
void MD5Init(LT_MD5_CTX_T *);
void MD5Update(LT_MD5_CTX_T *, const uint8_t *, size_t);
void MD5Final(uint8_t *, LT_MD5_CTX_T *);
"""

FAMILY_C = r"""
#include <lt_ota.h>
#include <string.h>

enum {
    FAM_HANDLE = 0,
    FAM_PASS,
    FAM_BEGIN_ERROR,
    FAM_WRITE_ERROR,
    FAM_WRITE_PASS,
    FAM_WRITE_BAD_CODE,
    FAM_END_ERROR,
    FAM_END_PASS,
};
int fam_mode = FAM_HANDLE;
int fam_begin_calls, fam_write_calls, fam_end_calls, fam_abort_calls;
int fam_contiguous = 1;
size_t fam_staged_len;
uint8_t fam_staged[4096];
uint32_t fam_next_seq;

lt_ota_family_result_t lt_ota_family_begin(lt_ota_ctx_t *ctx, size_t size) {
    (void)ctx; (void)size;
    fam_begin_calls++;
    fam_staged_len = 0;
    fam_next_seq = 1;
    fam_contiguous = 1;
    if (fam_mode == FAM_PASS) return LT_OTA_FAMILY_PASS;
    if (fam_mode == FAM_BEGIN_ERROR) return UF2_ERR_FAMILY;
    return LT_OTA_FAMILY_HANDLED;
}
lt_ota_family_result_t lt_ota_family_write_block(lt_ota_ctx_t *ctx, const uf2_block_t *block) {
    (void)ctx;
    fam_write_calls++;
    if (fam_mode == FAM_WRITE_ERROR && fam_write_calls == 2) return UF2_ERR_WRITE_FAILED;
    if (fam_mode == FAM_WRITE_BAD_CODE && fam_write_calls == 2) return 0x2000;
    if (fam_mode == FAM_WRITE_PASS && fam_write_calls == 1) return LT_OTA_FAMILY_PASS;
    if (!block || block->not_main_flash || !block->len ||
        block->block_seq != fam_next_seq || block->addr != fam_staged_len) {
        fam_contiguous = 0;
        return UF2_ERR_SEQ_MISMATCH;
    }
    if (fam_staged_len + block->len > sizeof(fam_staged)) {
        fam_contiguous = 0;
        return UF2_ERR_WRITE_FAILED;
    }
    memcpy(fam_staged + fam_staged_len, block->data, block->len);
    fam_staged_len += block->len;
    fam_next_seq++;
    return LT_OTA_FAMILY_HANDLED;
}
lt_ota_family_result_t lt_ota_family_end(lt_ota_ctx_t *ctx) {
    (void)ctx;
    fam_end_calls++;
    if (fam_mode == FAM_END_ERROR) return UF2_ERR_WRITE_FAILED;
    if (fam_mode == FAM_END_PASS) return LT_OTA_FAMILY_PASS;
    return LT_OTA_FAMILY_HANDLED;
}
void lt_ota_family_abort(lt_ota_ctx_t *ctx) { (void)ctx; fam_abort_calls++; }
"""

DIRECT_HARNESS_C = r"""
#include <lt_ota.h>
#include <stdio.h>
#include <string.h>

extern int uf2_write_calls, uf2_ctx_frees, uf2_info_frees, switch_calls;
extern int fam_mode, fam_begin_calls, fam_write_calls, fam_end_calls, fam_abort_calls;
extern int fam_contiguous;
extern size_t fam_staged_len;
extern uint8_t fam_staged[4096];
extern int FAM_HANDLE;

static uf2_block_t make_block(uint32_t seq, uint32_t count, uint32_t addr, uint32_t len, int header) {
    uf2_block_t b;
    memset(&b, 0, sizeof(b));
    b.magic1 = UF2_MAGIC_1;
    b.magic2 = UF2_MAGIC_2;
    b.magic3 = UF2_MAGIC_3;
    b.has_family_id = true;
    b.file_size = 0x12345678u;
    b.block_seq = seq;
    b.block_count = count;
    b.addr = addr;
    b.len = len;
    if (header) {
        b.has_tags = true;
        b.not_main_flash = true;
        b.len = 0;
    } else {
        for (uint32_t i = 0; i < len; ++i) b.data[i] = (uint8_t)(addr + i);
    }
    return b;
}

int main(int argc, char **argv) {
    if (argc != 2) return 2;
    const char *mode = argv[1];
    lt_ota_ctx_t ctx;
    uf2_block_t h = make_block(0, 3, 0, 0, 1);
    uf2_block_t d1 = make_block(1, 3, 0, 256, 0);
    uf2_block_t d2 = make_block(2, 3, 256, 256, 0);
    size_t w3 = 0;
    int result = 1;
    int fam_err = 0;

    if (!strcmp(mode, "handle_declined")) fam_mode = 1;           /* FAM_PASS */
    if (!strcmp(mode, "handle_begin_error")) fam_mode = 2;        /* FAM_BEGIN_ERROR */
    if (!strcmp(mode, "handle_write_error")) fam_mode = 3;        /* FAM_WRITE_ERROR */
    if (!strcmp(mode, "handle_write_pass")) fam_mode = 4;         /* FAM_WRITE_PASS */
    if (!strcmp(mode, "handle_write_bad_code")) fam_mode = 5;     /* FAM_WRITE_BAD_CODE */
    if (!strcmp(mode, "handle_end_error")) fam_mode = 6;          /* FAM_END_ERROR */
    if (!strcmp(mode, "handle_end_pass")) fam_mode = 7;           /* FAM_END_PASS */

    lt_ota_begin(&ctx, 3 * 512);
    lt_ota_write(&ctx, (uint8_t *)&h, sizeof(h));
    lt_ota_write(&ctx, (uint8_t *)&d1, sizeof(d1));
    if (strcmp(mode, "handle_incomplete") != 0)
        lt_ota_write(&ctx, (uint8_t *)&d2, sizeof(d2));
    if (!strcmp(mode, "handle_write_error") || !strcmp(mode, "handle_write_bad_code")) {
        /* a second attempt must not reach the family or clear the latch */
        w3 = lt_ota_write(&ctx, (uint8_t *)&d2, sizeof(d2));
    }

    if (!strcmp(mode, "handle_abort")) {
        lt_ota_abort(&ctx);
        lt_ota_abort(&ctx);
        result = (int)lt_ota_end(&ctx);
    } else {
        result = (int)lt_ota_end(&ctx);
        lt_ota_abort(&ctx); /* must be a no-op after a successful end */
    }
    fam_err = (int)ctx.family_error;

    printf("%d %d %d %d %d %d %d %d %d %d %d %d %d %d\n",
           result, fam_begin_calls, fam_write_calls, fam_end_calls, fam_abort_calls,
           switch_calls, uf2_write_calls, uf2_ctx_frees, uf2_info_frees,
           (int)ctx.error, fam_err, (int)fam_staged_len, fam_contiguous,
           (int)w3);
    return 0;
}
"""

DEFAULT_HARNESS_C = r"""
#include <lt_ota.h>
#include <stdio.h>
#include <string.h>

extern int uf2_write_calls, uf2_ctx_frees, uf2_info_frees, switch_calls;
extern int uf2_write_fail_on_call;

static uf2_block_t make_block(uint32_t seq, uint32_t count, uint32_t addr, uint32_t len, int header) {
    uf2_block_t b;
    memset(&b, 0, sizeof(b));
    b.magic1 = UF2_MAGIC_1;
    b.magic2 = UF2_MAGIC_2;
    b.magic3 = UF2_MAGIC_3;
    b.has_family_id = true;
    b.file_size = 0x12345678u;
    b.block_seq = seq;
    b.block_count = count;
    b.addr = addr;
    b.len = len;
    if (header) {
        b.has_tags = true;
        b.not_main_flash = true;
        b.len = 0;
    } else {
        for (uint32_t i = 0; i < len; ++i) b.data[i] = (uint8_t)(addr + i);
    }
    return b;
}

int main(int argc, char **argv) {
    if (argc != 2) return 2;
    const char *mode = argv[1];
    lt_ota_ctx_t ctx;
    uf2_block_t h = make_block(0, 3, 0, 0, 1);
    uf2_block_t d1 = make_block(1, 3, 0, 256, 0);
    uf2_block_t d2 = make_block(2, 3, 256, 256, 0);
    size_t w1, w2 = 0;
    int result = 1;

    if (!strcmp(mode, "default_write_error")) uf2_write_fail_on_call = 2;

    lt_ota_begin(&ctx, 3 * 512);
    lt_ota_write(&ctx, (uint8_t *)&h, sizeof(h));
    w1 = lt_ota_write(&ctx, (uint8_t *)&d1, sizeof(d1));
    if (strcmp(mode, "default_incomplete") != 0)
        w2 = lt_ota_write(&ctx, (uint8_t *)&d2, sizeof(d2));

    if (!strcmp(mode, "default_abort")) {
        lt_ota_abort(&ctx);
        result = (int)lt_ota_end(&ctx);
    } else {
        result = (int)lt_ota_end(&ctx);
    }

    printf("%d %d %d %d %d %d %d %d %d\n",
           result, switch_calls, uf2_write_calls, uf2_ctx_frees, uf2_info_frees,
           (int)ctx.error, (int)ctx.bytes_written, (int)w1, (int)w2);
    return 0;
}
"""

UPDATE_HARNESS_CPP = r"""
#include "Update.h"
#include <cstdio>
#include <cstring>

extern "C" {
extern int fam_begin_calls, fam_write_calls, fam_end_calls, fam_abort_calls;
extern int switch_calls;
}

static uf2_block_t make_block(uint32_t seq, uint32_t count, uint32_t addr, uint32_t len, int header) {
    uf2_block_t b;
    memset(&b, 0, sizeof(b));
    b.magic1 = UF2_MAGIC_1;
    b.magic2 = UF2_MAGIC_2;
    b.magic3 = UF2_MAGIC_3;
    b.has_family_id = true;
    b.file_size = 0x12345678u;
    b.block_seq = seq;
    b.block_count = count;
    b.addr = addr;
    b.len = len;
    if (header) {
        b.has_tags = true;
        b.not_main_flash = true;
        b.len = 0;
    } else {
        for (uint32_t i = 0; i < len; ++i) b.data[i] = (uint8_t)(addr + i);
    }
    return b;
}

int main(int argc, char **argv) {
    if (argc != 2) return 2;
    const char *mode = argv[1];
    UpdateClass update;
    uf2_block_t h = make_block(0, 3, 0, 0, 1);
    uf2_block_t d1 = make_block(1, 3, 0, 256, 0);
    uf2_block_t d2 = make_block(2, 3, 256, 256, 0);

    if (!update.begin(3 * 512)) {
        printf("begin-failed\n");
        return 3;
    }
    if (!strcmp(mode, "digest_bad"))
        update.setMD5("22222222222222222222222222222222");
    if (!strcmp(mode, "digest_good"))
        update.setMD5("11111111111111111111111111111111");

    update.write((const uint8_t *)&h, sizeof(h));
    update.write((const uint8_t *)&d1, sizeof(d1));
    update.write((const uint8_t *)&d2, sizeof(d2));

    bool result = update.end();
    printf("%d %d %d %d %d %d %d %d\n",
           (int)result, (int)update.getError(), (int)update.getUF2Error(),
           fam_begin_calls, fam_write_calls, fam_end_calls, fam_abort_calls,
           switch_calls);
    return 0;
}
"""


class OtaStagingHookTests(unittest.TestCase):
    @classmethod
    def setUpClass(cls):
        cc = shutil.which("cc")
        cxx = shutil.which("c++")
        if not cc or not cxx:
            raise unittest.SkipTest("host C/C++ compilers unavailable")

        cls.temp = tempfile.TemporaryDirectory(prefix="ota-staging-hooks-")
        cls.addClassCleanup(cls.temp.cleanup)
        tmp = Path(cls.temp.name)
        (tmp / "uf2ota").mkdir()
        files = {
            "libretiny.h": LIBRETINY_H,
            "uf2ota/uf2types.h": UF2TYPES_H,
            "uf2ota/uf2ota.h": UF2OTA_H,
            "Arduino.h": ARDUINO_H,
            "MD5.h": MD5_H,
            "md5stub.h": MD5STUB_H,
            "fal.h": FAL_H,
            "stub.c": STUB_C,
            "family.c": FAMILY_C,
            "harness_direct.c": DIRECT_HARNESS_C,
            "harness_default.c": DEFAULT_HARNESS_C,
            "harness_update.cpp": UPDATE_HARNESS_CPP,
        }
        for name, content in files.items():
            (tmp / name).write_text(content)

        def compile_c(source, obj, extra=()):
            subprocess.run(
                [cc, "-std=c11", "-O2", "-Wall", "-Wextra", *extra,
                 "-I" + str(tmp), "-I" + str(COMMON), "-c", str(source), "-o", str(obj)],
                check=True, capture_output=True, text=True,
            )

        def link_cxx(source, obj_out, objects):
            subprocess.run(
                [cxx, "-std=c++11", "-O2", "-Wall", "-Wextra",
                 "-I" + str(tmp), "-I" + str(COMMON), "-I" + str(UPDATE),
                 str(source), *objects, "-o", str(obj_out)],
                check=True, capture_output=True, text=True,
            )

        lt_ota_o = tmp / "lt_ota.o"
        stub_o = tmp / "stub.o"
        family_o = tmp / "family.o"
        compile_c(LT_OTA_C, lt_ota_o)
        compile_c(tmp / "stub.c", stub_o)
        compile_c(tmp / "family.c", family_o)

        cls.default_bin = tmp / "default-path"
        subprocess.run(
            [cc, "-std=c11", "-O2", "-Wall", "-Wextra",
             "-I" + str(tmp), "-I" + str(COMMON),
             str(tmp / "harness_default.c"), str(lt_ota_o), str(stub_o),
             "-o", str(cls.default_bin)],
            check=True, capture_output=True, text=True,
        )

        cls.family_bin = tmp / "family-path"
        subprocess.run(
            [cc, "-std=c11", "-O2", "-Wall", "-Wextra",
             "-I" + str(tmp), "-I" + str(COMMON),
             str(tmp / "harness_direct.c"), str(lt_ota_o), str(stub_o), str(family_o),
             "-o", str(cls.family_bin)],
            check=True, capture_output=True, text=True,
        )

        cls.update_bin = tmp / "update-digest"
        subprocess.run(
            [cxx, "-std=c++11", "-O2", "-Wall", "-Wextra",
             "-I" + str(tmp), "-I" + str(COMMON), "-I" + str(UPDATE),
             str(UPDATE / "Update.cpp"), str(UPDATE / "UpdateUtil.cpp"),
             str(tmp / "harness_update.cpp"), str(lt_ota_o), str(stub_o), str(family_o),
             "-o", str(cls.update_bin)],
            check=True, capture_output=True, text=True,
        )

    def run_bin(self, binary, mode):
        result = subprocess.run(
            [str(binary), mode], check=True, capture_output=True, text=True
        )
        return tuple(int(v) for v in result.stdout.split())

    # ------------------------------------------------------------------ default
    def test_default_path_is_unchanged(self):
        # result, switches, uf2 writes, ctx frees, info frees, error, bytes, w1, w2
        self.assertEqual(
            self.run_bin(self.default_bin, "default_ok"),
            (1, 1, 2, 1, 1, 0, 1536, 512, 512),
        )
        self.assertEqual(
            self.run_bin(self.default_bin, "default_incomplete"),
            (1, 0, 1, 1, 1, 0, 1024, 512, 0),
        )
        self.assertEqual(
            self.run_bin(self.default_bin, "default_abort"),
            (1, 0, 2, 1, 1, 0, 1536, 512, 512),
        )

    def test_default_writer_error_contract_is_unchanged(self):
        values = self.run_bin(self.default_bin, "default_write_error")
        self.assertEqual(values[0], 1)          # lt_ota_end: no activation attempted
        self.assertEqual(values[1], 0)          # no switch
        self.assertEqual(values[2], 2)          # uf2_write called twice
        self.assertEqual(values[5], 13)         # UF2_ERR_WRITE_FAILED latched
        self.assertEqual(values[6], 1024)       # only the successful prefix counted
        self.assertEqual(values[8], 0)          # failing write returned 0

    # ------------------------------------------------------------------- family
    def test_strong_hook_stages_and_short_circuits_fal(self):
        # result, begin, write, end, abort, switch, uf2w, frees, ifrees,
        # error, family_error, staged, contiguous, extra write
        self.assertEqual(
            self.run_bin(self.family_bin, "handle_ok"),
            (1, 1, 2, 1, 0, 0, 0, 1, 1, 0, 0, 512, 1, 0),
        )

    def test_strong_object_can_decline_to_default_path(self):
        self.assertEqual(
            self.run_bin(self.family_bin, "handle_declined"),
            (1, 1, 0, 0, 0, 1, 2, 1, 1, 0, 0, 0, 1, 0),
        )

    def test_abort_never_commits_and_is_idempotent(self):
        # end was not called; abort hook once; no switch; staged bytes retained
        self.assertEqual(
            self.run_bin(self.family_bin, "handle_abort"),
            (1, 1, 2, 0, 1, 0, 0, 1, 1, 0, 0, 512, 1, 0),
        )

    def test_incomplete_upload_never_calls_family_end(self):
        self.assertEqual(
            self.run_bin(self.family_bin, "handle_incomplete"),
            (1, 1, 1, 0, 1, 0, 0, 1, 1, 0, 0, 256, 1, 0),
        )

    def test_family_error_latches_and_is_not_cleared(self):
        self.assertEqual(
            self.run_bin(self.family_bin, "handle_write_error"),
            (1, 1, 2, 0, 1, 0, 0, 1, 1, 13, 13, 256, 1, 0),
        )

    def test_out_of_range_family_error_is_clamped(self):
        values = self.run_bin(self.family_bin, "handle_write_bad_code")
        self.assertEqual(values[5], 0)          # never activated
        self.assertEqual(values[9], 13)         # clamped to UF2_ERR_WRITE_FAILED
        self.assertEqual(values[10], 13)
        self.assertEqual(values[13], 0)         # later write did not reach family

    def test_begin_error_aborts_without_any_write(self):
        self.assertEqual(
            self.run_bin(self.family_bin, "handle_begin_error"),
            (1, 1, 0, 0, 1, 0, 0, 1, 1, 3, 3, 0, 1, 0),
        )

    def test_end_failure_returns_false_and_aborts(self):
        self.assertEqual(
            self.run_bin(self.family_bin, "handle_end_error"),
            (0, 1, 2, 1, 1, 0, 0, 1, 1, 13, 13, 512, 1, 0),
        )

    def test_end_pass_is_a_contract_violation(self):
        values = self.run_bin(self.family_bin, "handle_end_pass")
        self.assertEqual(values[0], 0)
        self.assertEqual(values[3], 1)          # end was attempted
        self.assertEqual(values[4], 1)          # and aborted
        self.assertEqual(values[5], 0)          # never activated
        self.assertEqual(values[10], 13)

    def test_write_pass_is_a_contract_violation(self):
        self.assertEqual(
            self.run_bin(self.family_bin, "handle_write_pass"),
            (1, 1, 1, 0, 1, 0, 0, 1, 1, 13, 13, 0, 1, 0),
        )

    # --------------------------------------------------------------- digest gate
    def test_end_only_after_update_digest_check(self):
        # result, ard error, uf2 error, begin, write, end, abort, switch
        self.assertEqual(
            self.run_bin(self.update_bin, "digest_good"),
            (1, 0, 0, 1, 2, 1, 0, 0),
        )
        self.assertEqual(
            self.run_bin(self.update_bin, "digest_bad"),
            (0, 7, 0, 1, 2, 0, 1, 0),
        )
        self.assertEqual(
            self.run_bin(self.update_bin, "digest_none"),
            (1, 0, 0, 1, 2, 1, 0, 0),
        )


if __name__ == "__main__":
    unittest.main(verbosity=2)
