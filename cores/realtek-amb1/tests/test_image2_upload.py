"""Native upload composition: real auth/SHA and all four session modules."""
import os
from pathlib import Path
import subprocess
import tempfile
import unittest

from test_image2_auth import SDK, KEY, BOARD, envelope
from test_image2_session import HARNESS, MODULES, OTA


class UploadTest(unittest.TestCase):
    def test_composition(self):
        if SDK is None:
            self.skipTest("SDK mbedTLS sources required; set AMB1_SDK_DIR")
        payload = bytearray(b"\xff" * 700)
        import struct
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


if __name__ == "__main__":
    unittest.main()
