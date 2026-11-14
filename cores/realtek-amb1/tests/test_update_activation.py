"""Actual-source Update/OTA lifecycle and non-activating cleanup tests.

No target compiler, hardware, network, or flash is used. UF2 parsing, FAL, clock,
and MD5 are deterministic host doubles, not format/cryptography validation.
Set UF2OTA_DIR to a local library-uf2ota 5.0.0 package if not installed normally.
"""

import os
import shutil
import subprocess
import tempfile
import unittest
from pathlib import Path

ROOT = Path(__file__).resolve().parents[3]
COMMON = ROOT / "cores/common"
UPDATE = COMMON / "arduino/libraries/common/Update"

LIBRETINY = r"""
#pragma once
#include <stdbool.h>
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
void lt_xtob(const char *, size_t, uint8_t *);
void lt_btox(const uint8_t *, size_t, char *);
"""

ARDUINO = r"""
#pragma once
#include <cstdint>
#include <cstring>
#include <cstdio>
#include <string>
extern "C" {
#include <lt_ota.h>
}
#define LOW 0
using String = std::string;
class Print { public: void println(const char *) {} };
class Stream { public: virtual int available() = 0;
 virtual size_t readBytes(uint8_t *, size_t) = 0; };
uint32_t millis();
"""

MD5 = r"""
#pragma once
#include <stdint.h>
#include <stddef.h>
typedef struct { uint8_t value; } LT_MD5_CTX_T;
void MD5Init(LT_MD5_CTX_T *);
void MD5Update(LT_MD5_CTX_T *, const uint8_t *, size_t);
void MD5Final(uint8_t *, LT_MD5_CTX_T *);
"""

FAL = r"""
#pragma once
#include <stddef.h>
#include <stdint.h>
struct fal_partition { uint32_t offset; uint32_t len; };
struct fal_flash_dev { struct {
 int (*erase)(long, size_t); int (*write)(long, const uint8_t *, size_t);
} ops; };
const struct fal_partition *fal_get_partition_table(size_t *);
"""

PARSER = r"""
#include <uf2ota/uf2ota.h>
#include <fal.h>
int ctx_frees, info_frees;
void real_uf2_ctx_free(uf2_ota_t *);
void real_uf2_info_free(uf2_info_t *);
void uf2_ctx_free(uf2_ota_t *ctx) { ctx_frees++; real_uf2_ctx_free(ctx); }
void uf2_info_free(uf2_info_t *info) { info_frees++; real_uf2_info_free(info); }
static int erase(long o, size_t n) { (void)o; return n; }
static int write_flash(long o, const uint8_t *p, size_t n) {
 (void)o; (void)p; return n;
}
static struct fal_partition part = { 0, 4096 };
static struct fal_flash_dev flash = { { erase, write_flash } };
const struct fal_partition *fal_get_partition_table(size_t *n) {
 *n = 1; return &part;
}
uf2_err_t uf2_parse_block(uf2_ota_t *ctx, const uf2_block_t *b, uf2_info_t *info) {
 (void)b; (void)info;
 ctx->seq++; ctx->is_format_ok = true; ctx->is_part_set = true;
 ctx->part = &part; ctx->flash = &flash;
 return UF2_ERR_OK;
}
bool uf2_is_erased(uf2_ota_t *ctx, uint32_t o, uint32_t n) {
 (void)ctx; (void)o; (void)n; return true;
}
uf2_err_t uf2_binpatch(uint8_t *d, const uint8_t *p, uint8_t n) {
 (void)d; (void)p; (void)n; return UF2_ERR_OK;
}
"""

