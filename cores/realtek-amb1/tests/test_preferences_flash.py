#!/usr/bin/env python3
"""Host-only NOR tests linking real installed FlashDB and FAL, never the SDK.

Set FLASHDB_SOURCE to an upstream FlashDB source tree if not installed via PIO.
The portal record is a representative opaque WiFi preference blob, not a claim
that the ESPHome portal or a hardware SDK was executed.
"""
import os
from pathlib import Path
import shutil
import subprocess
import tempfile
import unittest


PORT = Path(__file__).resolve().parents[1] / "base/port"
FLASHDB = Path(os.environ.get(
    "FLASHDB_SOURCE", str(Path.home() / ".platformio/packages/library-flashdb")))

HARNESS = r'''
#include <assert.h>
#include <limits.h>
#include <string.h>
#include <flashdb.h>
#include "preferences_flash.h"

static unsigned char memory[0x4000], baseline[0x4000];
static amb1_preferences_flash_t bridge;
static int held, locks, unlocks, writes, erases;
static int fail_lock, fail_read, fail_write, fail_erase, corrupt_write, corrupt_erase;
static int fail_erase_at;
static unsigned char *ptr(uint32_t a, size_t n) {
    assert(held && a >= AMB1_PREF_KVS_START && a < AMB1_PREF_KVS_END);
    assert(n && n <= AMB1_PREF_KVS_END - a);
    return memory + 0x1000 + a - AMB1_PREF_KVS_START;
}
static int lock(void *c) {
    (void)c; assert(!held); if (fail_lock) return -1;
    held = 1; ++locks; return 0;
}
static void unlock(void *c) { (void)c; assert(held); held = 0; ++unlocks; }
static int read_flash(void *c, uint32_t a, void *p, size_t n) {
    (void)c; unsigned char *s = ptr(a,n);
    if (fail_read) return -1;
    memcpy(p,s,n); return 0;
}
static int write_flash(void *c, uint32_t a, const void *p, size_t n) {
    (void)c; unsigned char *s = ptr(a,n); const unsigned char *b = p;
    ++writes;
    if (fail_write) return -1;
    for (size_t i=0;i<n;i++) { assert((s[i]&b[i])==b[i]); s[i]&=b[i]; }
    if (corrupt_write) s[0] ^= 1;
    return 0;
}
static int erase_flash(void *c, uint32_t a, size_t n) {
    (void)c; unsigned char *s = ptr(a,n);
    assert(!(a%4096) && n==4096); ++erases;
    if (fail_erase || (fail_erase_at && erases==fail_erase_at)) return -1;
    memset(s,255,n); if (corrupt_erase) s[0]=0;
    return 0;
}
static amb1_preferences_flash_ops_t ops = {
    NULL, read_flash, write_flash, erase_flash, lock, unlock
};
static amb1_preferences_flash_policy_t policy(void) {
    amb1_preferences_flash_policy_t p = {0};
    p.review = (amb1_preferences_review_t){1,1,1,1,1,1};
    memset(p.expected_snapshot_hash,0x42,32); memset(p.fresh_snapshot_hash,0x42,32);
    p.fresh_snapshot_attested=1; p.shared_controller_lock_attested=1;
    return p;
}
static void adjacent(void) {
    assert(!memcmp(memory,baseline,0x1000));
    assert(!memcmp(memory+0x3000,baseline+0x3000,0x1000));
    assert(!held && locks==unlocks);
}
static int fr(long o,uint8_t *p,size_t n) { return amb1_preferences_flash_read(&bridge,o,p,n); }
static int fw(long o,const uint8_t *p,size_t n) { return amb1_preferences_flash_write(&bridge,o,p,n); }
static int fe(long o,size_t n) { return amb1_preferences_flash_erase(&bridge,o,n); }
const struct fal_flash_dev preferences_test_dev = {
    "prefs",0,8192,4096,{NULL,fr,fw,fe},1
};

static void transport_tests(void) {
    unsigned char b[300]; memset(b,0xaa,sizeof(b));
    assert(fr(0,b,1)==-1); assert(fw(0,b,1)==-1); assert(fe(0,4096)==-1);
    amb1_preferences_flash_policy_t p=policy();
    assert(!amb1_preferences_flash_bind(&bridge,&ops,&p));
    assert(!amb1_preferences_flash_format_allowed(&bridge));
    memcpy(memory+0x1000,"foreign",7);
    assert(fr(0,b,7)==7 && !memcmp(b,"foreign",7));
    assert(fw(0,b,7)==-1 && fe(0,4096)==-1);
    p.initial_format_approved=1;
    assert(!amb1_preferences_flash_bind(&bridge,&ops,&p));
    assert(fe(0,8192)==8192);
    for (long o=-4096;o<16384;o+=4096)
        assert(fe(o,4096)==((o==0||o==4096)?4096:-1));
    assert(fe(1,4096)==-1 && fe(0,4095)==-1 && fe(4096,8192)==-1);
    assert(fr(LONG_MAX,b,1)==-1 && fw(-1,b,1)==-1);
    assert(fr(8191,b,2)==-1 && fw(0,b,(size_t)-1)==-1);
    assert(fr(0,NULL,1)==-1 && fw(0,NULL,1)==-1 && fr(0,b,0)==-1);
    memset(b,0xaa,sizeof(b)); assert(fw(0,b,sizeof(b))==sizeof(b));
    memset(b,0xff,sizeof(b)); int before=writes;
    assert(fw(0,b,sizeof(b))==-1 && writes==before);
    fail_lock=1; assert(fr(0,b,1)==-1); fail_lock=0;
    fail_read=1; assert(fr(0,b,1)==-1 && fw(300,b,1)==-1); fail_read=0;
    fail_write=1; assert(fw(300,b,1)==-1); fail_write=0;
    corrupt_write=1; assert(fw(300,b,1)==-1); corrupt_write=0;
    fail_erase=1; assert(fe(0,4096)==-1); fail_erase=0;
    corrupt_erase=1; assert(fe(0,4096)==-1); corrupt_erase=0;
    for (unsigned i=0;i<6;i++) {
        amb1_preferences_flash_policy_t bad=p;
        /* Review is six named integer flags, not a packed byte policy. */
        int *flags[] = {&bad.review.reviewed_reservation,&bad.review.snapshot_matches_review,
            &bad.review.migrated_native_loader,&bad.review.normal_selector,
            &bad.review.inactive_b_reservation,&bad.review.admitted_images_exclude_kvs};
        *flags[i]=0;
        assert(amb1_preferences_flash_bind(&bridge,&ops,&bad)==-1);
        assert(fr(0,b,1)==-1);
    }
    p.fresh_snapshot_hash[0]^=1; assert(amb1_preferences_flash_bind(&bridge,&ops,&p)==-1);
    p=policy(); p.fresh_snapshot_attested=0;
    assert(amb1_preferences_flash_bind(&bridge,&ops,&p)==-1);
    p=policy(); p.shared_controller_lock_attested=0;
    assert(amb1_preferences_flash_bind(&bridge,&ops,&p)==-1);
    p=policy(); memset(p.expected_snapshot_hash,0,32); memset(p.fresh_snapshot_hash,0,32);
    assert(amb1_preferences_flash_bind(&bridge,&ops,&p)==-1);
    adjacent();
}

static fdb_err_t init(struct fdb_kvdb *db) {
    memset(db,0,sizeof(*db));
    bool deny=!amb1_preferences_flash_format_allowed(&bridge);
    fdb_kvdb_control(db,FDB_KVDB_CTRL_SET_NOT_FORMAT,&deny);
    return fdb_kvdb_init(db,"portal","preferences",NULL,NULL);
}
struct portal_wifi { char ssid[33]; char password[65]; uint32_t sequence; };
static void check(struct fdb_kvdb *db,const struct portal_wifi *expected) {
    struct portal_wifi got; struct fdb_blob blob;
    memset(&got,0,sizeof(got));
    assert(fdb_kv_get_blob(db,"portal_wifi",fdb_blob_make(&blob,&got,sizeof(got)))==sizeof(got));
    assert(!memcmp(&got,expected,sizeof(got)));
}
static void database_tests(void) {
    amb1_preferences_flash_policy_t p=policy(); struct fdb_kvdb db;
    memset(memory+0x1000,0x5a,8192);
    assert(!amb1_preferences_flash_bind(&bridge,&ops,&p));
    int before=erases;
    assert(init(&db)!=FDB_NO_ERR && erases==before);
    for (unsigned i=0;i<8192;i++) assert(memory[0x1000+i]==0x5a);
    p.initial_format_approved=1;
    assert(!amb1_preferences_flash_bind(&bridge,&ops,&p));
    assert(init(&db)==FDB_NO_ERR);
    struct portal_wifi wifi={"Portal Test SSID","test-only-password",1};
    struct fdb_blob blob;
    assert(fdb_kv_set_blob(&db,"portal_wifi",fdb_blob_make(&blob,&wifi,sizeof(wifi)))==FDB_NO_ERR);
    /* FlashDB's successful set is synchronous; no mock sync API is invented. */
    assert(fdb_kvdb_deinit(&db)==FDB_NO_ERR);
    p.initial_format_approved=0; p.existing_flashdb_attested=1;
    assert(!amb1_preferences_flash_bind(&bridge,&ops,&p));
    assert(init(&db)==FDB_NO_ERR); check(&db,&wifi);
    before=erases;
    for (unsigned i=2;i<180;i++) {
        wifi.sequence=i;
        assert(fdb_kv_set_blob(&db,"portal_wifi",fdb_blob_make(&blob,&wifi,sizeof(wifi)))==FDB_NO_ERR);
    }
    assert(erases>before); /* Actual KVDB garbage collection, both sectors. */
    assert(fdb_kvdb_deinit(&db)==FDB_NO_ERR);
    assert(init(&db)==FDB_NO_ERR); check(&db,&wifi);
    /* Fail a real GC erase; after reboot the last committed record survives. */
    fail_erase_at=erases+1;
    unsigned failures=0;
    for (unsigned i=180;i<400;i++) {
        struct portal_wifi next=wifi; next.sequence=i;
        fdb_err_t result=fdb_kv_set_blob(&db,"portal_wifi",fdb_blob_make(&blob,&next,sizeof(next)));
        if (result!=FDB_NO_ERR) { failures++; break; }
        wifi=next;
    }
    assert(failures && erases>=fail_erase_at); fail_erase_at=0;
    assert(fdb_kvdb_deinit(&db)==FDB_NO_ERR);
    assert(init(&db)==FDB_NO_ERR); check(&db,&wifi);
    assert(fdb_kv_del(&db,"portal_wifi")==FDB_NO_ERR);
    assert(fdb_kvdb_deinit(&db)==FDB_NO_ERR);
    assert(init(&db)==FDB_NO_ERR);
    unsigned char got[200];
    assert(!fdb_kv_get_blob(&db,"portal_wifi",fdb_blob_make(&blob,got,sizeof(got))));
    assert(fdb_kvdb_deinit(&db)==FDB_NO_ERR);
    adjacent();
}
int main(void) {
    memset(memory,0xa5,sizeof(memory)); memcpy(baseline,memory,sizeof(memory));
    transport_tests(); database_tests(); puts("real FlashDB persistence and transport tests passed");
    return 0;
}
'''


