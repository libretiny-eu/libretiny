/* Ameba1 (RTL8711AM) boot-crash instrumentation — TEMPORARY.
 *
 * Purpose: localise the WCATA007 boot-loop. Two facilities:
 *
 *   1. lt_amb1_trace(tag) — phase markers along the early boot path, printed
 *      with the ROM's DiagPrintf (same UART the ROM banner / SDK messages use,
 *      so the existing H7 capture sees them). MSP/PSP are printed too, which
 *      answers whether the app kept the ROM's TCM-top stack.
 *
 *   2. lt_amb1_fault_dump() — called from HalHardFaultHandler_user_define()
 *      (defined in app_start_patch.c so that it is guaranteed to be linked).
 *      The weak hook is invoked by lib_platform.a(startup.o)
 *      HalHardFaultHandler_Patch_c *before* it chains to the ROM
 *      HalHardFaultHandler, so the existing ROM register dump is still emitted.
 *
 *      2026-11-12 finding this is built around: PC=0x1587a is inside the ROM's
 *      rt_sha1_update, whose prologue is `stmdb sp!,{r4,r5,r6,r7,r8,lr}`. The
 *      hardware fault frame therefore sits immediately *below* that saved
 *      register block, i.e.
 *          frame+0x20 = r4  (SHA-1 context)
 *          frame+0x24 = r5  (input+1; for PC=0x1587a the read is [r5-1])
 *          frame+0x28 = r6, +0x2c = r7 (input+len), +0x30 = r8
 *          frame+0x34 = the *return address* of the SHA-1 caller
 *      That saved LR is the value actually needed (the LR in the hardware frame
 *      is garbage here); for a normal call it must be one of the ROM
 *      rt_hmac_sha1 call sites (0x159ad/0x159b7/0x159ed/0x159f7/0x15a55).
 *
 * Every stack/frame read is range-checked so that a frame near the top of TCM
 * (0x20000000) or a corrupt SP cannot cause a nested fault inside the handler.
 * Optional diagnostics: enable with LT_AMB1_BRINGUP_DIAGNOSTICS=1.
 * Normal firmware retains only the UART interrupt mask and ROM fault report.
 */

#include <ameba_soc.h>

#ifndef LT_AMB1_BRINGUP_DIAGNOSTICS
#define LT_AMB1_BRINGUP_DIAGNOSTICS 0
#endif

/* Functional boot fix, not instrumentation: keep polled TX without changing
 * the ROM baud rate, even when all bring-up diagnostics are disabled. */
void lt_amb1_trace_init(void) {
#if LT_AMB1_BRINGUP_DIAGNOSTICS
	u32 ier_before = HAL_READ32(0x40003000u, 0x04u);
#endif
	HAL_WRITE32(0x40003000u, 0x04u, 0);
#if LT_AMB1_BRINGUP_DIAGNOSTICS
	u32 ier_after = HAL_READ32(0x40003000u, 0x04u);
	u32 lsr = HAL_READ32(0x40003000u, 0x14u);
	DiagPrintf("\r\n[amb1] log uart IER %08x -> %08x (LSR=%08x)\r\n", (unsigned)ier_before, (unsigned)ier_after,
		(unsigned)lsr);
#endif
}

#if LT_AMB1_BRINGUP_DIAGNOSTICS
#include <FreeRTOS.h>
#include <task.h>
#include <timers.h>
#include <stddef.h>

/* Linker-provided TCM bss bounds (the region our _AppStart clears). */
extern unsigned char __tcm_bss_start__[], __tcm_bss_end__[];

/* The app's own log backend (base/port/printf.c). */
extern void lt_amb1_log_putc(char c);
extern void lt_amb1_log_puts(const char *s);

/* ------------------------------------------------------------------ */
/* helpers                                                             */
/* ------------------------------------------------------------------ */

static u32 tr_msp(void) {
	u32 v;
	__asm volatile("mrs %0, msp" : "=r"(v));
	return v;
}

static u32 tr_psp(void) {
	u32 v;
	__asm volatile("mrs %0, psp" : "=r"(v));
	return v;
}

