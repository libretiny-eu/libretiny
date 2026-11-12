/* Copyright (c) Kuba Szczodrzyński 2023-03-02. */

#pragma once

#include_next "FreeRTOSConfig.h"

#undef configTOTAL_HEAP_SIZE
#define configTOTAL_HEAP_SIZE ((size_t)(160 * 1024))

/* SDRAM suspend/tickless hooks are not proven on the 2 MiB module. */
#undef configUSE_TICKLESS_IDLE
#define configUSE_TICKLESS_IDLE 0
#undef configTIMER_TASK_PRIORITY
#define configTIMER_TASK_PRIORITY (configMAX_PRIORITIES - 1)

/* Bring-up evidence: method 2 adds the kernel's stack-end fill-pattern check to
 * the SP-limit check. The port config already selects 2
 * (library-freertos-port/realtek-amb1/FreeRTOSConfig.h), and prvAllocateTCBAndStack
 * therefore fills every task stack with tskSTACK_FILL_BYTE; re-stating it here
 * keeps the choice explicit. A task that overflows its TCM stack is then reported
 * by the port's vApplicationStackOverflowHook with the task name instead of
 * turning into a silent INVSTATE/PC=0. */
#undef configCHECK_FOR_STACK_OVERFLOW
#define configCHECK_FOR_STACK_OVERFLOW 2

/* Bring-up evidence: expose FreeRTOS's public stack-usage query so the fault
 * handler can name the running task and report how much of its stack is left
 * using the kernel's own TCB walk - no hand-rolled TCB field offsets. This also
 * guarantees the tskSTACK_FILL_BYTE fill in prvAllocateTCBAndStack(). */
#undef INCLUDE_uxTaskGetStackHighWaterMark
#define INCLUDE_uxTaskGetStackHighWaterMark 1

/* Bring-up evidence: compiles uxTaskGetSystemState(), used by the one-shot task
 * map to enumerate every task and print its real stack base via the
 * source-backed TCB mirror. Also adds uxTCBNumber/uxTaskNumber to the TCB (the
 * mirror only reads fields before pcTaskName, so its layout is unaffected). */
#undef configUSE_TRACE_FACILITY
#define configUSE_TRACE_FACILITY 1

/* Print a marker before pvPortMalloc returns NULL (hook in
 * base/fixups/boot_trace.c returns, so FreeRTOS semantics are unchanged). */
#undef configUSE_MALLOC_FAILED_HOOK
#define configUSE_MALLOC_FAILED_HOOK 1
