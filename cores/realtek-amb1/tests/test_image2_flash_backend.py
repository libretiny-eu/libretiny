"""Compile and fault-inject the actual injection-only C controller."""
import pathlib
import subprocess
import tempfile
import unittest


ROOT = pathlib.Path(__file__).resolve().parents[1]
OTA = ROOT / "base/ota"

HARNESS = r'''
#include "image2_flash_backend.h"
#include <assert.h>
#include <stdint.h>
#include <string.h>

typedef struct {
    unsigned char mem[4096];
    const void *owner;
    int reads, writes, erases, locks, unlocks;
    int read_ret, write_ret, erase_ret, lock_ret;
    int partial_write, partial_erase, corrupt, fail_read_at, reenter;
} sim;
static sim s;
static image2_flash_backend b;
static image2_flash_config c;
static image2_flash_sdkops ops;
static int lock(void *p, const void *owner) {
    sim *x=p; x->locks++;
    if (x->owner || x->lock_ret != 1) return x->owner ? 0 : x->lock_ret;
    x->owner=owner; return 1;
}
static void unlock(void *p, const void *owner) {
    sim *x=p; assert(x->owner==owner); x->owner=0; x->unlocks++;
}
static int read_bytes(void *p, uint32_t a, void *out, size_t n) {
    sim *x=p; assert(x->owner==&b); assert(n && a<4096 && n<=4096-a);
    x->reads++;
    if (x->reenter) { x->reenter=0; assert(!image2_flash_read(&b,a,out,n)); }
    if (x->fail_read_at==x->reads) return 0;
    memcpy(out,x->mem+a,n); return x->read_ret;
}
static int program(void *p, uint32_t a, const void *in, size_t n) {
    sim *x=p; const unsigned char *v=in; size_t i, count=n;
    assert(x->owner==&b); assert(a>=2048 && n<=3072-a);
    assert(n && a/64==(a+n-1)/64); x->writes++;
    if (x->partial_write) count=n/2;
    for(i=0;i<count;i++) x->mem[a+i]&=v[i];
    if (x->corrupt) x->mem[2049]&=(unsigned char)~2u;
    return x->write_ret;
}
static int erase(void *p, uint32_t a) {
    sim *x=p; assert(x->owner==&b); assert(a>=2048 && a<3072 && a%256==0);
    x->erases++; memset(x->mem+a,255,x->partial_erase?128:256);
    return x->erase_ret;
}
static void setup(void) {
    memset(&s,0,sizeof(s)); memset(&b,0,sizeof(b)); memset(&c,0,sizeof(c));
    memset(s.mem,255,sizeof(s.mem));
    s.read_ret=s.write_ret=s.erase_ret=s.lock_ret=1;
    c.layout.flash_size=4096; c.layout.sector_size=256;
    c.layout.slots[0]=(image2_range){256,1280};
    c.layout.slots[1]=(image2_range){2048,3072};
    c.layout.reviewed=c.layout.slots[1];
    c.layout.reviewed_layout=c.layout.normal_selector=1;
    c.layout.protected_count=1;
    c.layout.protected_ranges[0]=(image2_range){0,256};
    c.image_size=700; c.page_size=64; c.extent=(image2_range){2048,2816};
    ops=(image2_flash_sdkops){&s,lock,unlock,read_bytes,program,erase,NULL};
}
static void opened(void) { setup(); assert(image2_flash_open(&b,&c,&ops)); }
static void dead(void) {
    unsigned char v=0;
    int r=s.reads,w=s.writes,e=s.erases;
    assert(!image2_flash_read(&b,2048,&v,1));
    assert(!image2_flash_write(&b,2048,&v,1));
    assert(!image2_flash_erase(&b,2048,256));
    assert(r==s.reads && w==s.writes && e==s.erases);
    assert(s.owner==&b); image2_flash_close(&b); assert(!s.owner);
    image2_flash_close(&b); assert(s.unlocks==1);
    assert(!image2_flash_open(&b,&c,&ops));
}
static void bad_geometry(void) {
    assert(!image2_flash_open(&b,&c,&ops));
    assert(!s.reads && !s.writes && !s.erases); dead();
}
static int authorize(void *p, uint32_t n) { (void)p; return n==700; }
static int hash_reset(void *p) { (void)p; return 1; }
static int hash_update(void *p, const void *v, size_t n) {
    (void)p; (void)v; (void)n; return 1;
}
static void le32(unsigned char *v, uint32_t n) {
    v[0]=(unsigned char)n; v[1]=(unsigned char)(n>>8);
    v[2]=(unsigned char)(n>>16); v[3]=(unsigned char)(n>>24);
}
static void transaction_integration(void) {
    unsigned char image[700]; image2_transaction t={0};
    image2_callbacks io={&b,image2_flash_read,image2_flash_erase,image2_flash_write,
                        authorize,hash_reset,hash_update,hash_reset};
    setup();
    c.layout.sram_base=0x10000000; c.layout.sram_end=0x10010000;
    c.layout.sdram_base=0x30000000; c.layout.sdram_end=0x30010000;
    memcpy(s.mem+264,"81958711",8);
    memset(image,255,sizeof(image));
    le32(image,10); le32(image+4,c.layout.sram_base);
    memcpy(image+8,"81958711",8); le32(image+16,c.layout.sram_base+1);
    memcpy(image+20,"RTKWin",6); le32(image+26,658);
    le32(image+30,c.layout.sdram_base);
    assert(image2_flash_open(&b,&c,&ops));
    assert(image2_begin(&t,&c.layout,&io,sizeof(image)));
    assert(image2_write(&t,image,sizeof(image)));
    assert(!memcmp(s.mem+2056,"\xff\xff\xff\xff\xff\xff\xff\xff",8));
    assert(image2_flash_phase(&b,2)); assert(image2_verify(&t));
    assert(image2_flash_phase(&b,3)); assert(image2_commit(&t));
    assert(!memcmp(s.mem+2048,image,sizeof(image)));
    assert(!image2_commit(&t)); image2_flash_close(&b);
}
int main(void) {
    unsigned char data[500], out[500]; int k, ret;
    memset(data,0x5a,sizeof(data));
    opened();
    assert(image2_flash_read(&b,264,out,8));
    assert(image2_flash_write(&b,2049,data,sizeof(data)));
    assert(s.writes>7 && !memcmp(s.mem+2049,data,sizeof(data)));
    assert(s.mem[2048]==255 && s.mem[2549]==255);
    assert(image2_flash_read(&b,2049,out,sizeof(out)) && !memcmp(out,data,sizeof(out)));
    assert(image2_flash_erase(&b,2048,256));
    for(k=2048;k<2304;k++) assert(s.mem[k]==255);
    image2_flash_close(&b); assert(!image2_flash_read(&b,2048,out,1));
    /* Verification is read-only; publication cannot erase or program bodies.
     * The dedicated source capability denies premature calls without SDK I/O.
     * Partial source reads are not an accidental general active-slot reader. */
    opened(); assert(image2_flash_phase(&b,2));
    assert(!image2_flash_write(&b,2048,data,1)); dead();
    opened(); assert(image2_flash_phase(&b,2));
    assert(!image2_flash_erase(&b,2048,256)); dead();
    opened(); assert(image2_flash_phase(&b,2)); assert(image2_flash_phase(&b,3));
    assert(!image2_flash_write(&b,2048,data,1)); dead();
    opened(); assert(image2_flash_phase(&b,2)); assert(image2_flash_phase(&b,3));
    assert(!image2_flash_erase(&b,2048,256)); dead();
    opened(); assert(!image2_flash_demote(&b));
    assert(!s.reads && !s.writes && !s.erases); dead();
    opened(); assert(!image2_flash_read(&b,264,out,1));
    assert(!s.reads); dead();
    /* Exact extent, source, protected intersections, and malformed geometry. */
    for(k=0;k<14;k++) {
        setup();
        switch(k) {
        case 0:c.extent.end+=256;break;
        case 1:c.extent.start++;break;
        case 2:c.layout.running_slot=2;break;
        case 3:c.layout.slots[0]=c.layout.slots[1];break;
        case 4:c.layout.protected_ranges[0]=(image2_range){2815,2817};break;
        case 5:c.layout.protected_ranges[0]=(image2_range){2047,2049};break;
        case 6:c.layout.protected_ranges[0]=(image2_range){7,7};break;
        case 7:c.image_size=UINT32_MAX;break;
        case 8:c.layout.sector_size=0;break;
        case 9:c.page_size=3;break;
        case 10:c.page_size=0;break;
        case 11:c.layout.reviewed_layout=0;break;
        case 12:c.layout.normal_selector=0;break;
        case 13:c.layout.protected_count=IMAGE2_MAX_PROTECTED+1;break;
        }
        bad_geometry();
    }
    /* No I/O for zero, overflow, source, overrun, chip/block, unaligned erase. */
    for(k=0;k<12;k++) {
        opened();
        switch(k) {
        case 0:ret=image2_flash_write(&b,2048,data,0);break;
        case 1:ret=image2_flash_write(&b,2048,data,SIZE_MAX);break;
        case 2:ret=image2_flash_write(&b,UINT32_MAX,data,2);break;
        case 3:ret=image2_flash_write(&b,264,data,8);break;
        case 4:ret=image2_flash_write(&b,2815,data,2);break;
        case 5:ret=image2_flash_erase(&b,2049,256);break;
        case 6:ret=image2_flash_erase(&b,0,4096);break;
        case 7:ret=image2_flash_erase(&b,2048,512);break;
        case 8:ret=image2_flash_erase(&b,2048,0);break;
        case 9:ret=image2_flash_read(&b,263,out,2);break;
        case 10:ret=image2_flash_read(&b,2048,out,0);break;
        default:ret=image2_flash_write(&b,2048,0,1);break;
        }
        assert(!ret && !s.reads && !s.writes && !s.erases); dead();
    }
    /* Full-request NOR preflight: late impossible bit cannot mutate prefix. */
    opened(); s.mem[2400]=0;
    assert(!image2_flash_write(&b,2049,data,sizeof(data)) && !s.writes); dead();
    for(k=0;k<3;k++) {
        int result=k==0?0:k==1?-1:2;
        opened(); s.read_ret=result;
        assert(!image2_flash_read(&b,2048,out,1)); dead();
        opened(); s.write_ret=result;
        assert(!image2_flash_write(&b,2049,data,20));
        assert(s.writes==1 && s.mem[2049]==0x5a); dead();
        opened(); s.erase_ret=result; memset(s.mem+2048,0,256);
        assert(!image2_flash_erase(&b,2048,256));
        assert(s.erases==1 && s.mem[2048]==255); dead();
    }
    opened(); s.partial_write=1;
    assert(!image2_flash_write(&b,2049,data,20)); dead();
    opened(); s.partial_erase=1; memset(s.mem+2048,0,256);
    assert(!image2_flash_erase(&b,2048,256)); dead();
    opened(); s.corrupt=1;
    assert(!image2_flash_write(&b,2049,data,sizeof(data))); dead();
    /* 500-byte preflight takes four reads; fail first full readback read. */
    opened(); s.fail_read_at=5;
    assert(!image2_flash_write(&b,2049,data,sizeof(data)) && s.writes>1); dead();
    opened(); s.fail_read_at=2;
    assert(!image2_flash_erase(&b,2048,256)); dead();
    opened(); s.reenter=1;
    assert(!image2_flash_write(&b,2049,data,20) && !s.writes); dead();
    /* Shared lock denies another client and copied capabilities cannot unlock. */
    opened(); {
        image2_flash_backend other={0}, copy=b;
        assert(!image2_flash_open(&other,&c,&ops));
        assert(!image2_flash_read(&copy,2048,out,1));
        image2_flash_close(&copy); assert(s.owner==&b);
        image2_flash_close(&b); assert(!s.owner);
    }
    setup(); s.lock_ret=2;
    assert(!image2_flash_open(&b,&c,&ops) && !s.owner && !b.held);
    transaction_integration();
    return 0;
}
'''


class Image2FlashBackendTests(unittest.TestCase):
    def test_actual_c_fault_injection(self):
        preferred_tmp = pathlib.Path(
            "/private/var/folders/4q/bvwkg_397v102xfwcpxgdymc0000gn/T/opencode"
        )
        with tempfile.TemporaryDirectory(
            prefix="image2-backend-",
            dir=preferred_tmp if preferred_tmp.is_dir() else None,
        ) as directory:
            path = pathlib.Path(directory)
            source = path / "faults.c"
            source.write_text(HARNESS)
            executable = path / "faults"
            subprocess.run(
                ["cc", "-std=c99", "-Wall", "-Wextra", "-Werror",
                 "-fsanitize=address,undefined", "-fno-omit-frame-pointer",
                 "-I", str(OTA), str(source), str(OTA / "image2_flash_backend.c"),
                 str(OTA / "image2_transaction.c"),
                 "-o", str(executable)], check=True,
            )
            subprocess.run([str(executable)], check=True)


if __name__ == "__main__":
    unittest.main()