class PreferencesFlashTest(unittest.TestCase):
    def test_real_flashdb_and_bounded_transport(self):
        cc = shutil.which("cc")
        if not cc:
            self.skipTest("host C compiler unavailable")
        if not (FLASHDB / "src/fdb_kvdb.c").is_file():
            self.skipTest("real FlashDB unavailable; set FLASHDB_SOURCE")
        with tempfile.TemporaryDirectory(prefix="amb1-prefs-") as temp:
            temp = Path(temp)
            (temp / "fdb_cfg.h").write_text(
                "#define FDB_USING_KVDB\n#define FDB_USING_FAL_MODE\n"
                "#define FDB_WRITE_GRAN 1\n#define FDB_PRINT(...) ((void)0)\n")
            (temp / "fal_cfg.h").write_text(
                '#include <fal_def.h>\nextern const struct fal_flash_dev preferences_test_dev;\n'
                '#define FAL_FLASH_DEV_TABLE { &preferences_test_dev }\n'
                '#define FAL_PART_HAS_TABLE_CFG\n'
                '#define FAL_PART_TABLE { {FAL_PART_MAGIC_WORD,"preferences","prefs",0,8192,0} }\n')
            source = temp / "harness.c"
            source.write_text(HARNESS)
            binary = temp / "preferences-test"
            includes = [temp, PORT, FLASHDB / "inc", FLASHDB / "port/fal/inc"]
            sources = [source, PORT / "preferences_flash.c"]
            sources += [FLASHDB / "src" / name for name in
                        ("fdb.c", "fdb_kvdb.c", "fdb_utils.c")]
            sources += [FLASHDB / "port/fal/src" / name for name in
                        ("fal.c", "fal_flash.c", "fal_partition.c")]
            command = [cc, "-std=c99", "-g", "-fsanitize=address,undefined"]
            command += [arg for path in includes for arg in ("-I", str(path))]
            command += [str(path) for path in sources] + ["-o", str(binary)]
            compiled = subprocess.run(command, capture_output=True, text=True)
            self.assertEqual(compiled.returncode, 0, compiled.stdout + compiled.stderr)
            result = subprocess.run([str(binary)], capture_output=True, text=True, timeout=30)
            self.assertEqual(result.returncode, 0, result.stdout + result.stderr)
            self.assertIn("real FlashDB persistence", result.stdout)


if __name__ == "__main__":
    unittest.main()
