"""Compile the actual SDK SHA-256 C, never a mock crypto implementation."""
import ctypes
import hashlib
import hmac
import os
from pathlib import Path
import shutil
import struct
import subprocess
import tempfile
import unittest


ROOT = Path(__file__).resolve().parents[3]
OTA = ROOT / "cores/realtek-amb1/base/ota"
SDK_COMPONENT = Path("component/common/network/ssl/mbedtls-2.4.0")


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


SDK = find_sdk()
KEY = b"public-test-key-not-a-device-credential"
BOARD = 0x12345678


def envelope(payload, key=KEY, board=BOARD):
    header = b"WCFOTA01" + struct.pack("<II", len(payload), board)
    header += hashlib.sha256(payload).digest()
    return header + hmac.digest(key, header, "sha256")


class Image2AuthTest(unittest.TestCase):
    @classmethod
    def setUpClass(cls):
        if not shutil.which("cc"):
            raise unittest.SkipTest("C compiler required to test SDK mbedTLS SHA-256")
        if SDK is None:
            raise unittest.SkipTest(
                "framework-realtek-amb1 mbedTLS sources not installed; set "
                "AMB1_SDK_DIR to the framework package directory"
            )
        cls.tmp = tempfile.TemporaryDirectory(prefix="image2-auth-")
        cls.addClassCleanup(cls.tmp.cleanup)
        directory = Path(cls.tmp.name)
        (directory / "auth_config.h").write_text("#define MBEDTLS_SHA256_C\n")
        (directory / "wrapper.c").write_text(r'''
#include "image2_auth.h"
#include <string.h>
static image2_auth a;
static image2_session_crypto c;
int init(const void *h, size_t n, uint64_t total, const void *k, size_t kn,
         uint32_t lo, uint32_t hi, int require, uint32_t board) {
    image2_auth_policy policy = {lo, hi, require, board};
    int ok = image2_auth_init(&a,h,n,total,k,kn,&policy);
    if (ok) image2_auth_callbacks(&a,&c);
    return ok;
}
int authorize(uint32_t n) { return c.authorize(c.ctx,n); }
int reset(void) { return c.hash_reset(c.ctx); }
int update(const void *p, size_t n) { return c.hash_update(c.ctx,p,n); }
int check(void) { return c.hash_check(c.ctx); }
int copied(void) {
    image2_auth copy = a;
    image2_session_crypto cb;
    return image2_auth_callbacks(&copy,&cb);
}
void cleanup(void) { image2_auth_cleanup(&a); }
int zero(void) {
    const unsigned char *p = (const unsigned char *)&a;
    size_t i;
    for (i=0;i<sizeof(a);i++) if (p[i]) return 0;
    return 1;
}
static unsigned char flash[512];
static int flash_calls;
static int rd(void *ctx, uint32_t off, void *p, size_t n) {
    (void)ctx; flash_calls++;
    if (off > sizeof(flash) || n > sizeof(flash)-off) return 0;
    memcpy(p,flash+off,n); return 1;
}
static int wr(void *ctx, uint32_t off, const void *p, size_t n) {
    (void)ctx; flash_calls++;
    if (off > sizeof(flash) || n > sizeof(flash)-off) return 0;
    memcpy(flash+off,p,n); return 1;
}
static int er(void *ctx, uint32_t off, size_t n) {
    (void)ctx; flash_calls++;
    if (off > sizeof(flash) || n > sizeof(flash)-off) return 0;
    memset(flash+off,255,n); return 1;
}
int transaction(const void *payload, uint32_t n, int corrupt) {
    image2_transaction t = {0};
    image2_layout layout = {0};
    image2_callbacks io = {0};
    memset(flash,255,sizeof(flash));
    memcpy(flash+8,"81958711",8); flash_calls=0;
    layout.slots[0].start=0; layout.slots[0].end=256;
    layout.slots[1].start=256; layout.slots[1].end=512;
    layout.flash_size=512; layout.sector_size=64;
    layout.reviewed=layout.slots[1];
    layout.reviewed_layout=1; layout.normal_selector=1;
    layout.sram_base=0x10006000; layout.sram_end=0x10006100;
    layout.sdram_base=0x30000000; layout.sdram_end=0x30000100;
    io.ctx=c.ctx; io.read=rd; io.write=wr; io.erase=er;
    io.authorize=c.authorize; io.hash_reset=c.hash_reset;
    io.hash_update=c.hash_update; io.hash_check=c.hash_check;
    if (!image2_begin(&t,&layout,&io,n)) return 0;
    if (!image2_write(&t,payload,n)) return 0;
    if (corrupt) flash[256+63]^=1;
    return image2_verify(&t);
}
int calls(void) { return flash_calls; }
''')
        output = directory / "auth.so"
        subprocess.run([
            "cc", "-std=c99", "-Wall", "-Wextra", "-Werror", "-O2",
            "-shared", "-fPIC", '-DMBEDTLS_CONFIG_FILE="auth_config.h"',
            "-I" + str(directory), "-I" + str(OTA), "-I" + str(SDK / "include"),
            str(OTA / "image2_auth.c"), str(OTA / "image2_transaction.c"),
            str(SDK / "library/sha256.c"),
            str(directory / "wrapper.c"), "-o", str(output),
        ], check=True, capture_output=True, text=True)
        cls.lib = ctypes.CDLL(str(output))
        cls.lib.init.argtypes = [ctypes.c_void_p, ctypes.c_size_t, ctypes.c_uint64,
                                ctypes.c_void_p, ctypes.c_size_t, ctypes.c_uint32,
                                ctypes.c_uint32, ctypes.c_int, ctypes.c_uint32]
        cls.lib.authorize.argtypes = [ctypes.c_uint32]
        cls.lib.update.argtypes = [ctypes.c_void_p, ctypes.c_size_t]
        cls.lib.transaction.argtypes = [ctypes.c_void_p, ctypes.c_uint32, ctypes.c_int]

    def setUp(self):
        self.lib.cleanup()

    def start(self, payload=b"abc", header=None, key=KEY, total=None,
              minimum=1, maximum=0x100000, require=1, board=BOARD):
        header = envelope(payload) if header is None else header
        return self.lib.init(header, len(header), len(payload) + 80 if total is None else total,
                             key, len(key), minimum, maximum, require, board)

    def verify(self, payload):
        self.assertEqual(self.lib.reset(), 1)
        for i in range(0, len(payload), 63):
            chunk = payload[i:i + 63]
            self.assertEqual(self.lib.update(chunk, len(chunk)), 1)
        self.assertEqual(self.lib.check(), 1)

    def test_known_sha_vectors_and_readback_pass(self):
        self.assertEqual(hashlib.sha256(b"abc").hexdigest(),
                         "ba7816bf8f01cfea414140de5dae2223b00361a396177a9cb410ff61f20015ad")
        for payload in (b"abc", b"a" * 1000000, bytes(range(256))):
            with self.subTest(length=len(payload)):
                self.lib.cleanup()
                self.assertEqual(self.start(payload), 1)
                self.assertEqual(self.lib.authorize(len(payload)), 1)
                self.verify(payload)
                self.verify(payload)

    def test_every_envelope_byte_is_checked(self):
        original = envelope(b"abc")
        for i in range(80):
            mutated = bytearray(original)
            mutated[i] ^= 1
            self.assertEqual(self.start(header=bytes(mutated)), 0, i)

    def test_bounds_and_version(self):
        for n in (0, 1, 47, 48, 79, 81, 160):
            self.assertEqual(self.start(header=(envelope(b"abc") + bytes(80))[:n]), 0)
        for total in (0, 82, 84, 2**32 + 83, 2**64 - 1):
            self.assertEqual(self.start(total=total), 0)
        for lo, hi in ((0, 10), (4, 10), (1, 2), (10, 1)):
            self.assertEqual(self.start(minimum=lo, maximum=hi), 0)
        for version in (b"WCFOTA00", b"WCFOTA02"):
            header = version + envelope(b"abc")[8:48]
            self.assertEqual(self.start(header=header + hmac.digest(KEY, header, "sha256")), 0)
        huge = b"WCFOTA01" + struct.pack("<II", 0xffffffff, BOARD) + bytes(32)
        huge += hmac.digest(KEY, huge, "sha256")
        self.assertEqual(self.start(header=huge, total=79, maximum=0xffffffff), 0)

    def test_key_and_board_policy(self):
        self.assertEqual(self.start(key=b"wrong"), 0)
        self.assertEqual(self.start(key=b""), 0)
        self.assertEqual(self.start(board=0), 0)
        self.assertEqual(self.start(require=0, board=0), 1)
        for key in (b"x", b"x" * 64, b"x" * 65, bytes(range(256))):
            self.lib.cleanup()
            self.assertEqual(self.start(header=envelope(b"abc", key=key), key=key), 1)
            self.assertEqual(self.lib.authorize(3), 1)
            self.verify(b"abc")

    def test_frozen_inputs_and_reinit_rejection(self):
        header = ctypes.create_string_buffer(envelope(b"abc"))
        key = ctypes.create_string_buffer(KEY)
        self.assertEqual(self.lib.init(header, 80, 83, key, len(KEY), 1, 10, 1, BOARD), 1)
        ctypes.memset(header, 0, 80)
        ctypes.memset(key, 0, len(KEY))
        self.assertEqual(self.start(b"def"), 0)
        self.assertEqual(self.lib.copied(), 0)
        self.assertEqual(self.lib.authorize(3), 1)
        self.verify(b"abc")

    def test_order_length_and_tamper_fail_closed(self):
        for failure in (lambda: self.lib.reset(), lambda: self.lib.update(b"a", 1),
                        lambda: self.lib.check(), lambda: self.lib.authorize(4)):
            self.lib.cleanup()
            self.assertEqual(self.start(), 1)
            self.assertEqual(failure(), 0)
            self.assertEqual(self.lib.authorize(3), 0)
        for payload in (b"abd", b"ab", b"abcd"):
            self.lib.cleanup()
            self.assertEqual(self.start(), 1)
            self.assertEqual(self.lib.authorize(3), 1)
            self.assertEqual(self.lib.reset(), 1)
            updated = self.lib.update(payload, len(payload))
            self.assertEqual(updated, int(len(payload) <= 3))
            self.assertEqual(self.lib.check(), 0)
            self.assertEqual(self.lib.reset(), 0)

    def test_duplicate_authorize_reset_check_and_cleanup(self):
        for duplicate in ("authorize", "reset", "check"):
            self.lib.cleanup()
            self.assertEqual(self.start(), 1)
            self.assertEqual(self.lib.authorize(3), 1)
            if duplicate == "authorize":
                self.assertEqual(self.lib.authorize(3), 0)
            elif duplicate == "reset":
                self.assertEqual(self.lib.reset(), 1)
                self.assertEqual(self.lib.reset(), 0)
            else:
                self.verify(b"abc")
                self.assertEqual(self.lib.check(), 0)
            self.assertEqual(self.lib.reset(), 0)
        self.lib.cleanup()
        self.lib.cleanup()
        self.assertEqual(self.lib.zero(), 1)
        self.assertEqual(self.lib.authorize(3), 0)
        self.assertEqual(self.lib.reset(), 0)
        self.assertEqual(self.lib.update(b"abc", 3), 0)
        self.assertEqual(self.lib.check(), 0)

    def test_actual_transaction_authorizes_before_flash_and_hashes_readback(self):
        payload = (struct.pack("<II", 16, 0x10006000) + b"81958711"
                   + struct.pack("<I", 0x10006001) + b"RTKWin" + bytes(6)
                   + struct.pack("<II", 16, 0x30000000) + bytes([255]) * 8
                   + bytes(range(16)))
        self.assertEqual(len(payload), 64)
        self.assertEqual(self.start(payload), 1)
        self.assertEqual(self.lib.transaction(payload, 63, 0), 0)
        self.assertEqual(self.lib.calls(), 0)
        for corrupt in (0, 1):
            self.lib.cleanup()
            self.assertEqual(self.start(payload), 1)
            self.assertEqual(self.lib.transaction(payload, len(payload), corrupt), 1-corrupt)
            self.assertGreater(self.lib.calls(), 0)


if __name__ == "__main__":
    unittest.main()
