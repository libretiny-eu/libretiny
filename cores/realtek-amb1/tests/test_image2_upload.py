"""Native upload composition: real auth/SHA and all four session modules."""
import os
from pathlib import Path
import struct
import subprocess
import tempfile
import unittest

from test_image2_auth import SDK, KEY, BOARD, envelope
from test_image2_session import HARNESS, MODULES, OTA


# Reuse the session fault-injection harness, but inject an upload-level reentry
# probe into every callback so the upload busy guard (not just the session one)
# is exercised from inside live SDK and crypto callbacks.
_PREFIX = HARNESS.split("int main(void)")[0]
_PREFIX = _PREFIX.replace(
    "static void reentry(void) {",
    "static void upload_reentry(void);\n"
    "static void reentry(void) {\n"
    "    upload_reentry();",
    1,
)
if "upload_reentry();" not in _PREFIX:
    raise RuntimeError("session harness reentry hook injection changed")


def _payload():
    payload = bytearray(b"\xff" * 700)
    struct.pack_into("<II", payload, 0, 10, 0x10000000)
    payload[8:16] = b"81958711"
    struct.pack_into("<I", payload, 16, 0x10000001)
    payload[20:26] = b"RTKWin"
    struct.pack_into("<II", payload, 26, 658, 0x30000000)
    return payload


_HEADER = ",".join(map(str, envelope(_payload())))


_COMMON = r'''
#undef session
#undef boot
#include "image2_upload.h"

static image2_upload *current_upload;
static image2_flash_sdkops g_sdk;
static image2_auth_policy g_policy;
static int reentry_hits;

static unsigned long memory_stamp(void) {
    unsigned long h = 0;
    size_t i;
    for (i = 0; i < sizeof(mem); i++) h = h * 33 + mem[i];
    return h;
}
/* Every callback re-enters the upload API. The busy guard must reject all of
 * it and the attempts must not touch flash or clear the live upload. */
static void upload_reentry(void) {
    unsigned long stamp;
    int c, cc;
    if (!current_upload) return;
    reentry_hits++;
    stamp = memory_stamp();
    c = calls;
    cc = crypto_calls;
    assert(!image2_upload_write(current_upload, "x", 1));
    assert(!image2_upload_finish(current_upload, 1));
    image2_upload_abort(current_upload);
    assert(calls == c && crypto_calls == cc);
    assert(memory_stamp() == stamp);
}
static int zero_auth(const image2_upload *u) {
    size_t i;
    const unsigned char *p = (const unsigned char *)&u->auth;
    for (i = 0; i < sizeof(u->auth); i++) if (p[i]) return 0;
    for (i = 0; i < sizeof(u->envelope); i++) if (u->envelope[i]) return 0;
    return !u->key && !u->key_size;
}
static void keep_expected(void) {
    (void)expected_calls;
    (void)expected_crypto_calls;
}
static void upload_setup(image2_upload_boot *b, image2_upload *u,
                         unsigned running, int old) {
    memset(u, 0, sizeof(*u));
    memset(b, 0, sizeof(*b));
    current_boot = &b->session;
    current_session = &u->session;
    current_upload = u;
    reentry_hits = 0;
    fault = crypto_fault = mode = status = 0;
    setup((int)running, old);
    fault = crypto_fault = 0;
    g_sdk = (image2_flash_sdkops){mem, lock, unlock, read_bytes, program, erase};
    g_policy = (image2_auth_policy){1, 700, 1, BOARD_ID};
}
static int start_valid(image2_upload *u, image2_upload_boot *b,
                       unsigned running, const void *key, size_t key_len) {
    return image2_upload_begin(u, b, 780, &config, &g_sdk, running,
                               &g_policy, key, key_len);
}
static void feed(image2_upload *u, const unsigned char *wire, size_t chunk) {
    size_t pos = 0, n;
    while (pos < 780) {
        n = 780 - pos;
        if (n > chunk) n = chunk;
        assert(image2_upload_write(u, wire + pos, n));
        pos += n;
    }
}
'''


