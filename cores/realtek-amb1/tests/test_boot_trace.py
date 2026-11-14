"""Host-check optional boot diagnostics; no firmware build or hardware needed."""

import pathlib
import shutil
import subprocess
import tempfile
import unittest

SOURCE = pathlib.Path(__file__).resolve().parents[1] / "base/fixups/boot_trace.c"


class BootTraceTests(unittest.TestCase):
    def test_diagnostic_guards(self):
        source = SOURCE.read_text()
        self.assertNotIn("frame_in_stack", source)
        self.assertNotIn("+%02x", source)
        self.assertIn("+%08x", source)
        self.assertIn("later WLAN tasks may be absent", source)
        self.assertIn("extern u32 g_tcm_heap[] __attribute__((weak))", source)
        fault = source.split("static void tr_dump_task(u32 frame)")[1]
        self.assertNotIn("uxTaskGetStackHighWaterMark", fault)

    def test_optional_diagnostic_helpers(self):
        compiler = shutil.which("cc")
        if compiler is None:
            self.skipTest("host C compiler unavailable")
        source = SOURCE.read_text()
        helpers = source.split("/* BEGIN host-tested TCM diagnostics")[1]
        helpers = helpers.split("*/", 1)[1].split(
            "/* END host-tested TCM diagnostics."
        )[0]
        harness = r"""
#include <assert.h>
#include <stdint.h>
#include <stddef.h>
#include <stdarg.h>
#include <stdio.h>
#include <string.h>
typedef uint32_t u32;
static u32 memory[0x10000 / 4];
static unsigned reads, lines;
static int tr_rd(u32 *p, u32 *out) {
    uintptr_t a = (uintptr_t)p;
    /* Reject invalid accesses here rather than letting a fake reader hide bugs. */
    assert(!(a & 3) && a >= 0x1fff0000 && a <= 0x1ffffffc);
    reads++;
    *out = memory[(a - 0x1fff0000) / 4];
    return 1;
}
static void DiagPrintf(const char *fmt, ...) {
    (void)fmt;
    lines++;
}
"""
        cases = r"""
static void chunk(u32 a, u32 next, u32 size) {
    memory[(a - TR_TCM_START) / 4] = next;
    memory[(a - TR_TCM_START) / 4 + 1] = size;
}
int main(void) {
    const u32 lo = TR_TCM_START, hi = TR_TCM_END;
    u32 base = 0, size = 0;
    int hit;
    assert(tr_tcm_range(lo, hi - lo));
    assert(tr_tcm_range(hi - 4, 4));
    assert(!tr_tcm_range(hi - 4, 8));
    assert(!tr_tcm_range(lo, 0));
    assert(!tr_tcm_range(lo + 1, 4));
    assert(!tr_tcm_range(lo - 4, 8));
    assert(!tr_tcm_range(hi, 4));
    assert(!tr_tcm_range(lo, UINT32_MAX));
    assert(tr_ranges_intersect(lo, 16, lo + 8, 16));
    assert(tr_ranges_intersect(lo + 8, 16, lo, 16));
    assert(!tr_ranges_intersect(lo, 8, lo + 8, 8));
    assert(!tr_ranges_intersect(lo, 0, lo, 8));
    assert(!tr_ranges_intersect(0xfffffff0u, 8, 0, 8));

    reads = 0;
    assert(!tr_tcm_allocation(lo, &base, &size));
    assert(!tr_tcm_allocation(hi + 4, &base, &size));
    assert(!tr_tcm_allocation(lo + 5, &base, &size));
    assert(reads == 0);
    memory[0] = 13; /* requested payload 9, header 4; rounded allocation 16 */
    assert(tr_tcm_allocation(lo + 4, &base, &size));
    assert(base == lo && size == 16);
    memory[0] = 4;
    assert(!tr_tcm_allocation(lo + 4, &base, &size));
    memory[0] = UINT32_MAX;
    assert(!tr_tcm_allocation(lo + 4, &base, &size));
    memory[(hi - lo) / 4 - 2] = 9; /* roundup would cross TCM end */
    assert(!tr_tcm_allocation(hi - 4, &base, &size));
    memory[(hi - lo) / 4 - 2] = 8;
    assert(tr_tcm_allocation(hi - 4, &base, &size));
    assert(base == hi - 8 && size == 8);

    assert(tr_tcm_walk(0, 0, 0, &hit) && !hit);
    chunk(lo, lo + 32, 16);
    chunk(lo + 32, 0, 16);
    assert(tr_tcm_walk(lo, lo + 8, 16, &hit) && hit);
    assert(tr_tcm_walk(lo, lo + 16, 16, &hit) && !hit);
    assert(tr_tcm_walk(lo, 0, 0, &hit) && !hit);
    reads = 0;
    assert(!tr_tcm_walk(lo - 4, 0, 0, &hit));
    assert(!tr_tcm_walk(hi - 4, 0, 0, &hit));
    assert(!tr_tcm_walk(lo + 1, 0, 0, &hit));
    assert(reads == 0);
    chunk(lo, lo, 8); /* self-cycle */
    assert(!tr_tcm_walk(lo, lo, 8, &hit) && hit);
    assert(reads == 2);
    chunk(lo, lo + 16, 8);
    chunk(lo + 16, lo, 8); /* backward cycle */
    assert(!tr_tcm_walk(lo, 0, 0, &hit));
    chunk(lo, lo + 8, 16); /* overlapping next node */
    assert(!tr_tcm_walk(lo, 0, 0, &hit));
    chunk(lo, 0x40000000, 8); /* unmapped next: never dereferenced */
    reads = 0;
    assert(!tr_tcm_walk(lo, 0, 0, &hit) && reads == 2);
    const u32 bad_sizes[] = {0, 4, 9, 0x10008, UINT32_MAX};
    for (unsigned i = 0; i < sizeof(bad_sizes) / sizeof(bad_sizes[0]); i++) {
        chunk(lo, 0, bad_sizes[i]);
        assert(!tr_tcm_walk(lo, lo, 8, &hit) && !hit);
    }
    for (unsigned i = 0; i < 33; i++)
        chunk(lo + i * 8, i == 32 ? 0 : lo + (i + 1) * 8, 8);
    reads = lines = 0;
    assert(!tr_tcm_walk(lo, lo + 32 * 8, 8, &hit) && !hit);
    assert(reads == 64 && lines == 33); /* max32, no read of node 33 */
    chunk(lo + 31 * 8, 0, 8);
    reads = lines = 0;
    assert(tr_tcm_walk(lo, lo + 31 * 8, 8, &hit) && hit);
    assert(reads == 64 && lines == 32);
    return 0;
}
"""
        with tempfile.TemporaryDirectory(prefix="amb1-boot-trace-") as tmp:
            cfile = pathlib.Path(tmp) / "test.c"
            binary = pathlib.Path(tmp) / "test"
            cfile.write_text(harness + helpers + cases)
            subprocess.run(
                [
                    compiler,
                    "-std=c99",
                    "-Wall",
                    "-Wextra",
                    "-Werror",
                    "-fsanitize=undefined",
                    str(cfile),
                    "-o",
                    str(binary),
                ],
                check=True,
                capture_output=True,
                text=True,
            )
            subprocess.run([str(binary)], check=True, capture_output=True, text=True)

    def test_compile_guards_and_production_noops(self):
        compiler = shutil.which("cc")
        if compiler is None:
            self.skipTest("host C compiler unavailable")
        stub = r"""
#include <stdint.h>
typedef uint32_t u32;
extern unsigned writes;
extern void record_write(u32 address, u32 offset, u32 value);
#define HAL_WRITE32(a, o, v) record_write(a, o, v)
#define HAL_READ32(a, o) 0u
#define configUSE_TIMERS 1
#define configUSE_TRACE_FACILITY 1
#define portUSING_MPU_WRAPPERS 0
"""
        harness = r"""
#include <assert.h>
unsigned writes;
void record_write(u32 address, u32 offset, u32 value) {
    assert(address == 0x40003000u && offset == 4u && value == 0u);
    writes++;
}
int main(void) {
    lt_amb1_trace_init();
    assert(writes == 1);
    lt_amb1_trace("test");
    lt_amb1_trace2("test", 42);
    lt_amb1_trace_tcm("test");
    lt_amb1_app_log_test("test");
    lt_amb1_heartbeat_init();
    vApplicationMallocFailedHook();
    lt_amb1_fault_dump((void *)(uintptr_t)1);
    lt_amb1_led_identify_step();
    assert(writes == 1);
    return 0;
}
"""
        led = SOURCE.with_name("led_identify.c")
        with tempfile.TemporaryDirectory(prefix="amb1-boot-guards-") as tmp:
            root = pathlib.Path(tmp)
            (root / "ameba_soc.h").write_text(stub)
            for header in ("FreeRTOS.h", "task.h", "timers.h"):
                (root / header).write_text("")
            for flag in (None, 0, 1):
                args = [] if flag is None else [f"-DLT_AMB1_BRINGUP_DIAGNOSTICS={flag}"]
                result = subprocess.run(
                    [compiler, "-E", "-P", "-I", tmp, *args, str(SOURCE)],
                    check=True,
                    capture_output=True,
                    text=True,
                ).stdout
                for forbidden in (
                    "gpio_init(",
                    "gpio_write(",
                    "PC_0",
                    "PC_2",
                    "lt_amb1_led_identify_step(",
                ):
                    self.assertNotIn(forbidden, result)
                if flag == 1:
                    for diagnostic in (
                        "xTimerCreate(",
                        "uxTaskGetSystemState(",
                        "tr_dump_task(",
                        "DiagPrintf(",
                    ):
                        self.assertIn(diagnostic, result)
                else:
                    for diagnostic in (
                        "xTimerCreate(",
                        "uxTaskGetSystemState(",
                        "tr_dump_task(",
                        "DiagPrintf(",
                        "lt_amb1_log_puts(",
                    ):
                        self.assertNotIn(diagnostic, result)
                    cfile = root / "test.c"
                    binary = root / "test"
                    cfile.write_text(result + led.read_text() + harness)
                    subprocess.run(
                        [
                            compiler,
                            "-std=c99",
                            "-Wall",
                            "-Wextra",
                            "-Werror",
                            str(cfile),
                            "-o",
                            str(binary),
                        ],
                        check=True,
                        capture_output=True,
                        text=True,
                    )
                    subprocess.run(
                        [str(binary)], check=True, capture_output=True, text=True
                    )


if __name__ == "__main__":
    unittest.main()
