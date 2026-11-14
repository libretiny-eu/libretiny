#!/usr/bin/env python3
"""Static ordering regression test for the amb1 startup path.

Candidate005 created the diagnostic heartbeat timer (xTimerCreate -> pvPortMalloc)
from _AppStart, i.e. BEFORE the _AppStart BSS clears. The FreeRTOS heap
(ucHeap / xHeapRegions / its bookkeeping) lives in .sdr_bss and is garbage at
that point, so the boot hard-faulted inside the allocator:

    PC   3001cbec  pvPortMalloc   heap_5.c:276
    LR   3001cbaf  heap_5.c:245   (called from)
    ...  3001df41  xTimerCreate   timers.c:287
    ...  300112dd  lt_amb1_heartbeat_init  boot_trace.c

This test encodes the rule that broke: nothing on the _AppStart path may call a
timer or heap API, and the timer/LED work must sit in hooks that provably run
after the BSS clears. It is a source-order check (no toolchain, no hardware).

Run: python3 cores/realtek-amb1/tests/test_startup_ordering.py
"""

import os
import re
import unittest

CORE = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))
APP_START = os.path.join(CORE, "base", "fixups", "app_start_patch.c")
BOOT_TRACE = os.path.join(CORE, "base", "fixups", "boot_trace.c")
FAL_PORT = os.path.join(CORE, "base", "port", "fal_flash_amb1_port.c")
PRINTF_C = os.path.join(CORE, "base", "port", "printf.c")
LWIP_NETCONF = os.path.join(CORE, "base", "port", "lwip_netconf.c")
FREERTOS_CONFIG = os.path.join(CORE, "base", "config", "FreeRTOSConfig.h")

# Everything that needs a working FreeRTOS heap or the BSS to be zeroed.
FORBIDDEN_PRE_BSS = [
    "xTimerCreate",
    "xTimerStart",
    "xTimerGenericCommand",
    "xTimerStop",
    "xSemaphoreCreateBinary",
    "pvPortMalloc",
    "vPortFree",
    "malloc(",
    "calloc(",
    "realloc(",
    "tcm_heap",
    "lt_amb1_heartbeat_init",
    "lt_amb1_led_init",
]

# Calls that are safe before the BSS clears: register writes / ROM console only.
ALLOWED_EARLY = [
    "lt_amb1_trace_init",  # log UART IER/LSR writes
    "lt_amb1_trace",  # ROM DiagPrintf
    "lt_amb1_trace2",
    "lt_amb1_trace_tcm",  # read-only TCM probes
    "lt_amb1_app_log_test",  # log UART register writes
    "_memset(",  # the BSS clears themselves
    "InterruptForOSInit",
    "lt_main",
]


def read(path):
    with open(path, "r", encoding="utf-8") as fh:
        return fh.read()


def strip_comments(text):
    """Remove /* ... */, // ... and "..." so the checks only see code."""
    text = re.sub(r"/\*.*?\*/", "", text, flags=re.S)
    text = re.sub(r"//[^\n]*", "", text)
    text = re.sub(r'"(?:\\.|[^"\\])*"', '""', text)
    return text


def strip_comments_keep_strings(text):
    """Remove comments but keep string literals (for format-string checks)."""
    text = re.sub(r"/\*.*?\*/", "", text, flags=re.S)
    text = re.sub(r"//[^\n]*", "", text)
    return text


def body_of(text, signature):
    """Return the text of the first *definition* whose signature matches.

    A forward declaration (`... ;`) of the same signature must not win over the
    definition, so the first `{`-opened match is used.
    """
    start = 0
    while True:
        start = text.find(signature, start)
        if start < 0:
            return None
        brace = text.find("{", start)
        semi = text.find(";", start)
        if brace >= 0 and (semi < 0 or brace < semi):
            depth = 0
            for j in range(brace, len(text)):
                if text[j] == "{":
                    depth += 1
                elif text[j] == "}":
                    depth -= 1
                    if depth == 0:
                        return text[brace : j + 1]
            return None
        start += len(signature)