_REENTRY_BODY = r'''
static void reentry_case(unsigned running, int activate) {
    image2_upload_boot b;
    image2_upload u;
    unsigned char wire[780] = {HEADER_BYTES};
    upload_setup(&b, &u, running, 1);
    memcpy(wire + 80, image, 700);
    assert(start_valid(&u, &b, running, KEY_TEXT, KEY_LEN));
    feed(&u, wire, 17);
    assert(reentry_hits > 0);
    activating = activate;
    assert(image2_upload_finish(&u, activate));
    assert(reentry_hits > 0);
    assert(!u.active && zero_auth(&u));
    assert(image2_session_get_status(&u.session) ==
           (activate ? IMAGE2_SESSION_ACTIVATED : IMAGE2_SESSION_NOT_ACTIVATED));
    if (activate) {
        assert(!memcmp(mem + target, image, 700));
        assert(!memcmp(mem + source + 8, "01958711", 8));
    } else {
        assert(!memcmp(mem + source + 8, "81958711", 8));
        /* Verify-only never publishes the candidate signature. */
        assert(memcmp(mem + target + 8, "81958711", 8));
    }
    current_upload = NULL;
}
int main(void) {
    keep_expected();
    reentry_case(0, 1);
    reentry_case(1, 1);
    reentry_case(0, 0);
    reentry_case(1, 0);
    puts("upload callback reentry rejected without flash mutation");
    return 0;
}
'''


_KEY_BODY = r'''
static unsigned char key_buffer[KEY_LEN];

static void key_borrowed_until_envelope(unsigned running) {
    image2_upload_boot b;
    image2_upload u;
    unsigned char wire[780] = {HEADER_BYTES};
    upload_setup(&b, &u, running, 1);
    memcpy(wire + 80, image, 700);
    memset(key_buffer, 'x', sizeof(key_buffer));
    assert(start_valid(&u, &b, running, key_buffer, sizeof(key_buffer)));
    assert(image2_upload_write(&u, wire, 79));
    /* begin must not consume the key. It is still borrowed, so making the
     * buffer valid before the envelope completes is the documented contract. */
    memcpy(key_buffer, KEY_TEXT, KEY_LEN);
    assert(image2_upload_write(&u, wire + 79, sizeof(wire) - 79));
    activating = 0;
    assert(image2_upload_finish(&u, 0));
    assert(!u.active && zero_auth(&u));
    assert(!memcmp(mem + source + 8, "81958711", 8));
    current_upload = NULL;
}

static void key_changed_before_envelope_rejected(unsigned running) {
    image2_upload_boot b;
    image2_upload u;
    unsigned char wire[780] = {HEADER_BYTES};
    upload_setup(&b, &u, running, 1);
    memcpy(wire + 80, image, 700);
    memcpy(key_buffer, KEY_TEXT, KEY_LEN);
    assert(start_valid(&u, &b, running, key_buffer, sizeof(key_buffer)));
    assert(image2_upload_write(&u, wire, 79));
    /* A temporary key whose storage is reused/erased before the envelope
     * arrives is not accepted. This enforces the lifetime contract without
     * claiming a use-after-free. */
    memset(key_buffer, 0xa5, sizeof(key_buffer));
    assert(!image2_upload_write(&u, wire + 79, sizeof(wire) - 79));
    assert(!u.active && zero_auth(&u));
    assert(!calls && !crypto_calls);
    assert(!memcmp(mem, before, sizeof(mem)));
    current_upload = NULL;
}

static void key_abort_before_envelope(unsigned running) {
    image2_upload_boot b;
    image2_upload u;
    upload_setup(&b, &u, running, 1);
    memcpy(key_buffer, KEY_TEXT, KEY_LEN);
    assert(start_valid(&u, &b, running, key_buffer, sizeof(key_buffer)));
    image2_upload_abort(&u);
    assert(!u.active && zero_auth(&u));
    assert(!calls && !crypto_calls);
    assert(!memcmp(mem, before, sizeof(mem)));
    memset(key_buffer, 0, sizeof(key_buffer));
    current_upload = NULL;
}

int main(void) {
    keep_expected();
    key_borrowed_until_envelope(0);
    key_borrowed_until_envelope(1);
    key_changed_before_envelope_rejected(0);
    key_changed_before_envelope_rejected(1);
    key_abort_before_envelope(0);
    key_abort_before_envelope(1);
    puts("borrowed key lifetime contract enforced");
    return 0;
}
'''