/* Is a word address inside a region this SoC actually maps? */
static int tr_readable(u32 a) {
	if (a < 0x00020000u)
		return 1; /* mask ROM */
	if (a >= 0x10000000u && a < 0x10040000u)
		return 1; /* 256 KiB SRAM (BD_RAM etc.) */
	if (a >= 0x1FFF0000u && a < 0x20000000u)
		return 1; /* 64 KiB TCM */
	if (a >= 0x30000000u && a < 0x30200000u)
		return 1; /* 2 MiB SDRAM */
	return 0;
}

static u32 tr_rd(u32 *p, u32 *out) {
	if (((u32)p & 3u) || !tr_readable((u32)p) || !tr_readable((u32)p + 3u)) {
		*out = 0xDEADDEADu;
		return 0;
	}
	*out = *p;
	return 1;
}

/* BEGIN host-tested TCM diagnostics (no allocator calls or writes). */
#define TR_TCM_START 0x1FFF0000u
#define TR_TCM_END 0x20000000u
#define TR_TCM_MAX_CHUNKS 32u

/* Half-open ranges, using subtraction so corrupt lengths cannot wrap. */
static int tr_tcm_range(u32 addr, u32 size) {
	return !(addr & 3u) && size != 0u && addr >= TR_TCM_START &&
		addr < TR_TCM_END && size <= TR_TCM_END - addr;
}

static int tr_ranges_intersect(u32 a, u32 an, u32 b, u32 bn) {
	return an && bn && (a <= b ? b - a < an : a - b < bn);
}

/* SDK component/os/os_dep/tcm_heap.c: tcm_heap_malloc(n) asks allocmem
 * for n+sizeof(int), writes that UNROUNDED value at the allocation start,
 * then returns start+4. allocmem rounds UP to sizeof(MemChunk) == 8 bytes
 * (32-bit next, int size); free passes the stored value back for rounding.
 * This is only an inferred extent: sanity checks cannot authenticate a header
 * or prove pxStack came from this allocator. Never apply it outside TCM. */
static int tr_tcm_allocation(u32 stack, u32 *base, u32 *size) {
	u32 stored;
	if (stack < TR_TCM_START + 4u || !tr_tcm_range(stack - 4u, 4u) ||
		!tr_rd((u32 *)(size_t)(stack - 4u), &stored))
		return 0;
	/* A task stack contains at least one word, not just the malloc header. */
	if (stored < 8u || stored > TR_TCM_END - (stack - 4u))
		return 0;
	u32 rounded = (stored + 7u) & ~7u;
	if (!tr_tcm_range(stack - 4u, rounded))
		return 0;
	*base = stack - 4u;
	*size = rounded;
	return 1;
}

/* Snapshot only: a fault may have interrupted an allocator update. Increasing,
 * non-overlapping links are required by freemem's address-sorted free list.
 * Return 1 only for a fully validated traversal; incomplete is NOT no overlap. */
static int tr_tcm_walk(u32 node, u32 alloc, u32 alloc_size, int *hit) {
	*hit = 0;
	for (u32 i = 0; node && i < TR_TCM_MAX_CHUNKS; i++) {
		u32 next, size;
		if (!tr_tcm_range(node, 8u) ||
			!tr_rd((u32 *)(size_t)node, &next) ||
			!tr_rd((u32 *)(size_t)(node + 4u), &size)) {
			DiagPrintf("tcm free: invalid header @%08x\r\n", (unsigned)node);
			return 0;
		}
		int valid = size >= 8u && !(size & 7u) && tr_tcm_range(node, size);
		int overlap = valid && alloc_size && tr_ranges_intersect(node, size, alloc, alloc_size);
		*hit |= overlap;
		DiagPrintf("tcm free: @%08x next=%08x size=%08x valid=%d intersects=%d\r\n",
			(unsigned)node, (unsigned)next, (unsigned)size, valid, alloc_size ? overlap : -1);
		if (!valid || (next && (!tr_tcm_range(next, 8u) || next < node + size))) {
			DiagPrintf("tcm free: corrupt size/link; stop\r\n");
			return 0;
		}
		node = next;
	}
	if (node)
		DiagPrintf("tcm free: 32-chunk limit; incomplete\r\n");
	return node == 0u;
}
/* END host-tested TCM diagnostics. */

/* The SDK may keep this symbol local. A weak unresolved reference is NULL;
 * do not invent an address or change the allocator's linkage to obtain it. */
extern u32 g_tcm_heap[] __attribute__((weak));