class StartupOrderingTest(unittest.TestCase):
    # ---------------------------------------------------------------- pre-BSS
    def test_app_start_path_has_no_timer_or_heap_calls(self):
        code = strip_comments(read(APP_START))
        body = body_of(code, "void lt_amb1_app_start(void)")
        self.assertIsNotNone(body, "lt_amb1_app_start not found")
        for token in FORBIDDEN_PRE_BSS:
            self.assertNotIn(
                token,
                body,
                f"{token} must not be called before the BSS clears (_AppStart path)",
            )

    def test_bss_clears_come_before_lt_main_in_app_start(self):
        code = strip_comments(read(APP_START))
        body = body_of(code, "void lt_amb1_app_start(void)")
        sdram = body.index("__sdram_bss_start__")
        tcm = body.index("__tcm_bss_start__")
        main = body.index("lt_main()")
        self.assertLess(sdram, main, "SDRAM BSS clear must precede lt_main()")
        self.assertLess(tcm, main, "TCM BSS clear must precede lt_main()")

    def test_naked_app_start_entry_stays_minimal(self):
        # Raw text for the inline-asm literal (strip_comments blanks strings out).
        raw = read(APP_START)
        self.assertIn("b lt_amb1_app_start", raw)
        # No call to the timer/LED entry points in actual code (comments stripped).
        code = strip_comments(raw)
        self.assertNotIn("lt_amb1_heartbeat_init", code)
        self.assertNotIn("lt_amb1_led_init", code)

    # --------------------------------------------- where the timer/LED may live
    def test_heartbeat_timer_created_from_the_fal_hook(self):
        fal = strip_comments(read(FAL_PORT))
        body = body_of(fal, "static int init(void)")
        self.assertIsNotNone(body, "flash0 init() not found")
        self.assertIn(
            "lt_amb1_heartbeat_init()",
            body,
            "the heartbeat timer must be created from the FAL flash0 init hook "
            "(after the _AppStart BSS clears and after __libc_init_array)",
        )

    def test_timer_api_only_inside_the_safe_hooks(self):
        code = strip_comments(read(BOOT_TRACE))
        # The only functions allowed to create FreeRTOS timers. Both are reached
        # solely from the FAL flash0 init hook (post-BSS, post-__libc_init_array):
        # the heartbeat, and the one-shot stack map started from it.
        bodies = []
        for sig in (
            "void lt_amb1_heartbeat_init(void)",
            "static void lt_amb1_task_map_start(void)",
        ):
            body = body_of(code, sig)
            self.assertIsNotNone(body, f"{sig} not found")
            bodies.append(body)
        self.assertIn("xTimerCreate", bodies[0])
        self.assertIn("xTimerStart", bodies[0])
        self.assertIn("xTimerCreate", bodies[1])
        # ... and nowhere else in the file (outside those definitions).
        rest = code
        for body in bodies:
            rest = rest.replace(body, "")
        for token in (
            "xTimerCreate",
            "xTimerStart",
            "xTimerGenericCommand",
            "pvPortMalloc",
            "tcm_heap_malloc",
            "tcm_heap_calloc",
            "tcm_heap_freemem",
        ):
            self.assertNotIn(
                token, rest, f"{token} must appear only inside the safe timer hooks"
            )

    def test_heartbeat_no_longer_drives_led_tests(self):
        code = strip_comments(read(BOOT_TRACE))
        cb = body_of(code, "static void hb_cb(TimerHandle_t t)")
        self.assertIsNotNone(cb, "hb_cb not found")
        self.assertNotIn("led_step", cb)
        self.assertNotIn("lt_amb1_led_identify_step", cb)
        self.assertNotIn("gpio_", cb)
        self.assertIn("xTimerStop", cb, "optional log heartbeat remains bounded")

    def test_diagnostics_default_off_and_uart_mask_is_unconditional(self):
        code = strip_comments(read(BOOT_TRACE))
        self.assertRegex(
            code,
            r"#ifndef LT_AMB1_BRINGUP_DIAGNOSTICS\s+"
            r"#define LT_AMB1_BRINGUP_DIAGNOSTICS 0",
        )
        init = body_of(code, "void lt_amb1_trace_init(void)")
        self.assertIn("HAL_WRITE32(0x40003000u, 0x04u, 0)", init)
        # test_boot_trace.py also preprocesses both modes and host-executes the
        # normal-mode hooks to verify the mask survives without diagnostics.
        self.assertLess(
            code.index("void lt_amb1_trace_init(void)"),
            code.index("#include <FreeRTOS.h>"),
        )

    # --------------------------------------------------- the proven UART fix
    def test_direct_log_uart_backend_is_kept(self):
        code = strip_comments(read(PRINTF_C))
        self.assertIn(
            "lt_amb1_log_putc", code, "the direct polled log-UART writer must stay"
        )
        self.assertIn("0x40003000", code, "log UART register block")
        self.assertNotIn(
            "HalSerialPutcRtl8195a",
            code,
            "the app print path must not go back to the gated SDK console backend",
        )

    def test_no_tcm_heap_reservation_came_back(self):
        # Both the failed reservation experiment and the speculative
        # tcm_heap_free()/vPortSetExtFree() alias guard are forbidden: neither is
        # evidence-based, and both change allocator behaviour.
        banned = (
            "tcm_heap_malloc",
            "tcm_heap_freemem",
            "tcm_heap_free",
            "vPortSetExtFree",
            "lt_amb1_tcm_free",
        )
        for path in (BOOT_TRACE, APP_START, FAL_PORT, LWIP_NETCONF, PRINTF_C):
            code = strip_comments(read(path))
            for token in banned:
                self.assertNotIn(token, code, f"{token} must not return ({path})")

    # -------------------------------------------------- 007 fault introspection
    def test_fault_dump_identifies_task_via_freertos_api(self):
        """The fault handler must name the running task with public FreeRTOS
        APIs, never by dereferencing pxCurrentTCB through a hand-written TCB
        offset (the layout is FreeRTOS's business, not ours)."""
        code = strip_comments(read(BOOT_TRACE))
        dump = body_of(code, "void lt_amb1_fault_dump(void *frame_)")
        self.assertIsNotNone(dump, "lt_amb1_fault_dump not found")
        for call in (
            "xTaskGetCurrentTaskHandle",
            "pcTaskGetTaskName",
        ):
            self.assertIn(call, code, f"fault diagnostics must use {call}")
        self.assertIn(
            "tr_tcb_layout_ok",
            code,
            "fault-time stack fields require a validated TCB layout",
        )
        # No raw TCB access: pxCurrentTCB may not be dereferenced directly.
        self.assertNotIn("pxCurrentTCB", code, "do not dereference pxCurrentTCB raw")

    def test_tcb_mirror_is_source_backed_and_runtime_validated(self):
        """v8.2.3 has no public stack-base API, so the stack fields come from a
        mirror of the exact tasks.c TCB_t prefix - gated on a runtime check
        against the kernel's own pcTaskGetTaskName() offset."""
        code = strip_comments(read(BOOT_TRACE))
        self.assertIn("tr_tcb_head_t", code)
        for field in (
            "pxTopOfStack",
            "xGenericListItem",
            "xEventListItem",
            "uxPriority",
            "pxStack",
            "pcTaskName",
        ):
            self.assertIn(field, code, f"TCB mirror must mirror tasks.c field {field}")
        self.assertIn("offsetof(tr_tcb_head_t, pcTaskName)", code)
        check = body_of(code, "static int tr_tcb_layout_ok(TaskHandle_t h)")
        self.assertIsNotNone(check, "tr_tcb_layout_ok not found")
        self.assertIn(
            "pcTaskGetTaskName", check, "the mirror must be checked against the kernel"
        )
        dump = body_of(code, "static void tr_dump_task(u32 frame)")
        self.assertIsNotNone(dump, "tr_dump_task not found")
        self.assertIn(
            "tr_tcb_layout_ok",
            dump,
            "the stack-field read must be gated on the layout check",
        )

    def test_fault_dump_bounds_checks_the_tcb_pointer(self):
        code = strip_comments(read(BOOT_TRACE))
        helper = body_of(code, "static void tr_dump_task(u32 frame)")
        self.assertIsNotNone(helper, "tr_dump_task not found")
        self.assertIn(
            "tr_readable", helper, "the TCB pointer must be range-checked first"
        )

    def test_rom_printf_format_strings_have_no_percent_u(self):
        # The ROM DiagPrintf does not consume an argument for "%u" (it prints a
        # literal 'u'), which shifts every later vararg. Use %d or %08x.
        for path in (BOOT_TRACE, APP_START, FAL_PORT, PRINTF_C, LWIP_NETCONF):
            code = strip_comments_keep_strings(read(path))
            self.assertNotIn("%u", code, f"ROM DiagPrintf does not support %u ({path})")

    def test_freertos_config_keeps_minimal_instrumentation(self):
        cfg = strip_comments(read(FREERTOS_CONFIG))
        self.assertRegex(cfg, r"#define\s+configCHECK_FOR_STACK_OVERFLOW\s+2")
        self.assertRegex(cfg, r"#define\s+INCLUDE_uxTaskGetStackHighWaterMark\s+1")
        self.assertRegex(cfg, r"#define\s+configUSE_TRACE_FACILITY\s+1")
        # configASSERT stays off: enabling it also turns on
        # vPortValidateInterruptPriority(), which the vendor never exercised.
        self.assertNotRegex(cfg, r"#define\s+configASSERT\s*\(")

    def test_task_map_uses_public_api_and_mirror(self):
        code = strip_comments(read(BOOT_TRACE))
        fn = body_of(code, "static void tr_dump_task_map(TimerHandle_t t)")
        self.assertIsNotNone(fn, "tr_dump_task_map not found")
        self.assertIn(
            "uxTaskGetSystemState",
            fn,
            "the map must enumerate tasks via the public API",
        )
        self.assertIn(
            "tr_tcb_layout_ok",
            fn,
            "each TCB must pass the layout check before reading pxStack",
        )
        self.assertIn("pxStack", fn)


if __name__ == "__main__":
    unittest.main(verbosity=2)