_BOUNDARY_BODY = r'''
static void invalid_begin(uint64_t total, uint32_t image_size,
                          const image2_flash_config *cfg,
                          const image2_flash_sdkops *ops,
                          unsigned begin_running,
                          const image2_auth_policy *policy,
                          const void *key, size_t key_size) {
    image2_upload_boot b;
    image2_upload u;
    image2_upload other;
    upload_setup(&b, &u, 0, 1);
    config.image_size = image_size;
    assert(!image2_upload_begin(&u, &b, total, cfg, ops, begin_running,
                                policy, key, key_size));
    assert(u.identity == &u && !u.active && !u.busy);
    assert(!calls && !crypto_calls && zero_auth(&u));
    assert(!memcmp(mem, before, sizeof(mem)));
    /* A rejected begin still consumes the shared one-boot latch. */
    memset(&other, 0, sizeof(other));
    current_session = &other.session;
    current_upload = &other;
    assert(!image2_upload_begin(&other, &b, 780, &config, &g_sdk, 0,
                                &g_policy, KEY_TEXT, KEY_LEN));
    assert(!other.identity && !other.active);
    assert(!calls && !crypto_calls);
    current_upload = NULL;
}

int main(void) {
    keep_expected();
    invalid_begin(0, 700, &config, &g_sdk, 0, &g_policy, KEY_TEXT, KEY_LEN);
    invalid_begin(1, 700, &config, &g_sdk, 0, &g_policy, KEY_TEXT, KEY_LEN);
    invalid_begin(79, 700, &config, &g_sdk, 0, &g_policy, KEY_TEXT, KEY_LEN);
    invalid_begin(IMAGE2_AUTH_ENVELOPE_SIZE, 700, &config, &g_sdk, 0,
                  &g_policy, KEY_TEXT, KEY_LEN);
    invalid_begin((uint64_t)IMAGE2_AUTH_ENVELOPE_SIZE +
                  (uint64_t)UINT32_MAX + 1, UINT32_MAX,
                  &config, &g_sdk, 0, &g_policy, KEY_TEXT, KEY_LEN);
    invalid_begin(780, 700, &config, &g_sdk, 2, &g_policy, KEY_TEXT, KEY_LEN);
    invalid_begin(780, 700, &config, &g_sdk, 0xffffffffu, &g_policy,
                  KEY_TEXT, KEY_LEN);
    invalid_begin(780, 700, &config, &g_sdk, 0, NULL, KEY_TEXT, KEY_LEN);
    invalid_begin(780, 700, &config, &g_sdk, 0, &g_policy, NULL, KEY_LEN);
    invalid_begin(780, 700, &config, &g_sdk, 0, &g_policy, KEY_TEXT, 0);
    invalid_begin(780, 700, NULL, &g_sdk, 0, &g_policy, KEY_TEXT, KEY_LEN);
    invalid_begin(780, 700, &config, NULL, 0, &g_policy, KEY_TEXT, KEY_LEN);
    invalid_begin(780, 699, &config, &g_sdk, 0, &g_policy, KEY_TEXT, KEY_LEN);
    puts("invalid upload begin boundaries rejected");
    return 0;
}
'''