static void tr_dump_tcm_free(u32 alloc, u32 alloc_size) {
	u32 head;
	if (!g_tcm_heap || !tr_rd(g_tcm_heap, &head)) {
		DiagPrintf("tcm free: g_tcm_heap unavailable\r\n");
		return;
	}
	int hit;
	int complete = tr_tcm_walk(head, alloc, alloc_size, &hit);
	DiagPrintf("tcm free: complete=%d any_intersection=%d (inferred task allocation; -1=unknown)\r\n",
		complete, alloc_size && (hit || complete) ? hit : -1);
}

void lt_amb1_trace(const char *tag) {
	DiagPrintf("\r\n[amb1] %s (msp=%08x psp=%08x)\r\n", tag, (unsigned)tr_msp(), (unsigned)tr_psp());
}

/* Same, with one numeric value appended. Hex-only: the ROM DiagPrintf does not
 * consume an argument for "%u" (it prints a literal 'u'), which shifts every
 * later vararg — candidate001/002 markers carried wrong MSP/port values. */
void lt_amb1_trace2(const char *tag, u32 value) {
	DiagPrintf("\r\n[amb1] %s=0x%08x msp=%08x psp=%08x\r\n", tag, (unsigned)value, (unsigned)tr_msp(),
			   (unsigned)tr_psp());
}

/* Observational only: the TCM overlay VMAs fixed by
 * cores/realtek-amb1/misc/rlx8195A-symbol-v02-img2.ld. TCM is always mapped, so
 * these reads cannot fault, but they are only meaningful while that layout is
 * unchanged. Nothing in SDRAM is dereferenced (those addresses move between
 * builds).
 *
 * Historical note: the SDK's TCM heap (component/os/os_dep/tcm_heap.c, enabled
 * by CONFIG_USE_TCM_HEAP=1 for CONFIG_PLATFORM_8195A) is hard-coded to
 * base 0x1FFF0000 / size 0xA000 and hands blocks out from the *end* of a free
 * chunk, so it would eventually reach these addresses. A "reserve the overlay
 * range" experiment tried to claim the bottom of that heap; on hardware the
 * linked allocator differed from the ported source arithmetic by 4 bytes
 * (top came back 0x1FFF1FFC, leftover size 0x1FF8) so the second allocation
 * failed, the top 32 KiB stayed allocated, and the boot stalled after
 * LwIP_Init. The reservation was removed. This instrumentation neither disables
 * the TCM heap nor reserves any part of it. */
static const u32 tcm_overlay_probe[] = {
	0x1FFF0000u, /* gRamStartFun (also the tcm_heap header when TCM heap is on) */
	0x1FFF0004u, /* gRamPatchWAKE */
	0x1FFF0008u, /* gRamPatchFun0 */
	0x1FFF000Cu, /* gRamPatchFun1 */
	0x1FFF001Cu, /* rand_x */
	0x1FFF0020u, /* SdrDramInfo */
	0x1FFF1034u, /* SdrDramTiming */
};

void lt_amb1_trace_tcm(const char *tag) {
	DiagPrintf("\r\n[amb1] %s tcm_bss=%08x..%08x overlay_words:\r\n", tag, (unsigned)&__tcm_bss_start__,
			   (unsigned)&__tcm_bss_end__);
	for (unsigned i = 0; i < sizeof(tcm_overlay_probe) / sizeof(tcm_overlay_probe[0]); i++) {
		DiagPrintf("[amb1]   @%08x = %08x\r\n", (unsigned)tcm_overlay_probe[i], (unsigned)HAL_READ32(tcm_overlay_probe[i], 0));
	}
}

/* ------------------------------------------------------------------ */
/* app log path self-test + liveness heartbeat + malloc-failure hook    */
/* ------------------------------------------------------------------ */

/* Emit the text through the *app's own* print path (base/port/printf.c ->
 * lt_amb1_log_putc). Seeing this line in the capture proves the app log backend
 * works; if only the ROM-path [amb1] markers appear, the app path is broken. */
void lt_amb1_app_log_test(const char *tag) {
	lt_amb1_log_puts("\r\n[applog] ");
	lt_amb1_log_puts(tag);
	lt_amb1_log_puts("\r\n");
}

/* ------------------------------------------------------------------ */
/* bounded status-LED diagnostic (PC_2)                                */
/* ------------------------------------------------------------------ */

