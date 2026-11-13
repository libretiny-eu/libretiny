"""Host-compile the real FAL adapter with mocked SDK dependencies."""
import pathlib
import shutil
import subprocess
import tempfile
import unittest

ROOT = pathlib.Path(__file__).resolve().parents[3]
PORT = ROOT / "cores/realtek-amb1/base/port"
HEADERS = {
    "libretiny.h": """
#include <stdint.h>
#include <stddef.h>
#include <stdio.h>
#define FLASH_LENGTH 0x800000UL
#define LT_W(...) ((void)snprintf(NULL, 0, __VA_ARGS__))
#define LT_E(...) ((void)snprintf(NULL, 0, __VA_ARGS__))
""",
    "sdk_private.h": """
typedef struct { int unused; } flash_t;
int flash_stream_read(flash_t *, unsigned long, size_t, uint8_t *);
""",
    "device_lock.h": """
#define RT_DEV_LOCK_FLASH 7
void device_mutex_lock(int);
void device_mutex_unlock(int);
""",
    "fal.h": """
#define FAL_FLASH_DEV_NAME "flash0"
struct fal_flash_dev {
    const char *name;
    unsigned long addr, len, blk_size;
    struct {
        int (*init)(void);
        int (*read)(long, uint8_t *, size_t);
        int (*write)(long, const uint8_t *, size_t);
        int (*erase)(long, size_t);
    } ops;
    int write_gran;
};
""",
}
MAIN = r"""
#include <assert.h>
#include <limits.h>
#include <string.h>
#include <libretiny.h>
#include <sdk_private.h>
#include <device_lock.h>
#include <fal.h>
#include "preferences_sdk.h"
extern const struct fal_flash_dev flash0;
void lt_amb1_preferences_configure(void);
static int locked, locks, unlocks, reads, calls, bound, fail;
static int traces, heartbeats, configured;
static long expected_offset;
static size_t expected_size;
static const void *expected_buffer;
void lt_amb1_trace(const char *tag) {
    assert(!strcmp(tag, "lt_main -> fal_init -> flash0 init")); traces++;
}
void lt_amb1_heartbeat_init(void) { heartbeats++; }
void test_configure(void) {
    assert(traces == 1 && heartbeats == 1);
    configured++; bound = 1;
}
void device_mutex_lock(int id) {
    assert(id == RT_DEV_LOCK_FLASH && !locked); locked = 1; locks++;
}
void device_mutex_unlock(int id) {
    assert(id == RT_DEV_LOCK_FLASH && locked); locked = 0; unlocks++;
}
static void check(long offset, const void *buf, size_t size) {
    assert(offset == expected_offset && size == expected_size && buf == expected_buffer);
}
int flash_stream_read(flash_t *object, unsigned long offset, size_t size, uint8_t *buf) {
    assert(object && locked); check(offset,buf,size); reads++; return fail ? 0 : 1;
}
#if LT_AMB1_EXPERIMENTAL_KVS
static int delegate(long offset, const void *buf, size_t size) {
    assert(!locked); /* The bridge, not FAL, must acquire the shared lock. */
    check(offset,buf,size); calls++;
    if (!bound || fail) return -1;
    device_mutex_lock(RT_DEV_LOCK_FLASH); device_mutex_unlock(RT_DEV_LOCK_FLASH);
    return (int)size;
}
int amb1_preferences_sdk_read(long o, uint8_t *b, size_t n) { return delegate(o,b,n); }
int amb1_preferences_sdk_write(long o, const uint8_t *b, size_t n) { return delegate(o,b,n); }
int amb1_preferences_sdk_erase(long o, size_t n) { return delegate(o,0,n); }
#endif
static void expect(long offset, const void *buf, size_t size) {
    expected_offset = offset; expected_buffer = buf; expected_size = size;
}
static void normal(long offset, uint8_t *buf, size_t size) {
    int r = reads, c = calls; expect(offset,buf,size);
    assert(flash0.ops.read(offset,buf,size) == (int)size);
    assert(reads == r+1 && calls == c && !locked);
}
int main(void) {
    uint8_t buf[8] = {0};
    const long start = AMB1_PREF_KVS_START, end = AMB1_PREF_KVS_END;
    assert(!flash0.ops.init());
    assert(traces == 1 && heartbeats == 1);
    assert(!configured && !bound); /* FAL init must never bind, even with a strong hook. */
    assert(!calls && !reads && !locks);
    normal(0,buf,1);
    normal(start-1,buf,2); /* Spanning reads retain the normal locked path. */
    normal(end-1,buf,2);
    normal(end,buf,1);
    normal(FLASH_LENGTH-1,buf,1);
    fail = 1; expect(0,buf,1); assert(flash0.ops.read(0,buf,1) == -1); fail = 0;
    int r = reads, c = calls;
    assert(flash0.ops.read(-1,buf,1) == -1);
    assert(flash0.ops.read(LONG_MAX,buf,1) == -1);
    assert(flash0.ops.read(0,buf,(size_t)-1) == -1);
    assert(flash0.ops.read(FLASH_LENGTH,buf,1) == -1);
    assert(flash0.ops.read(FLASH_LENGTH-1,buf,2) == -1);
    assert(flash0.ops.read(start,0,1) == -1);
    assert(!flash0.ops.read(start,buf,0));
    assert(!flash0.ops.read(FLASH_LENGTH,0,0));
    assert(reads == r && calls == c);
    for (int enabled=0; enabled<2; enabled++) {
        if (enabled) lt_amb1_preferences_configure(); /* Explicit late caller only. */
        assert(configured == (enabled && STRONG_HOOK));
        assert(bound == configured);
#if LT_AMB1_EXPERIMENTAL_KVS
        expect(start,buf,1);
        assert(flash0.ops.read(start,buf,1) == (bound ? 1 : -1));
        assert(reads == r); /* Unbound must never fall back to normal reads. */
        assert(flash0.ops.write(start,buf,1) == (bound ? 1 : -1));
        expect(start,0,8192);
        assert(flash0.ops.erase(start,8192) == (bound ? 8192 : -1));
        expect(end-1,buf,1);
        assert(flash0.ops.read(end-1,buf,1) == (bound ? 1 : -1));
        assert(flash0.ops.write(end-1,buf,1) == (bound ? 1 : -1));
        fail = 1;
        assert(flash0.ops.read(end-1,buf,1) == -1);
        assert(flash0.ops.write(end-1,buf,1) == -1);
        expect(end-4096,0,4096);
        assert(flash0.ops.erase(end-4096,4096) == -1);
        fail = 0;
#else
        normal(start,buf,1); r = reads;
        assert(flash0.ops.write(start,buf,1) == -1);
        assert(flash0.ops.erase(start,4096) == -1);
        assert(!calls);
#endif
    }
    c = calls;
    const long offsets[] = {-1, LONG_MAX, 0, start-1, start, end-1, end};
    const size_t sizes[] = {1, 1, 1, 2, (size_t)-1, 2, 1};
    for (unsigned i=0; i<sizeof(offsets)/sizeof(offsets[0]); i++) {
        assert(flash0.ops.write(offsets[i],buf,sizes[i]) == -1);
        assert(flash0.ops.erase(offsets[i],sizes[i]) == -1);
    }
    assert(flash0.ops.write(start,0,1) == -1);
    assert(flash0.ops.write(start,buf,0) == -1);
    assert(flash0.ops.erase(start,0) == -1);
    assert(calls == c && locks == unlocks && !locked);
    return 0;
}
"""