_STATUS_BODY = r'''
static void prepare(image2_upload *u, image2_upload_boot *b,
                    unsigned running, unsigned char *wire) {
    memcpy(wire + 80, image, 700);
    assert(start_valid(u, b, running, KEY_TEXT, KEY_LEN));
    feed(u, wire, 29);
}

static void status_verify_only(unsigned running) {
    image2_upload_boot b;
    image2_upload u;
    unsigned char wire[780] = {HEADER_BYTES};
    upload_setup(&b, &u, running, 1);
    prepare(&u, &b, running, wire);
    activating = 0;
    assert(image2_upload_finish(&u, 0));
    assert(image2_session_get_status(&u.session) == IMAGE2_SESSION_NOT_ACTIVATED);
    assert(!u.active && zero_auth(&u));
    /* Verify-only must close without publishing anything. */
    assert(!memcmp(mem + source + 8, "81958711", 8));
    assert(memcmp(mem + target + 8, "81958711", 8));
    image2_upload_abort(&u);
    assert(!image2_upload_finish(&u, 1));
    image2_session_abort(&u.session);
    image2_auth_cleanup(&u.auth);
    assert(image2_session_get_status(&u.session) == IMAGE2_SESSION_NOT_ACTIVATED);
    assert(zero_auth(&u));
    current_upload = NULL;
}

static void status_activated(unsigned running) {
    image2_upload_boot b;
    image2_upload u;
    unsigned char wire[780] = {HEADER_BYTES};
    upload_setup(&b, &u, running, 1);
    prepare(&u, &b, running, wire);
    activating = 1;
    assert(image2_upload_finish(&u, 1));
    assert(image2_session_get_status(&u.session) == IMAGE2_SESSION_ACTIVATED);
    assert(!u.active && zero_auth(&u));
    assert(!memcmp(mem + target, image, 700));
    assert(!memcmp(mem + source + 8, "01958711", 8));
    /* Terminal and repeated cleanup must not erase ACTIVATED. */
    image2_upload_abort(&u);
    assert(!image2_upload_finish(&u, 1));
    image2_session_abort(&u.session);
    image2_auth_cleanup(&u.auth);
    assert(image2_session_get_status(&u.session) == IMAGE2_SESSION_ACTIVATED);
    assert(zero_auth(&u));
    current_upload = NULL;
}

static void status_uncertain(unsigned running) {
    image2_upload_boot b;
    image2_upload u;
    unsigned char wire[780] = {HEADER_BYTES};
    int verified_calls;
    upload_setup(&b, &u, running, 1);
    prepare(&u, &b, running, wire);
    activating = 0;
    assert(image2_upload_finish(&u, 0));
    verified_calls = calls;
    /* Re-run the identical prefix and fail the first publication callback. */
    upload_setup(&b, &u, running, 1);
    prepare(&u, &b, running, wire);
    fault = verified_calls + 1;
    mode = 0;
    status = 0;
    activating = 1;
    assert(!image2_upload_finish(&u, 1));
    assert(image2_session_get_status(&u.session) == IMAGE2_SESSION_UNCERTAIN);
    assert(!u.active && zero_auth(&u));
    assert(calls == verified_calls + 1);
    assert(!memcmp(mem + source + 8, "81958711", 8));
    assert(memcmp(mem + target + 8, "81958711", 8));
    image2_upload_abort(&u);
    assert(!image2_upload_finish(&u, 1));
    image2_session_abort(&u.session);
    image2_auth_cleanup(&u.auth);
    assert(image2_session_get_status(&u.session) == IMAGE2_SESSION_UNCERTAIN);
    assert(zero_auth(&u));
    current_upload = NULL;
}

int main(void) {
    keep_expected();
    status_verify_only(0);
    status_verify_only(1);
    status_activated(0);
    status_activated(1);
    status_uncertain(0);
    status_uncertain(1);
    puts("terminal upload statuses preserved");
    return 0;
}
'''