/* Evidence: fridge-local/docs/amb1-led-mapping.md (static analysis of the
 * retained factory firmware). The factory app drives its network/commissioning
 * status LED as channel 1 = PC_2 (PinName 0x22 = PORT_C<<4|2), as a plain GPIO
 * output, boot default HIGH; its write path has no inversion, so logical 1 = pin
 * HIGH. Physical illumination polarity (does HIGH light the LED) is a board
 * property and is NOT decidable from firmware - both readings are listed in the
 * bring-up notes. PC_3 (the led_controller channel) and every pin not present in
 * the factory table are deliberately left untouched.
 *
 * The pin is only driven from the diagnostic heartbeat timer, i.e. after the
 * scheduler is up: startup timing is unchanged, nothing blocks, and a missing
 * blink therefore also means "the scheduler never started". */
/* Retired bring-up experiment, retained as historical source only. The core
 * must never drive these pins; the application owns the status LED. */
#if 0
#include "PinNames.h"
#include "gpio_api.h"

#define LED_PULSES 3u /* slow blinks; 2 s per half-cycle at the heartbeat rate */

/* Temporary identification mode: blink each factory output pin in turn
 * (PC_2, PC_3, PC_0) so the populated indicator can be found without guessing.
 * Set to 0 to restore the single-pin test below. */
#ifndef LT_AMB1_LED_IDENTIFY
#define LT_AMB1_LED_IDENTIFY 1
#endif

#if LT_AMB1_LED_IDENTIFY
extern void lt_amb1_led_identify_step(void);
#endif

#if !LT_AMB1_LED_IDENTIFY
static gpio_t s_led;
static unsigned s_led_inited;
static unsigned s_led_halves; /* LED half-cycles emitted so far */

/* Driven from the heartbeat timer callback, i.e. only once the scheduler is
 * running (never from _AppStart): the pin is initialised on the first tick and
 * left at the factory boot default HIGH. gpio_init/dir/mode/write are plain
 * pinmux/register calls in the SDK's gpio_api.c - no heap use - but they must
 * still not run before the BSS clears, so they live here. */
static void led_init(void) {
	gpio_init(&s_led, PC_2);
	/* gpio_init already selects PullNone. gpio_dir/mode re-enter HAL_GPIO_Init,
	 * whose GpioFunctionChk rejects the pin claimed by gpio_init, leaving DDR
	 * as input. The SDK's direct direction API updates DDR without re-claiming. */
	gpio_change_dir(&s_led, PIN_OUTPUT);
	gpio_write(&s_led, 1); /* factory boot default */
	s_led_inited = 1;
	DiagPrintf("[amb1] led PC_2 init dir=out mode=pullnone level=HIGH (factory default)\r\n");
}

/* One level change per heartbeat tick => 2 s half-cycles. Bounded to LED_PULSES
 * blinks; afterwards the pin is left HIGH (factory boot default) and untouched. */
static void led_step(void) {
	if (!s_led_inited) {
		led_init();
		return;
	}
	if (s_led_halves >= LED_PULSES * 2)
		return;
	int high = (s_led_halves & 1u) ? 1 : 0; /* LOW, HIGH, LOW, ... */
	s_led_halves++;
	gpio_write(&s_led, high);
	DiagPrintf("[amb1] led pulse %d/%d level=%s\r\n", (int)((s_led_halves + 1u) >> 1), (int)LED_PULSES, high ? "HIGH" : "LOW");
	if (s_led_halves == LED_PULSES * 2)
		DiagPrintf("[amb1] led done: %d pulses emitted, holding HIGH (factory boot default)\r\n", (int)LED_PULSES);
}
#endif /* !LT_AMB1_LED_IDENTIFY */
#endif /* LT_AMB1_STATUS_LED */

/* ------------------------------------------------------------------ */
/* source-backed TCB prefix mirror (shared by the fault dump + task map) */
/* ------------------------------------------------------------------ */

