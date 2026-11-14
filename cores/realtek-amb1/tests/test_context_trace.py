"""Compile the real context wrapper against host mocks; no hardware required."""

import pathlib
import subprocess
import tempfile
import unittest

ROOT = pathlib.Path(__file__).resolve().parents[1]


class ContextTraceTest(unittest.TestCase):
    def test_normal_wrapper_only_calls_scheduler(self):
        with tempfile.TemporaryDirectory(prefix="amb1-context-normal-") as directory:
            d = pathlib.Path(directory)
            source = '#include "' + str(ROOT / "base/fixups/context_trace.c") + '"\n'
            source += """
#include <assert.h>
static unsigned calls;
void __real_vTaskSwitchContext(void) { calls++; }
int main(void) {
    __wrap_vTaskSwitchContext();
    assert(calls == 1);
    return 0;
}
"""
            (d / "test.c").write_text(source)
            for flags in ([], ["-DLT_AMB1_BRINGUP_DIAGNOSTICS=0"]):
                subprocess.run(
                    [
                        "cc",
                        "-std=c11",
                        "-Wall",
                        "-Wextra",
                        "-Werror",
                        *flags,
                        str(d / "test.c"),
                        "-o",
                        str(d / "test"),
                    ],
                    check=True,
                )
                subprocess.run([str(d / "test")], check=True)

    def test_host_wrapper(self):
        with tempfile.TemporaryDirectory(prefix="amb1-context-") as directory:
            d = pathlib.Path(directory)
            (d / "ameba_soc.h").write_text("int DiagPrintf(const char *, ...);\n")
            (d / "FreeRTOS.h").write_text("""
#include <stdint.h>
#define portUSING_MPU_WRAPPERS 0
#define configMAX_TASK_NAME_LEN 16
typedef uint32_t StackType_t;
typedef unsigned UBaseType_t;
typedef struct { uintptr_t words[5]; } ListItem_t;
""")
            (d / "task.h").write_text("""
typedef void *TaskHandle_t;
TaskHandle_t xTaskGetCurrentTaskHandle(void);
char *pcTaskGetTaskName(TaskHandle_t);
""")
            source = r"""
#include <assert.h>
#include <stdarg.h>
#include <stdio.h>
#include <string.h>
#include <stdint.h>
/* Real linker-style bounds around a host arena, including its final byte. */
#ifdef __APPLE__
__asm__(".bss\n.p2align 4\n.globl ___sdram_data_start__\n"
        "___sdram_data_start__:\n.space 4096\n.globl ___sdram_bss_end__\n___sdram_bss_end__:\n.text");
#else
__asm__(".bss\n.p2align 4\n.globl __sdram_data_start__\n"
        "__sdram_data_start__:\n.space 4096\n.globl __sdram_bss_end__\n__sdram_bss_end__:\n.text");
#endif
#include "CONTEXT_SOURCE"

static TaskHandle_t current, incoming;
static unsigned calls, logs, name_calls;
static int bad_layout;
static char line[1024];
TaskHandle_t xTaskGetCurrentTaskHandle(void) { return current; }
char *pcTaskGetTaskName(TaskHandle_t h) {
    name_calls++;
    return bad_layout ? (char *)1 : ((ctx_tcb_t *)h)->pcTaskName;
}
void __real_vTaskSwitchContext(void) { calls++; current = incoming; }
int DiagPrintf(const char *fmt, ...) {
    va_list ap;
    va_start(ap, fmt);
    vsnprintf(line, sizeof(line), fmt, ap);
    va_end(ap);
    logs++;
    return 0;
}
static void switch_once(TaskHandle_t a, TaskHandle_t b) {
    unsigned before = calls;
    current = a; incoming = b;
    __wrap_vTaskSwitchContext();
    assert(calls == before + 1);
    assert(current == b);
}
int main(void) {
    ctx_tcb_t *a = (void *)__sdram_data_start__;
    ctx_tcb_t *b = (void *)(__sdram_data_start__ + 256);
    StackType_t *stack = (void *)(__sdram_data_start__ + 512);
    strcpy(a->pcTaskName, "idle");
    strcpy(b->pcTaskName, "rtw_recv_task");
    a->pxStack = b->pxStack = stack;
    a->pxTopOfStack = stack;
    b->pxTopOfStack = stack + 32;
    stack[6] = 0xaaaaaaaa; stack[7] = 0xbbbbbbbb;
    stack[14] = 0x12345678; stack[15] = 0x01000000;
    stack[46] = 0x30038775; stack[47] = 0x01000000;
    current = a;
    ctx_snapshot_t s = ctx_snapshot();
    assert(s.valid && s.pc == 0x12345678 && s.xpsr == 0x01000000);
    assert(ctx_recv("rtw_recv_") && ctx_recv("rtw_recv_task"));
    assert(!ctx_recv("") && !ctx_recv("rtw_recv") && !ctx_recv("xrtw_recv_task"));
    assert(!ctx_recv("rtw_recx_task"));
    switch_once(a, a); assert(logs == 0);
    assert(ctx_transitions == 0);
    for (int i = 0; i < 31; i++) switch_once(a, b);
    assert(logs == 0 && ctx_transitions == 31);
    switch_once(b, a); assert(logs == 1 && ctx_samples == 1);
    assert(strstr(line, "ctx seq=00000020 suspicious=0 rtw_recv_task top="));
    assert(strstr(line, "pc=12345678 xpsr=01000000"));
    for (int i = 0; i < 288; i++) switch_once(a, b);
    assert(logs == 10 && ctx_transitions == 320 && ctx_samples == 10);
    /* The candidate009 failure appeared after its initial log budget. */
    stack[46] = 0; stack[47] = 0x20000000;
    switch_once(a, b); assert(logs == 11);
    assert(strstr(line, "ctx seq=00000141 suspicious=1 idle top="));
    assert(strstr(line, " -> rtw_recv_task top="));
    assert(strstr(line, "pc=00000000 xpsr=20000000"));
    assert(strstr(line, "read=1"));
    switch_once(b, a); assert(logs == 12);
    assert(strstr(line, "seq=00000142 suspicious=1 rtw_recv_task top="));
    /* Each bad register independently qualifies, on either side. */
    stack[47] = 0x01000000;
    switch_once(a, b); assert(logs == 13);
    stack[46] = 0x30038775; stack[47] = 0x20000000;
    switch_once(b, a); assert(logs == 14);
    stack[47] = 0x01000000;
    /* Valid samples cannot consume or disable suspicious reporting. */
    while (ctx_transitions < 2048) switch_once(a, b);
    assert(ctx_samples == 32 && logs == 36);
    stack[46] = 0;
    for (int i = 0; i < 40; i++) {
        unsigned previous = logs;
        switch_once(a, b);
        assert(logs == previous + 1 && ctx_samples == 32);
    }
    stack[46] = 0x30038775;
    unsigned before = name_calls;
    current = NULL; s = ctx_snapshot(); assert(!s.valid);
    current = (void *)1; s = ctx_snapshot(); assert(!s.valid);
    current = (void *)(uintptr_t)-16; s = ctx_snapshot(); assert(!s.valid);
    current = (void *)(__sdram_data_start__ + 4096 - 8);
    s = ctx_snapshot(); assert(!s.valid && name_calls == before);
    current = b; bad_layout = 1;
    s = ctx_snapshot(); assert(!s.valid && !strcmp(s.name, "?"));
    bad_layout = 0;
    unsigned previous = logs;
    bad_layout = 1;
    switch_once(b, a); /* No recognizable receive name: not a receive transition. */
    assert(logs == previous);
    bad_layout = 0;
    current = b;
    b->pxTopOfStack = (void *)(__sdram_data_start__ + 4096 - 60);
    s = ctx_snapshot(); assert(!s.valid);
    b->pxTopOfStack = (void *)((uintptr_t)stack + 1);
    s = ctx_snapshot(); assert(!s.valid);
    b->pxTopOfStack = stack; b->pxStack = stack + 1;
    s = ctx_snapshot(); assert(!s.valid);
    b->pxStack = NULL; s = ctx_snapshot(); assert(!s.valid);
    b->pxStack = stack; b->pxTopOfStack = (void *)1;
    previous = logs;
    switch_once(a, b); assert(logs == previous + 1 && strstr(line, "read=0"));
    switch_once(b, a); assert(logs == previous + 2 && strstr(line, "suspicious=1"));
    b->pxTopOfStack = stack + 32;
    switch_once(NULL, b); assert(logs == previous + 3 && strstr(line, "? top="));
    switch_once(b, (void *)1); assert(logs == previous + 4 && strstr(line, " -> ? top="));
    switch_once((void *)1, b); assert(logs == previous + 5);
    switch_once(b, (void *)(__sdram_data_start__ + 4096 - 8));
    assert(logs == previous + 6);
    assert(ctx_readable(0x1ffffffc, 4));
    assert(!ctx_readable(0x1ffffffc, 8));
    assert(!ctx_readable(0x1003fffc, 8));
    assert(!ctx_readable(UINTPTR_MAX - 3, 16));
    assert(!ctx_readable(0, 4));
    assert(ctx_readable((uintptr_t)__sdram_data_start__ + 4092, 4));
    assert(!ctx_readable((uintptr_t)__sdram_data_start__ + 4092, 8));
    memset(b->pcTaskName, 'z', configMAX_TASK_NAME_LEN);
    current = b; s = ctx_snapshot();
    assert(s.name[configMAX_TASK_NAME_LEN] == 0);
    strcpy(b->pcTaskName, "rtw_recv_task");
    previous = logs;
    for (int i = 0; i < 128; i++) switch_once(a, b);
    assert(logs == previous && ctx_samples == 32);
    before = name_calls;
    switch_once((void *)1, (void *)1);
    assert(name_calls == before && logs == previous);
    puts("context trace host checks passed");
    return 0;
}
""".replace("CONTEXT_SOURCE", str(ROOT / "base/fixups/context_trace.c"))
            (d / "test.c").write_text(source)
            subprocess.run(
                [
                    "cc",
                    "-std=c11",
                    "-Wall",
                    "-Wextra",
                    "-Werror",
                    "-DLT_AMB1_BRINGUP_DIAGNOSTICS=1",
                    "-I",
                    str(d),
                    str(d / "test.c"),
                    "-o",
                    str(d / "test"),
                ],
                check=True,
            )
            subprocess.run([str(d / "test")], check=True)


if __name__ == "__main__":
    unittest.main()