_LATCH_BODY = r'''
static void failed_auth_client(unsigned running) {
    image2_upload_boot b;
    image2_upload u1;
    image2_upload u2;
    unsigned char wire[780] = {HEADER_BYTES};
    upload_setup(&b, &u1, running, 1);
    memcpy(wire + 80, image, 700);
    wire[79] ^= 1; /* corrupt the HMAC before the envelope completes */
    assert(start_valid(&u1, &b, running, KEY_TEXT, KEY_LEN));
    assert(!image2_upload_write(&u1, wire, sizeof(wire)));
    assert(!u1.active && zero_auth(&u1));
    assert(!calls && !crypto_calls);
    assert(!memcmp(mem, before, sizeof(mem)));
    /* Failed auth consumed only the upload-level latch, not the session latch. */
    assert(b.used && !b.session.used);
    wire[79] ^= 1;
    memset(&u2, 0, sizeof(u2));
    current_session = &u2.session;
    current_upload = &u2;
    assert(!image2_upload_begin(&u2, &b, 780, &config, &g_sdk, running,
                                &g_policy, KEY_TEXT, KEY_LEN));
    assert(!u2.identity && !u2.active);
    assert(!image2_upload_write(&u2, wire, sizeof(wire)));
    assert(!image2_upload_finish(&u2, 1));
    image2_upload_abort(&u2);
    assert(!calls && !crypto_calls);
    assert(!memcmp(mem, before, sizeof(mem)));
    current_upload = NULL;
}

static void failed_begin_client(unsigned running) {
    image2_upload_boot b;
    image2_upload u1;
    image2_upload u2;
    upload_setup(&b, &u1, running, 1);
    assert(!image2_upload_begin(&u1, &b, 80, &config, &g_sdk, running,
                                &g_policy, KEY_TEXT, KEY_LEN));
    assert(!u1.active && zero_auth(&u1));
    assert(b.used && !b.session.used);
    memset(&u2, 0, sizeof(u2));
    current_session = &u2.session;
    current_upload = &u2;
    assert(!image2_upload_begin(&u2, &b, 780, &config, &g_sdk, running,
                                &g_policy, KEY_TEXT, KEY_LEN));
    assert(!u2.identity);
    assert(!calls && !crypto_calls);
    current_upload = NULL;
}

int main(void) {
    keep_expected();
    failed_auth_client(0);
    failed_auth_client(1);
    failed_begin_client(0);
    failed_begin_client(1);
    puts("shared one-boot latch denies later clients after failure");
    return 0;
}
'''