/* Identify a task's stack boundaries without guessing a TCB offset. Public APIs
 * give the name and the free-stack count, but v8.2.3 has no API for the stack
 * base (TaskStatus_t.pxStackBase arrived in later versions). So this mirrors the
 * *exact* TCB_t prefix from the pinned kernel source
 * (library-freertos/FreeRTOS/Source/tasks.c, struct tskTaskControlBlock):
 *
 *     volatile StackType_t *pxTopOfStack;   // member 0, documented "MUST BE FIRST"
 *     ListItem_t            xGenericListItem;
 *     ListItem_t            xEventListItem;
 *     UBaseType_t           uxPriority;
 *     StackType_t          *pxStack;
 *     char                  pcTaskName[ configMAX_TASK_NAME_LEN ];
 *
 * ListItem_t comes from freertos_list.h (via task.h), so both list members have
 * exactly the kernel's size/alignment. The mirror is then validated at runtime:
 * pcTaskGetTaskName(handle) returns &pxTCB->pcTaskName[0], so if
 * (name_ptr - handle) equals offsetof(tr_tcb_head_t, pcTaskName) the layout is
 * the kernel's; otherwise the stack fields are not read at all. */
#if ( portUSING_MPU_WRAPPERS == 1 )
#error "tr_tcb_head_t mirrors the TCB_t prefix with portUSING_MPU_WRAPPERS == 0"
#endif

typedef struct {
	volatile StackType_t *pxTopOfStack;
	ListItem_t xGenericListItem;
	ListItem_t xEventListItem;
	UBaseType_t uxPriority;
	StackType_t *pxStack;
	char pcTaskName[configMAX_TASK_NAME_LEN];
} tr_tcb_head_t;

static int tr_tcb_layout_ok(TaskHandle_t h) {
	const char *name = pcTaskGetTaskName(h);
	return name != NULL && (size_t)(name - (const char *)h) == offsetof(tr_tcb_head_t, pcTaskName);
}

#if configUSE_TIMERS
/* Liveness beacon: if nothing else prints, these lines still show whether the
 * scheduler is running (and the tick is advancing), which separates "stuck" from
 * "silent log". Bounded so the capture stays readable. */
static TimerHandle_t hb_timer;

#if ( configUSE_TRACE_FACILITY == 1 )
/* Snapshot currently existing tasks with uxTaskGetSystemState() and read each
 * one's real stack base/saved-SP through the same source-backed mirror. Runs
 * from a timer callback (scheduler up; uxTaskGetSystemState suspends the
 * scheduler internally), never from the fault handler. */
#define TR_TASK_MAP_MAX 20
static TaskStatus_t s_task_map[TR_TASK_MAP_MAX];
static TimerHandle_t map_timer;

static void tr_dump_task_map(TimerHandle_t t) {
	(void)t;
	UBaseType_t n = uxTaskGetSystemState(s_task_map, TR_TASK_MAP_MAX, NULL);
	DiagPrintf("\r\n[amb1] task map: %d task(s), one-shot snapshot; later WLAN tasks may be absent\r\n", (int)n);
	for (UBaseType_t i = 0; i < n; i++) {
		TaskHandle_t h = s_task_map[i].xHandle;
		if (h == NULL || !tr_readable((u32)h) || !tr_tcb_layout_ok(h))
			continue;
		{
			const tr_tcb_head_t *tc = (const tr_tcb_head_t *)h;
			DiagPrintf("[amb1]   %s stack=%08x sp=%08x free=%d words\r\n", s_task_map[i].pcTaskName,
					   (unsigned)tc->pxStack, (unsigned)tc->pxTopOfStack, (int)s_task_map[i].usStackHighWaterMark);
		}
	}
}

/* One-shot before the first heartbeat: bases/saved SPs only, not extents.
 * This cannot inventory WLAN tasks created after the callback runs. A zero
 * count can also mean the fixed snapshot buffer was too small. */
static void lt_amb1_task_map_start(void) {
	map_timer = xTimerCreate("amb1map", pdMS_TO_TICKS(100), pdFALSE, NULL, tr_dump_task_map);
	if (map_timer)
		xTimerStart(map_timer, 0);
}
#else
static void lt_amb1_task_map_start(void) {}
#endif

static uint32_t hb_count;

static void hb_cb(TimerHandle_t t) {
	(void)t;
	hb_count++;
	if (hb_count > 30) {
		DiagPrintf("[amb1] heartbeat stop after %d\r\n", (int)hb_count);
		xTimerStop(hb_timer, 0);
		return;
	}
	DiagPrintf("[amb1] heartbeat #%d tick=%d\r\n", (int)hb_count, (int)xTaskGetTickCount());
}