HARNESS = r"""
#include "Update.h"
#include <iostream>
static int switches;
static bool switch_ok = true;
static uint32_t clock_ms;
extern "C" { extern int ctx_frees, info_frees; }
extern "C" uint32_t lt_cpu_get_family() { return 0x12345678; }
#ifndef LN_FAMILY
extern "C" lt_ota_type_t lt_ota_get_type() { return OTA_TYPE_DUAL; }
extern "C" uint8_t lt_ota_dual_get_current() { return 1; }
extern "C" bool lt_ota_is_valid(uint8_t) { return true; }
extern "C" bool lt_ota_switch(bool) { switches++; return switch_ok; }
#else
extern "C" void ln_nvds_set_ota_upg_state(int state) {
 if (state == 2) switches++;
}
#endif
extern "C" void lt_xtob(const char *s, size_t n, uint8_t *out) {
 for (size_t i=0; i<n/2; ++i) { unsigned x; sscanf(s+2*i, "%2x", &x); out[i]=x; }
}
extern "C" void lt_btox(const uint8_t *p, size_t n, char *out) {
 for (size_t i=0; i<n; ++i) sprintf(out+2*i, "%02x", p[i]);
}
uint32_t millis() { return clock_ms += 31001; }
void MD5Init(LT_MD5_CTX_T *p) { p->value = 0; }
void MD5Update(LT_MD5_CTX_T *, const uint8_t *, size_t) {}
void MD5Final(uint8_t *out, LT_MD5_CTX_T *) { memset(out, 0x11, 16); }
static uf2_block_t block(unsigned seq) {
 uf2_block_t b{}; b.magic1=UF2_MAGIC_1; b.magic2=UF2_MAGIC_2;
 b.magic3=UF2_MAGIC_3; b.has_family_id=true; b.file_size=0x12345678;
 b.has_tags=true; b.block_count=2; b.block_seq=seq; b.len=seq?4:0;
 return b;
}
class EmptyStream : public Stream {
 int available() override { return 0; }
 size_t readBytes(uint8_t *, size_t) override { return 0; }
};
class BlockStream : public Stream {
 uf2_block_t data; bool sent=false;
 public: BlockStream(uf2_block_t b) : data(b) {}
 int available() override { return sent?0:sizeof(data); }
 size_t readBytes(uint8_t *p, size_t n) override {
  memcpy(p, &data, n); sent=true; return n;
 }
};
int main(int argc, char **argv) {
 if (argc != 2) return 2;
 std::string mode=argv[1]; UpdateClass u;
  auto a=block(0), b=block(1);
  if (mode.compare(0, 2, "c_")==0) {
   lt_ota_ctx_t ctx;
   lt_ota_begin(&ctx, 1024);
   lt_ota_write(&ctx, (uint8_t *)&a, sizeof(a));
   if (mode!="c_incomplete") lt_ota_write(&ctx, (uint8_t *)&b, sizeof(b));
   if (mode=="c_error") ctx.error=UF2_ERR_MAGIC;
   if (mode=="c_tail") { uint8_t byte=0; lt_ota_write(&ctx, &byte, 1); }
   if (mode=="c_switch_fail") switch_ok=false;
   bool result=true;
   if (mode=="c_abort") lt_ota_abort(&ctx);
   else result=lt_ota_end(&ctx);
   lt_ota_abort(&ctx); lt_ota_end(&ctx); lt_ota_abort(nullptr);
   std::cout << result << " 0 " << (int)ctx.error << " " << switches
             << " " << ctx_frees << " " << info_frees << "\n";
   return 0;
  }
  u.begin(mode=="unknown" || mode=="empty_unknown"?UPDATE_SIZE_UNKNOWN:1024);
 if (mode=="md5") u.setMD5("22222222222222222222222222222222");
 if (mode=="match") u.setMD5("11111111111111111111111111111111");
  if (mode=="size_mismatch" || mode=="stream_size_mismatch") a.block_count=3;
  if (mode=="stream_size_mismatch") { BlockStream s(a); u.writeStream(s); }
  else if (mode!="empty_unknown") u.write((uint8_t *)&a, sizeof(a));
  if (mode=="incomplete" || mode=="even_incomplete" || mode=="timeout" ||
      mode=="empty_unknown" || mode=="size_mismatch" || mode=="stream_size_mismatch") {
   if (mode=="timeout") { EmptyStream s; u.writeStream(s); }
  } else if (mode=="stream_bad") {
   b.magic1=0; BlockStream s(b); u.writeStream(s);
  } else u.write((uint8_t *)&b, sizeof(b));
 bool result=false;
 if (mode=="abort") u.abort();
 else if (mode=="restart") { u.begin(1024); u.abort(); }
 else if (mode=="bad_extra" || mode=="bad_extra_even") {
  b.magic1=0; u.write((uint8_t *)&b, sizeof(b));
  result=u.end(mode=="bad_extra_even");
 } else if (mode=="partial_extra") {
  uint8_t byte=0; u.write(&byte, 1); result=u.end();
 } else {
  if (mode=="switch_fail") switch_ok=false;
  result=u.end(mode=="even_incomplete");
 }
  u.end(true); u.abort(); u.abort();
  std::cout << result << " " << (int)u.getError() << " "
            << (int)u.getUF2Error() << " " << switches
            << " " << ctx_frees << " " << info_frees << "\n";
}
"""