class UploadTest(unittest.TestCase):
    def _run_harness(self, body):
        if SDK is None:
            self.skipTest("SDK mbedTLS sources required; set AMB1_SDK_DIR")
        harness = _PREFIX + _COMMON + body
        harness = harness.replace("HEADER_BYTES", _HEADER)
        harness = harness.replace("BOARD_ID", str(BOARD))
        harness = harness.replace("KEY_TEXT", '"' + KEY.decode() + '"')
        harness = harness.replace("KEY_LEN", str(len(KEY)))
        with tempfile.TemporaryDirectory(prefix="image2-upload-") as temp:
            path = Path(temp)
            (path / "upload_config.h").write_text("#define MBEDTLS_SHA256_C\n")
            (path / "test.c").write_text(harness)
            exe = path / "test"
            subprocess.run(["cc", "-std=c99", "-Wall", "-Wextra", "-Werror",
                            "-Wno-unused-function", "-O2", "-fstrict-aliasing",
                            "-fsanitize=address,undefined",
                            '-DMBEDTLS_CONFIG_FILE="upload_config.h"',
                            "-I" + temp, "-I" + str(OTA),
                            "-I" + str(SDK / "include"),
                            str(path / "test.c"), *map(str, MODULES),
                            str(OTA / "image2_auth.c"),
                            str(OTA / "image2_upload.c"),
                            str(SDK / "library/sha256.c"), "-o", str(exe)],
                           check=True)
            subprocess.run([str(exe)], check=True,
                           env={**os.environ, "ASAN_OPTIONS": "detect_leaks=0"})

    def test_composition(self):
        if SDK is None:
            self.skipTest("SDK mbedTLS sources required; set AMB1_SDK_DIR")
        payload = bytearray(b"\xff" * 700)
        struct.pack_into("<II", payload, 0, 10, 0x10000000)
        payload[8:16] = b"81958711"
        struct.pack_into("<I", payload, 16, 0x10000001)
        payload[20:26] = b"RTKWin"
        struct.pack_into("<II", payload, 26, 658, 0x30000000)
        header = ",".join(map(str, envelope(payload)))
        harness = HARNESS.split("int main(void)")[0] + r'''
#undef session
#include "image2_upload.h"
static int zero_auth(image2_upload *u) {
    size_t i; const unsigned char *p=(const unsigned char *)&u->auth;
    for(i=0;i<sizeof(u->auth);i++) if(p[i]) return 0;
    for(i=0;i<sizeof(u->envelope);i++) if(u->envelope[i]) return 0;
    return !u->key && !u->key_size;
}
static void upload_case(int running, int kind, size_t chunk) {
    image2_upload_boot b={0}; image2_upload u={0}, other={0};
    image2_flash_sdkops sdk={mem,lock,unlock,read_bytes,program,erase};
    image2_auth_policy policy={1,700,1,BOARD_ID};
    unsigned char wire[780]={HEADER_BYTES};
    size_t pos, n; int ok=1;
    current_boot=&b.session; current_session=&u.session;
    fault=crypto_fault=0; setup(running,1);
    if(kind==2) { fault=6; mode=status=0; }
    memcpy(wire+80,image,700);
    if(kind>=10 && kind<90) wire[kind-10]^=1;
    if(kind==4) wire[779]^=1;
    assert(image2_upload_begin(&u,&b,780,&config,&sdk,running,&policy,KEY_TEXT,KEY_LEN));
    if(kind==7) { assert(!image2_upload_write(&u,NULL,1)); ok=0; }
    if(kind==8) { assert(!image2_upload_write(&u,wire,(size_t)-1)); ok=0; }
    for(pos=0;ok && pos<sizeof(wire)-(kind==3 ? 1u : 0u);pos+=n) {
        n=sizeof(wire)-(kind==3 ? 1u : 0u)-pos;
        if(n>chunk) n=chunk;
        ok=image2_upload_write(&u,wire+pos,n);
    }
    if(kind==5) { image2_upload_abort(&u); ok=0; }
    if(kind==6) { assert(!image2_upload_write(&u,"x",1)); ok=0; }
    if(ok) {
        activating=kind!=1;
        ok=image2_upload_finish(&u,kind==1 ? 0 : kind==9 ? 2 : 1);
    }
    assert(ok==(kind==0 || kind==1));
    assert(!locked && zero_auth(&u));
    if(kind==0) {
        assert(!memcmp(mem+target,image,700));
        assert(!memcmp(mem+source+8,"01958711",8));
    } else assert(memcmp(mem+target+8,"81958711",8));
    if(kind>=10 || kind==7 || kind==8) assert(!calls && !memcmp(mem,before,sizeof(mem)));
    assert(!image2_upload_begin(&other,&b,780,&config,&sdk,running,&policy,KEY_TEXT,KEY_LEN));
    image2_upload_abort(&u); assert(!image2_upload_finish(&u,1));
}
int main(void) {
    int r,k; size_t chunk;
    (void)expected_calls; (void)expected_crypto_calls;
    for(r=0;r<2;r++) {
        for(chunk=1;chunk<=780;chunk++) upload_case(r,0,chunk);
        for(k=1;k<90;k++) upload_case(r,k,17);
    }
    return 0;
}
'''
        harness = harness.replace("HEADER_BYTES", header).replace("BOARD_ID", str(BOARD))
        harness = harness.replace("KEY_TEXT", '"' + KEY.decode() + '"')
        harness = harness.replace("KEY_LEN", str(len(KEY)))
        with tempfile.TemporaryDirectory(prefix="image2-upload-") as temp:
            path = Path(temp)
            (path / "upload_config.h").write_text("#define MBEDTLS_SHA256_C\n")
            (path / "test.c").write_text(harness)
            exe = path / "test"
            subprocess.run(["cc", "-std=c99", "-Wall", "-Wextra", "-Werror",
                            "-Wno-unused-function", "-O2", "-fstrict-aliasing",
                            "-fsanitize=address,undefined", '-DMBEDTLS_CONFIG_FILE="upload_config.h"',
                            "-I" + temp, "-I" + str(OTA), "-I" + str(SDK / "include"),
                            str(path / "test.c"), *map(str, MODULES),
                            str(OTA / "image2_auth.c"), str(OTA / "image2_upload.c"),
                            str(SDK / "library/sha256.c"), "-o", str(exe)], check=True)
            subprocess.run([str(exe)], check=True,
                           env={**os.environ, "ASAN_OPTIONS": "detect_leaks=0"})

    def test_upload_callback_reentry(self):
        self._run_harness(_REENTRY_BODY)

    def test_borrowed_key_lifetime(self):
        self._run_harness(_KEY_BODY)

    def test_begin_boundaries(self):
        self._run_harness(_BOUNDARY_BODY)

    def test_terminal_status_preservation(self):
        self._run_harness(_STATUS_BODY)

    def test_shared_boot_latch_after_failure(self):
        self._run_harness(_LATCH_BODY)


if __name__ == "__main__":
    unittest.main()