void lt_amb1_heartbeat_init(void) {
	hb_timer = xTimerCreate("amb1hb", pdMS_TO_TICKS(2000), pdTRUE, NULL, hb_cb);
	if (!hb_timer) {
		DiagPrintf("[amb1] heartbeat timer create failed\r\n");
		return;
	}
	if (xTimerStart(hb_timer, 0) != pdPASS)
		DiagPrintf("[amb1] heartbeat timer start failed\r\n");
	/* One-shot stack map, well before the first heartbeat tick. */
	lt_amb1_task_map_start();
}
#else
void lt_amb1_heartbeat_init(void) {}
#endif

/* FreeRTOS calls this before pvPortMalloc returns NULL (configUSE_MALLOC_FAILED_HOOK).
 * Print and return, so FreeRTOS keeps its documented semantics. */
void vApplicationMallocFailedHook(void) {
	DiagPrintf("[amb1] pvPortMalloc FAILED (heap exhausted?)\r\n");
}

/* ------------------------------------------------------------------ */
/* hard fault frame dump                                               */
/* ------------------------------------------------------------------ */

#define SCB_CFSR  (*(volatile u32 *)0xE000ED28u)
#define SCB_HFSR  (*(volatile u32 *)0xE000ED2Cu)
#define SCB_MMFAR (*(volatile u32 *)0xE000ED34u)
#define SCB_BFAR  (*(volatile u32 *)0xE000ED38u)

/* The TCB prefix mirror (tr_tcb_head_t / tr_tcb_layout_ok) is defined above,
 * before the timer section, so both the fault dump and the task map use it. */

/* Print the running task: name (public API), then - only if the mirror checks
 * out - the actual stack base (pxStack) and the saved SP (pxTopOfStack), and
 * an inferred TCM allocation extent when its header passes sanity checks. */
static void tr_dump_task(u32 frame) {
	TaskHandle_t cur = xTaskGetCurrentTaskHandle();
	u32 tcb			 = (u32)cur;
	char name[configMAX_TASK_NAME_LEN + 1];
	u32 stack = 0u, sp = 0u;
	u32 alloc = 0u, alloc_size = 0u;

	if (tcb == 0u || (tcb & 3u) || !tr_readable(tcb) ||
		tcb > 0xFFFFFFFFu - sizeof(tr_tcb_head_t) ||
		!tr_readable(tcb + sizeof(tr_tcb_head_t) - 1u)) {
		DiagPrintf("task: pxCurrentTCB=%08x (not a mapped address)\r\n", (unsigned)tcb);
		tr_dump_tcm_free(0u, 0u);
		return;
	}
	if (!tr_tcb_layout_ok(cur)) {
		DiagPrintf("task: tcb=%08x (TCB layout check failed; stack fields not read)\r\n", (unsigned)tcb);
		tr_dump_tcm_free(0u, 0u);
		return;
	}
	{
		const tr_tcb_head_t *t = (const tr_tcb_head_t *)tcb;
		stack				   = (u32)t->pxStack;
		sp					   = (u32)t->pxTopOfStack;
		for (unsigned i = 0; i < configMAX_TASK_NAME_LEN; i++)
			name[i] = t->pcTaskName[i];
		name[configMAX_TASK_NAME_LEN] = '\0';
	}
	/* Avoid the high-water API's unbounded fill-pattern scan at fault time. */
	DiagPrintf("task: name=%s tcb=%08x stack=%08x sp=%08x\r\n", name, (unsigned)tcb,
			   (unsigned)stack, (unsigned)sp);
	if (tr_tcm_allocation(stack, &alloc, &alloc_size)) {
		DiagPrintf("task: inferred_tcm_allocation=%08x..%08x frame_in_payload=%d\r\n",
			(unsigned)alloc, (unsigned)(alloc + alloc_size),
			frame >= stack && frame < alloc + alloc_size && 32u <= alloc + alloc_size - frame);
	} else {
		DiagPrintf("task: allocation extent unknown\r\n");
	}
	tr_dump_tcm_free(alloc, alloc_size);
}

/* Words above the exception frame to dump (the caller's stack). 100 words =
 * 400 bytes, which covers the lwIP/HMAC caller frame and its 176-byte SHA-1
 * context (frame+0x20+0xB0) when the fault lands in rt_sha1_update. */