class UpdateActivationTests(unittest.TestCase):
    @classmethod
    def setUpClass(cls):
        cc, cxx = shutil.which("cc"), shutil.which("c++")
        if not cc or not cxx:
            raise unittest.SkipTest("host C/C++ compilers unavailable")
        candidates = [
            Path(os.environ.get("UF2OTA_DIR", "/nonexistent")),
            Path.home() / ".platformio/packages/library-uf2ota",
            ROOT.parent / "fridge-local/esphome/.platformio/packages/library-uf2ota",
        ]
        pkg = next((p for p in candidates if (p / "src/uf2ota.c").is_file()), None)
        if pkg is None:
            raise unittest.SkipTest("set UF2OTA_DIR to installed library-uf2ota 5.0.0")
        if (
            "#define UF2OTA_VERSION 50000"
            not in (pkg / "include/uf2ota/uf2types.h").read_text()
        ):
            raise unittest.SkipTest("host fixture supports UF2OTA_VERSION 50000 only")
        cls.temp = tempfile.TemporaryDirectory(prefix="update-activation-")
        cls.addClassCleanup(cls.temp.cleanup)
        tmp = Path(cls.temp.name)
        for name, content in {
            "libretiny.h": LIBRETINY,
            "Arduino.h": ARDUINO,
            "MD5.h": MD5,
            "fal.h": FAL,
            "parser.c": PARSER,
            "harness.cpp": HARNESS,
        }.items():
            (tmp / name).write_text(content)
        includes = [
            "-I" + str(p) for p in (tmp, COMMON / "base/api", UPDATE, pkg / "include")
        ]
        objects = []
        for i, source in enumerate(
            (COMMON / "base/api/lt_ota.c", pkg / "src/uf2ota.c", tmp / "parser.c")
        ):
            obj = tmp / (str(i) + ".o")
            defines = (
                [
                    "-Duf2_ctx_free=real_uf2_ctx_free",
                    "-Duf2_info_free=real_uf2_info_free",
                ]
                if source == pkg / "src/uf2ota.c"
                else []
            )
            subprocess.run(
                [
                    cc,
                    "-std=c11",
                    *defines,
                    *includes,
                    "-c",
                    str(source),
                    "-o",
                    str(obj),
                ],
                check=True,
                capture_output=True,
            )
            objects.append(str(obj))
        cls.binary = tmp / "activation"
        subprocess.run(
            [
                cxx,
                "-std=c++11",
                *includes,
                str(UPDATE / "Update.cpp"),
                str(UPDATE / "UpdateUtil.cpp"),
                str(tmp / "harness.cpp"),
                *objects,
                "-o",
                str(cls.binary),
            ],
            check=True,
            capture_output=True,
        )
        (tmp / "sdk_private.h").write_text(
            "#define UPG_STATE_DOWNLOAD_ING 1\n#define UPG_STATE_DOWNLOAD_OK 2\n"
            "#define FLASH_OTA_OFFSET 0\nvoid ln_nvds_set_ota_upg_state(int);\n"
        )
        (tmp / "ota_image.h").write_text(
            "typedef int image_hdr_t;\n#define OTA_ERR_NONE 0\n"
            "static inline int image_header_fast_read(int o, image_hdr_t *h) { return 0; }\n"
            "static inline int image_header_verify(image_hdr_t *h) { return 0; }\n"
            "static inline int image_body_verify(int o, image_hdr_t *h) { return 0; }\n"
        )
        ln_obj = tmp / "ln.o"
        subprocess.run(
            [
                cc,
                "-std=c11",
                *includes,
                "-include",
                "lt_ota.h",
                "-c",
                str(ROOT / "cores/lightning-ln882h/base/api/lt_ota.c"),
                "-o",
                str(ln_obj),
            ],
            check=True,
            capture_output=True,
        )
        cls.ln_binary = tmp / "ln-activation"
        subprocess.run(
            [
                cxx,
                "-std=c++11",
                "-DLN_FAMILY",
                *includes,
                str(UPDATE / "Update.cpp"),
                str(UPDATE / "UpdateUtil.cpp"),
                str(tmp / "harness.cpp"),
                *objects,
                str(ln_obj),
                "-o",
                str(cls.ln_binary),
            ],
            check=True,
            capture_output=True,
        )

    def run_case(self, mode, binary=None):
        result = subprocess.run(
            [str(binary or self.binary), mode],
            check=True,
            capture_output=True,
            text=True,
        )
        values = tuple(map(int, result.stdout.split()))
        releases = 2 if mode == "restart" else 1
        self.assertEqual(values[4:], (releases, releases), mode)
        return values[:4]

    def test_success_and_matching_md5_activate_once(self):
        self.assertEqual(self.run_case("success"), (1, 0, 0, 1))
        self.assertEqual(self.run_case("match"), (1, 0, 0, 1))
        self.assertEqual(self.run_case("unknown"), (1, 0, 0, 1))

    def test_incomplete_and_timeout_do_not_activate(self):
        self.assertEqual(self.run_case("incomplete"), (0, 12, 0, 0))
        self.assertEqual(self.run_case("timeout"), (0, 6, 0, 0))

    def test_activation_failure_reported(self):
        self.assertEqual(self.run_case("switch_fail"), (0, 9, 0, 1))

    def test_actual_ln_backend_must_not_mark_download_ok_on_bad_md5(self):
        self.assertEqual(self.run_case("md5", self.ln_binary), (0, 7, 0, 0))

    def test_md5_rejection_must_not_activate(self):
        self.assertEqual(self.run_case("md5"), (0, 7, 0, 0))

    def test_abort_after_complete_must_not_activate(self):
        self.assertEqual(self.run_case("abort")[3], 0)

    def test_rebegin_must_not_activate_previous_upload(self):
        self.assertEqual(self.run_case("restart")[3], 0)

    def test_parser_failure_after_complete_must_not_activate(self):
        result = self.run_case("bad_extra")
        self.assertEqual(result[1:3], (8, 2))
        self.assertEqual(result[3], 0)

    def test_trailing_partial_block_must_not_activate(self):
        self.assertEqual(self.run_case("partial_extra")[3], 0)

    def test_even_if_remaining_must_not_report_uncommitted_success(self):
        self.assertEqual(self.run_case("even_incomplete")[0], 0)

    def test_even_if_remaining_preserves_parser_error(self):
        self.assertEqual(self.run_case("bad_extra_even"), (0, 8, 2, 0))

    def test_empty_unknown_size_is_not_complete(self):
        self.assertEqual(self.run_case("empty_unknown"), (0, 12, 0, 0))

    def test_stream_errors_release_once_and_preserve_evidence(self):
        self.assertEqual(self.run_case("stream_bad"), (0, 8, 2, 0))
        for mode in ("size_mismatch", "stream_size_mismatch"):
            self.assertEqual(self.run_case(mode), (0, 11, 11, 0))

    def test_c_api_release_and_legacy_return_contract(self):
        for mode in ("c_abort", "c_incomplete", "c_error", "c_tail"):
            with self.subTest(mode=mode):
                result = self.run_case(mode)
                self.assertEqual(result[0], 1)
                self.assertEqual(result[3], 0)
        self.assertEqual(self.run_case("c_success"), (1, 0, 0, 1))
        self.assertEqual(self.run_case("c_switch_fail"), (0, 0, 0, 1))

    def test_actual_ln_backend_rejection_and_success_paths(self):
        for mode in ("abort", "restart", "bad_extra", "partial_extra", "timeout"):
            with self.subTest(mode=mode):
                self.assertEqual(self.run_case(mode, self.ln_binary)[3], 0)
        self.assertEqual(self.run_case("match", self.ln_binary), (1, 0, 0, 1))


if __name__ == "__main__":
    unittest.main(verbosity=2)
