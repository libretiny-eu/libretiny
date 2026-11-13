"""Actual four-module integration, with ASan/UBSan SDK fault injection."""
from pathlib import Path
import os
import shutil
import subprocess
import tempfile
import unittest

OTA = Path(__file__).resolve().parents[1] / "base/ota"
MODULES = tuple(OTA / ("image2_" + name + ".c") for name in
                ("transaction", "activation", "flash_backend", "session"))
HARNESS = r'''
#include "image2_session.h"
#include <assert.h>
#include <stdio.h>
#include <string.h>

static image2_session_boot *current_boot;
static image2_session *current_session;
#define boot (*current_boot)
#define session (*current_session)
static image2_flash_config config;
static unsigned char mem[4096], before[4096], image[700];
static const void *owner;
static unsigned source, target;
static int calls, fault, mode, status, unlocks, locks, activating, locked;
static int expected_calls, expected_crypto_calls, crypto_fault, crypto_calls;
static unsigned long digest, expected_digest;
static int crypto_cookie;
static int hit(void) { assert(locked); return ++calls == fault; }
static void reentry(void) {
    assert(!image2_session_write(&session,"x",1));
    assert(!image2_session_verify(&session));
    assert(!image2_session_activate(&session));
    image2_session_abort(&session);
}
static int lock(void *p, const void *who) {
    int f;
    assert(p==mem && !locked); locks++; owner=who; locked=1;
    f=hit(); reentry();
    if(f) { locked=0; owner=0; return status; }
    return 1;
}
static void unlock(void *p, const void *who) {
    assert(p==mem && locked && who==owner);
    unlocks++; locked=0; owner=0; reentry();
}
static int read_bytes(void *p, uint32_t a, void *out, size_t n) {
    int f=hit(); assert(p==mem); reentry();
    assert((a==source+8 && n==8) || (a>=target && a+n<=target+768));
    if(f && mode==0) return status;
    if(f && mode==2) { memcpy(out,mem+a,n/2); return status; }
    memcpy(out,mem+a,n); return f ? status : 1;
}
static int program(void *p, uint32_t a, const void *in, size_t n) {
    const unsigned char *v=in; size_t i, count=n; int f=hit();
    assert(p==mem && n && a/64==(a+n-1)/64); reentry();
    if(a==source+8) {
        assert(activating && n==1 && v[0]=='0');
        assert(!memcmp(mem+source+8,"81958711",8));
        assert(!memcmp(mem+target+8,"81958711",8));
    } else {
        assert(a>=target && a+n<=target+768);
        if(activating) assert(a==target+8 && n==8 && !memcmp(v,"81958711",8));
    }
    if(f && mode==0) return status;
    if(f && mode==2) count=n/2;
    for(i=0;i<count;i++) mem[a+i]&=v[i];
    return f ? status : 1;
}
static int erase(void *p, uint32_t a) {
    int f=hit(); assert(p==mem && !activating && a>=target && a<target+768 && a%256==0);
    reentry();
    if(f && mode==0) return status;
    memset(mem+a,255,f && mode==2 ? 128 : 256);
    return f ? status : 1;
}
static int crypto_ok(void *p) {
    assert(p==&crypto_cookie && locked); reentry();
    return ++crypto_calls==crypto_fault ? status : 1;
}
static int authorize(void *p, uint32_t n) { assert(n==700); return crypto_ok(p); }
static int reset(void *p) { digest=0; return crypto_ok(p); }
static int update(void *p, const void *v, size_t n) {
    const unsigned char *s=v; size_t i;
    for(i=0;i<n;i++) digest=digest*33+s[i];
    return crypto_ok(p);
}
static int check(void *p) { return crypto_ok(p)==1 && digest==expected_digest; }
static void le32(unsigned char *p, uint32_t n) {
    p[0]=(unsigned char)n; p[1]=(unsigned char)(n>>8);
    p[2]=(unsigned char)(n>>16); p[3]=(unsigned char)(n>>24);
}
static void setup(int running, int old) {
    size_t i;
    memset(&config,0,sizeof(config)); memset(mem,0xa5,sizeof(mem));
    calls=unlocks=locks=locked=activating=crypto_calls=0; owner=0;
    config.layout.flash_size=4096; config.layout.sector_size=256;
    config.layout.slots[0]=(image2_range){256,1280};
    config.layout.slots[1]=(image2_range){2048,3072};
    config.layout.running_slot=(unsigned)running;
    source=config.layout.slots[running].start;
    target=config.layout.slots[1-running].start;
    config.layout.reviewed=config.layout.slots[1-running];
    config.layout.reviewed_layout=config.layout.normal_selector=1;
    config.layout.sram_base=0x10000000; config.layout.sram_end=0x10010000;
    config.layout.sdram_base=0x30000000; config.layout.sdram_end=0x30010000;
    config.layout.protected_count=2;
    config.layout.protected_ranges[0]=(image2_range){0,256};
    config.layout.protected_ranges[1]=config.layout.slots[running];
    config.image_size=700; config.page_size=64; config.allow_source_demote=1;
    config.extent=(image2_range){target,target+768};
    memcpy(mem+source+8,"81958711",8);
    if(old) memcpy(mem+target+8,"01958711",8);
    else memset(mem+target+8,255,8);
    memset(image,255,sizeof(image)); le32(image,10);
    le32(image+4,config.layout.sram_base); memcpy(image+8,"81958711",8);
    le32(image+16,config.layout.sram_base+1); memcpy(image+20,"RTKWin",6);
    le32(image+26,658); le32(image+30,config.layout.sdram_base);
    expected_digest=0;
    for(i=0;i<sizeof(image);i++) expected_digest=expected_digest*33+image[i];
    memcpy(before,mem,sizeof(mem));
}
static int begin(void) {
    image2_flash_sdkops sdk={mem,lock,unlock,read_bytes,program,erase};
    image2_session_crypto crypto={&crypto_cookie,authorize,reset,update,check};
    return image2_session_begin(&session,&boot,&config,&sdk,&crypto,config.layout.running_slot);
}
static int run(void) {
    if(!begin()) return 0;
    /* Callers cannot alter the snapshotted geometry. */
    config.image_size=1; config.extent.start=0;
    if(!image2_session_write(&session,image,17) ||
       !image2_session_write(&session,image+17,sizeof(image)-17) ||
       !image2_session_verify(&session)) return 0;
    activating=1;
    return image2_session_activate(&session);
}
static void unused(void) {
    image2_session fresh={0};
    assert(image2_session_get_status(&fresh)==IMAGE2_SESSION_UNUSED);
    assert(image2_session_get_status(NULL)==IMAGE2_SESSION_UNUSED);
    assert(!image2_session_write(&fresh,image,1));
    assert(!image2_session_verify(&fresh));
    assert(!image2_session_activate(&fresh));
    image2_session_abort(&fresh);
}
static void terminal(int ok) {
    size_t i; int count=calls;
    image2_session fresh={0};
    image2_flash_sdkops sdk={mem,lock,unlock,read_bytes,program,erase};
    image2_session_crypto crypto={&crypto_cookie,authorize,reset,update,check};
    image2_session_status result=image2_session_get_status(&session);
    assert(!locked && unlocks==(locks && fault!=1 ? 1 : 0));
    assert(result==(ok ? IMAGE2_SESSION_ACTIVATED :
                     activating ? IMAGE2_SESSION_UNCERTAIN : IMAGE2_SESSION_NOT_ACTIVATED));
    assert(!image2_session_begin(&fresh,&boot,&config,&sdk,&crypto,0));
    assert(image2_session_get_status(&fresh)==IMAGE2_SESSION_UNUSED);
    {
        image2_session copy=session;
        assert(image2_session_get_status(&copy)==IMAGE2_SESSION_UNUSED);
        assert(!image2_session_write(&copy,image,1));
        assert(!image2_session_verify(&copy));
        assert(!image2_session_activate(&copy));
        image2_session_abort(&copy);
    }
    assert(!begin()); assert(!image2_session_write(&session,image,1));
    assert(!image2_session_verify(&session)); assert(!image2_session_activate(&session));
    image2_session_abort(&session); image2_session_abort(&session); assert(calls==count);
    for(i=0;i<sizeof(mem);i++)
        if(!(i>=target && i<target+768) && i!=source+8) assert(mem[i]==before[i]);
    if(ok) {
        assert(!memcmp(mem+source+8,"01958711",8));
        assert(!memcmp(mem+target,image,sizeof(image)));
    }
}
/* Each call models a distinct boot lifetime with newly initialized objects.
 * Never reset a used boot latch, even between independent fault scenarios. */
static int scenario(int running, int old, int kind) {
    image2_session_boot fresh_boot={0};
    image2_session fresh_session={0};
    int ok=0;
    current_boot=&fresh_boot; current_session=&fresh_session;
    setup(running,old); unused();
    switch(kind) {
    case 0: ok=run(); break;
    case 1: assert(begin()); image2_session_abort(&session); break;
    case 2: config.page_size=3; assert(!begin()); break;
    case 3: config.allow_source_demote=0; assert(!begin()); break;
    case 4: assert(begin()); assert(!image2_session_activate(&session)); break;
    case 5:
    case 6:
        assert(begin());
        assert(image2_session_write(&session,image,sizeof(image)));
        assert(image2_session_verify(&session));
        if(kind==5) assert(!image2_session_write(&session,image,1));
        else assert(!image2_session_verify(&session));
        break;
    default: assert(0);
    }
    terminal(ok);
    current_boot=NULL; current_session=NULL;
    return ok;
}
int main(void) {
    int running, old, f, m, r, total=0;
    for(running=0;running<2;running++) for(old=0;old<2;old++) {
        fault=crypto_fault=0; assert(scenario(running,old,0));
        expected_calls=calls; expected_crypto_calls=crypto_calls; total++;
        for(f=1;f<=expected_calls;f++) for(m=0;m<3;m++) for(r=0;r<3;r++) {
            fault=f; mode=m; status=r==0 ? 0 : r==1 ? -1 : 2;
            assert(!scenario(running,old,0)); total++;
        }
        /* False success must be caught for every partial SDK mutation. Reads
         * return partial data too; unchanged bytes can legitimately match. */
        for(f=2;f<=expected_calls;f++) {
            fault=f; mode=2; status=1;
            scenario(running,old,0); total++;
        }
        fault=0;
        for(f=1;f<=expected_crypto_calls;f++) for(r=0;r<3;r++) {
            crypto_fault=f; status=r==0 ? 0 : r==1 ? -1 : 2;
            scenario(running,old,0); total++;
        }
        crypto_fault=0;
        for(r=1;r<=6;r++) { assert(!scenario(running,old,r)); total++; }
    }
    printf("%d linked fault-injection scenarios passed\n",total);
    return 0;
}
'''