#define TR_STACK_WORDS 100u

void lt_amb1_fault_dump(void *frame_) {
	u32 *f = (u32 *)frame_;
	u32 v[14];
	int n = 0;

	for (int i = 0; i < 14; i++)
		n += tr_rd(f + i, &v[i]);

	DiagPrintf("\r\n*** AMB1 APP FAULT FRAME ***\r\n");
	DiagPrintf("frame=%08x msp=%08x psp=%08x\r\n", (unsigned)f, (unsigned)tr_msp(), (unsigned)tr_psp());
	/* Report the current TCB; interrupt-context faults need not originate in
	 * that task. Frame containment is reported separately, never assumed. */
	tr_dump_task((u32)f);
	DiagPrintf("tcm bss=%08x..%08x frame_in_tcm=%d\r\n", (unsigned)__tcm_bss_start__, (unsigned)__tcm_bss_end__,
			   tr_tcm_range((u32)f, 32u));
	if (n < 8) {
		DiagPrintf("frame partially unreadable (%d/14 words)\r\n", n);
	}
	DiagPrintf("r0=%08x r1=%08x r2=%08x r3=%08x r12=%08x\r\n", v[0], v[1], v[2], v[3], v[4]);
	DiagPrintf("lr=%08x pc=%08x xpsr=%08x\r\n", v[5], v[6], v[7]);
	DiagPrintf("CFSR=%08x HFSR=%08x MMFAR=%08x BFAR=%08x\r\n", SCB_CFSR, SCB_HFSR, SCB_MMFAR, SCB_BFAR);
	/* rt_sha1_update prologue: {r4-r8,lr} saved just above the hardware frame. */
	DiagPrintf("saved: r4(ctx)=%08x r5=%08x r6=%08x r7=%08x r8=%08x\r\n", v[8], v[9], v[10], v[11], v[12]);
	DiagPrintf("caller LR (frame+0x34) = %08x\r\n", v[13]);

	DiagPrintf("stack (frame+0x20 .., unreadable words marked):\r\n");
	for (u32 i = 0; i < TR_STACK_WORDS; i += 4) {
		u32 w[4] = {0xDEADDEADu, 0xDEADDEADu, 0xDEADDEADu, 0xDEADDEADu};
		int ok = 1;
		int printed = 0;
		for (int k = 0; k < 4; k++) {
			if (!tr_rd(f + 8 + i + k, &w[k])) {
				ok = 0;
				break;
			}
			printed++;
		}
		if (ok) {
			DiagPrintf("  +%08x: %08x %08x %08x %08x\r\n", 0x20u + i * 4u, w[0], w[1], w[2], w[3]);
		} else if (printed) {
			/* Partly mapped group: print what is readable, then flag it. */
			DiagPrintf("  +%08x: %08x %08x %08x %08x (partly unmapped)\r\n", 0x20u + i * 4u, w[0], w[1], w[2], w[3]);
		} else {
			/* Unmapped group (e.g. the gap between TCM and SDRAM): skip it
			 * rather than stopping, so a window that crosses a hole still shows
			 * the rest. Every read goes through tr_readable(). */
			DiagPrintf("  +%08x: (unmapped)\r\n", 0x20u + i * 4u);
		}
	}
	DiagPrintf("*** END AMB1 FAULT FRAME ***\r\n");

	/* Return to HalHardFaultHandler_Patch_c, which chains to the ROM handler. */
}

#else /* !LT_AMB1_BRINGUP_DIAGNOSTICS */

/* Keep existing call sites linkable without timers, logs or fault-time walks.
 * Returning from the fault hook leaves the bounded ROM report in control. */
void lt_amb1_trace(const char *tag) { (void)tag; }
void lt_amb1_trace2(const char *tag, u32 value) { (void)tag; (void)value; }
void lt_amb1_trace_tcm(const char *tag) { (void)tag; }
void lt_amb1_app_log_test(const char *tag) { (void)tag; }
void lt_amb1_heartbeat_init(void) {}
void vApplicationMallocFailedHook(void) {}
void lt_amb1_fault_dump(void *frame) { (void)frame; }

#endif /* LT_AMB1_BRINGUP_DIAGNOSTICS */