class KvsFalDispatchTest(unittest.TestCase):
    def test_actual_adapter(self):
        compiler = shutil.which("cc")
        if not compiler:
            self.skipTest("host C compiler unavailable")
        for flag in (None, 0, 1):
            for strong in (0, 1):
                with self.subTest(flag=flag, strong=strong), tempfile.TemporaryDirectory() as tmp:
                    temp = pathlib.Path(tmp)
                    for name, content in HEADERS.items():
                        (temp / name).write_text(content)
                    (temp / "main.c").write_text(MAIN)
                    (temp / "hook.c").write_text(
                        "extern void test_configure(void);\n"
                        "void lt_amb1_preferences_configure(void) { test_configure(); }\n"
                    )
                    command = [compiler, "-std=c99", "-Wall", "-Wextra", "-Werror",
                               f"-I{temp}", f"-I{PORT}", f"-DSTRONG_HOOK={strong}"]
                    if flag is not None:
                        command.append(f"-DLT_AMB1_EXPERIMENTAL_KVS={flag}")
                    command += [str(PORT / "fal_flash_amb1_port.c"), str(temp / "main.c")]
                    if strong:
                        command.append(str(temp / "hook.c"))
                    command += ["-o", str(temp / "test")]
                    result = subprocess.run(command, capture_output=True, text=True)
                    self.assertEqual(result.returncode, 0, result.stderr)
                    result = subprocess.run([str(temp / "test")], capture_output=True, text=True)
                    self.assertEqual(result.returncode, 0, result.stderr)


if __name__ == "__main__":
    unittest.main()