class SessionTest(unittest.TestCase):
    def run_integration(self, flags):
        with tempfile.TemporaryDirectory(prefix="image2-session-") as temp:
            source = Path(temp) / "session.c"
            exe = Path(temp) / "session"
            source.write_text(HARNESS)
            subprocess.run(["cc", "-std=c99", "-Wall", "-Wextra", "-Werror", "-pedantic",
                            *flags,
                            "-I", str(OTA), str(source),
                            *map(str, MODULES),
                            "-o", str(exe)], check=True)
            subprocess.run([str(exe)], check=True,
                            env={**os.environ, "ASAN_OPTIONS": "detect_leaks=0"})

    def test_linked_sanitized_integration(self):
        self.run_integration(["-O2", "-fstrict-aliasing", "-fsanitize=address,undefined",
                              "-fno-omit-frame-pointer", "-g"])

    def test_linked_optimized_strict_aliasing(self):
        self.run_integration(["-O2", "-fstrict-aliasing", "-Wstrict-aliasing"])

    def test_cpp_typed_storage(self):
        with tempfile.TemporaryDirectory(prefix="image2-session-cpp-") as temp:
            source = Path(temp) / "session.cpp"
            source.write_text('''#include "image2_session.h"
static image2_session_boot boot{};
static image2_session session{};
int main() {
    return image2_session_begin(&session, &boot, nullptr, nullptr, nullptr, 0);
}
''')
            subprocess.run(["c++", "-std=c++11", "-Wall", "-Wextra", "-Werror",
                            "-pedantic", "-O2", "-fstrict-aliasing", "-I", str(OTA),
                            "-c", str(source), "-o", str(Path(temp) / "session.o")],
                           check=True)

    def test_arm_optimized_compile_only(self):
        compiler = (os.environ.get("ARM_NONE_EABI_GCC") or
                    shutil.which("arm-none-eabi-gcc"))
        if not compiler:
            installed = (Path.home() / ".platformio/packages/toolchain-gccarmnoneeabi"
                         / "bin/arm-none-eabi-gcc")
            if installed.is_file():
                compiler = str(installed)
        if not compiler:
            self.skipTest("arm-none-eabi-gcc not installed")
        with tempfile.TemporaryDirectory(prefix="image2-session-arm-") as temp:
            source = Path(temp) / "session.c"
            source.write_text(HARNESS)
            for unit in (*MODULES, source):
                subprocess.run([compiler, "-std=c99", "-Wall", "-Wextra", "-Werror",
                                "-pedantic", "-O2", "-fstrict-aliasing",
                                "-Wstrict-aliasing=2", "-mcpu=cortex-m3", "-mthumb",
                                "-I", str(OTA), "-c", str(unit), "-o",
                                str(Path(temp) / (unit.stem + ".o"))], check=True)


if __name__ == "__main__":
    unittest.main()
