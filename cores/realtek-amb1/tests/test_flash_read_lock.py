"""Compile the normal read paths against a non-recursive SDK lock mock."""

import pathlib
import subprocess
import tempfile
import unittest

CORE = pathlib.Path(__file__).resolve().parents[1]


class FlashReadLockTest(unittest.TestCase):
    def build_and_run(self, id_return):
        with tempfile.TemporaryDirectory() as directory:
            path = pathlib.Path(directory)
            (path / "libretiny.h").write_text("""
#pragma once
#include <stddef.h>
#include <stdint.h>
#define FLASH_LENGTH 0x800000
#define LT_W(...) ((void)0)
#define LT_E(...) ((void)0)
typedef struct {
    uint8_t manufacturer_id, chip_id, chip_size_id;
} lt_flash_id_t;
""")
            (path / "sdk_private.h").write_text(f"""
#pragma once
typedef struct {{ int unused; }} flash_t;
int flash_stream_read(flash_t *, uint32_t, uint32_t, uint8_t *);
{id_return} flash_read_id(flash_t *, uint8_t *, uint8_t);
""")
            (path / "device_lock.h").write_text("""
#pragma once
typedef enum { RT_DEV_LOCK_FLASH = 2 } RT_DEV_LOCK_E;
void device_mutex_lock(RT_DEV_LOCK_E);
void device_mutex_unlock(RT_DEV_LOCK_E);
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
            id_result = "return id_bytes;" if id_return == "int" else "return;"
            (path / "test.c").write_text(f"""
#include <assert.h>
#include "{CORE / 'base/port/fal_flash_amb1_port.c'}"
#include "{CORE / 'base/api/lt_flash.c'}"
static int locked, locks, unlocks, reads, ids, result = 1, id_bytes = 3;
void lt_amb1_trace(const char *tag) {{ (void)tag; }}
void lt_amb1_heartbeat_init(void) {{}}
void device_mutex_lock(RT_DEV_LOCK_E device) {{
    assert(device == RT_DEV_LOCK_FLASH && !locked);
    locked = 1;
    locks++;
}}
void device_mutex_unlock(RT_DEV_LOCK_E device) {{
    assert(device == RT_DEV_LOCK_FLASH && locked);
    locked = 0;
    unlocks++;
}}
int flash_stream_read(flash_t *obj, uint32_t offset, uint32_t size, uint8_t *buf) {{
    assert(locked && obj == &lt_flash_obj);
    assert(offset == 1 || offset == FLASH_LENGTH - 1);
    reads++;
    for (uint32_t i = 0; i < size; i++) buf[i] = (uint8_t)(offset + i);
    return result;
}}
{id_return} flash_read_id(flash_t *obj, uint8_t *buf, uint8_t size) {{
    assert(locked && obj == NULL && size == 3);
    ids++;
    for (int i = 0; i < id_bytes; i++) buf[i] = (uint8_t)(0xef + i);
    {id_result}
}}
static void balanced(int expected) {{
    assert(!locked && locks == expected && unlocks == expected);
}}
int main(void) {{
    uint8_t buf[4];
    assert(flash0.ops.init() == 0);
    balanced(0);
    assert(flash0.ops.read(-1, buf, 1) == -1);
    assert(flash0.ops.read(FLASH_LENGTH + 1, buf, 0) == -1);
    assert(flash0.ops.read(FLASH_LENGTH - 1, buf, 2) == -1);
    assert(flash0.ops.read(0, buf, (size_t)-1) == -1);
    assert(flash0.ops.read(0, NULL, 1) == -1);
    assert(flash0.ops.read(FLASH_LENGTH, NULL, 0) == 0);
    assert(flash0.ops.read(0, buf, 0) == 0);
    balanced(0);
    assert(reads == 0);
    assert(flash0.ops.read(1, buf, sizeof(buf)) == 4);
    assert(buf[0] == 1 && buf[3] == 4);
    balanced(1);
    assert(flash0.ops.read(FLASH_LENGTH - 1, buf, 1) == 1);
    balanced(2);
    result = 0;
    assert(flash0.ops.read(1, buf, 4) == -1);
    balanced(3);
    result = -1;
    assert(flash0.ops.read(1, buf, 4) == -1);
    balanced(4);
    result = 4; /* SDK success is exactly 1, not a byte count. */
    assert(flash0.ops.read(1, buf, 4) == -1);
    balanced(5);
    assert(reads == 5);
    lt_flash_id_t id = lt_flash_get_id();
    assert(id.manufacturer_id == 0xef && id.chip_id == 0xf0 && id.chip_size_id == 0xf1);
    balanced(6);
    id_bytes = 1;
    id = lt_flash_get_id();
    assert(id.manufacturer_id == 0xef && id.chip_id == 0 && id.chip_size_id == 0);
    balanced(7);
    id_bytes = -1;
    id = lt_flash_get_id();
    assert(id.manufacturer_id == 0 && id.chip_id == 0 && id.chip_size_id == 0);
    balanced(8);
    assert(ids == 3);
    assert(flash0.ops.write(0, buf, 4) == -1);
    assert(flash0.ops.write(0, NULL, 0) == -1);
    assert(flash0.ops.erase(0, 4096) == -1);
    assert(flash0.ops.erase(0, 0) == -1);
    balanced(8);
    return 0;
}}
""")
            executable = path / "test"
            subprocess.run(
                [
                    "cc",
                    "-std=c99",
                    "-Wall",
                    "-Wextra",
                    "-Werror",
                    "-Wno-unused-parameter",
                    "-I",
                    str(path),
                    str(path / "test.c"),
                    "-o",
                    str(executable),
                ],
                check=True,
            )
            subprocess.run([str(executable)], check=True)

    def test_sdk_int_id(self):
        self.build_and_run("int")

    def test_sdk_void_id(self):
        self.build_and_run("void")


if __name__ == "__main__":
    unittest.main()
