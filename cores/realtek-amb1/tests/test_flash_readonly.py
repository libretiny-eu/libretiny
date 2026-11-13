"""Host-only contract tests; no SDK, hardware, credentials or flash access."""
import pathlib
import subprocess
import tempfile
import unittest


CORE = pathlib.Path(__file__).resolve().parents[1]


class FlashReadOnlyTest(unittest.TestCase):
    def test_adapter_and_sdk_guards(self):
        with tempfile.TemporaryDirectory() as directory:
            path = pathlib.Path(directory)
            (path / "libretiny.h").write_text("""
#pragma once
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#define FLASH_LENGTH 0x800000
#define LT_W(...) ((void)printf(__VA_ARGS__))
#define LT_E(...) ((void)printf(__VA_ARGS__))
""")
            (path / "sdk_private.h").write_text("""
#pragma once
typedef struct { int unused; } flash_t;
int flash_stream_read(flash_t *, uint32_t, uint32_t, uint8_t *);
""")
            (path / "device_lock.h").write_text("""
#define RT_DEV_LOCK_FLASH 0
static void device_mutex_lock(int device) { (void)device; }
static void device_mutex_unlock(int device) { (void)device; }
""")
            (path / "fal.h").write_text("""
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
    unsigned write_gran;
};
""")
            (path / "test.c").write_text(f"""
#include <assert.h>
#include "{CORE / 'base/port/fal_flash_amb1_port.c'}"
#include "{CORE / 'base/wraps/flash_guard.c'}"
/* Host stubs for the bring-up diagnostics the adapter references (they are
 * provided by base/fixups/boot_trace.c in the real image; this TU is host-only). */
void lt_amb1_trace(const char *tag) {{ (void)tag; }}
void lt_amb1_heartbeat_init(void) {{}}
static int reads, result = 1;
int flash_stream_read(flash_t *obj, uint32_t offset, uint32_t size, uint8_t *buf) {{
    assert(obj == &lt_flash_obj);
    assert(offset == 0xb001 || offset == FLASH_LENGTH - 1);
    reads++;
    for (uint32_t i = 0; i < size; i++) buf[i] = (offset + i) & 255;
    return result;
}}
int main(void) {{
    uint8_t buf[4];
    assert(flash0.ops.init() == 0 && reads == 0);
    assert(flash0.ops.read(0xb001, buf, 4) == 4);
    assert(buf[0] == 1 && buf[3] == 4);
    assert(flash0.ops.read(FLASH_LENGTH - 1, buf, 1) == 1);
    assert(flash0.ops.read(FLASH_LENGTH, NULL, 0) == 0);
    assert(flash0.ops.read(-1, buf, 1) == -1);
    assert(flash0.ops.read(FLASH_LENGTH - 1, buf, 2) == -1);
    assert(flash0.ops.read(0, buf, (size_t)-1) == -1);
    assert(flash0.ops.read(0, NULL, 1) == -1);
    assert(reads == 2);
    result = 0;
    assert(flash0.ops.read(0xb001, buf, 4) == -1);
    result = -1;
    assert(flash0.ops.read(0xb001, buf, 4) == -1);
    assert(flash0.ops.write(0xb000, buf, 4) == -1);
    assert(flash0.ops.write(0, NULL, 0) == -1);
    assert(flash0.ops.erase(0x2e3000, 4096) == -1);
    assert(flash0.ops.erase(0, 0) == -1);
    assert(__wrap_flash_write_word(NULL, 0, 0) == 0);
    assert(__wrap_flash_stream_write(NULL, 0, 4, buf) == 0);
    assert(__wrap_flash_burst_write(NULL, 0, 4, buf) == 0);
    assert(__wrap_flash_set_status(NULL, 0) == 0);
    assert(__wrap_flash_set_extend_addr(NULL, 0) == 0);
    __wrap_flash_erase_sector(NULL, 0);
    __wrap_flash_erase_block(NULL, 0);
    __wrap_flash_erase_chip(NULL);
    __wrap_flash_write_protect(NULL, 0);
    __wrap_flash_reset_status(NULL);
    return 0;
}}
""")
            executable = path / "test"
            subprocess.run([
                "cc", "-std=c99", "-Wall", "-Wextra", "-Werror",
                "-I", str(path), str(path / "test.c"), "-o", str(executable),
            ], check=True)
            result = subprocess.run([str(executable)], check=True, capture_output=True, text=True)
            self.assertIn("read-only bench build", result.stdout)
            self.assertIn("denied erase", result.stdout)
            self.assertIn("denied SDK flash_stream_write", result.stdout)


if __name__ == "__main__":
    unittest.main()
