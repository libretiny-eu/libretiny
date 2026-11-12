/* Temporary candidate010 PendSV context probe. No scheduler/stack writes. */
#ifndef LT_AMB1_BRINGUP_DIAGNOSTICS
#define LT_AMB1_BRINGUP_DIAGNOSTICS 0
#endif

#if LT_AMB1_BRINGUP_DIAGNOSTICS
#include <ameba_soc.h>
#include <FreeRTOS.h>
#include <task.h>
#include <stddef.h>
#include <stdint.h>

extern unsigned char __sdram_data_start__[], __sdram_bss_end__[];
extern void __real_vTaskSwitchContext(void);

#if ( portUSING_MPU_WRAPPERS == 1 )
#error "context trace requires the non-MPU TCB prefix"
#endif

/* Same prefix as boot_trace.c, from pinned FreeRTOS/Source/tasks.c TCB_t.
 * Validate pcTaskName's public-API address before trusting the stack members. */
typedef struct {
	volatile StackType_t *pxTopOfStack;
	ListItem_t xGenericListItem;
	ListItem_t xEventListItem;
	UBaseType_t uxPriority;
	StackType_t *pxStack;
	char pcTaskName[configMAX_TASK_NAME_LEN];
} ctx_tcb_t;

_Static_assert(sizeof(StackType_t) == 4, "PendSV frame requires 32-bit words");

static unsigned ctx_transitions, ctx_samples;

static int ctx_within(uintptr_t p, size_t n, uintptr_t lo, uintptr_t hi) {
	return p >= lo && p < hi && n <= hi - p;
}

static int ctx_readable(uintptr_t p, size_t n) {
	/* SRAM and TCM are always mapped. SDRAM bounds come from this image;
	 * its BSS contains ucHeap. Deliberately exclude ROM and peripheral space. */
	return p != 0 && (ctx_within(p, n, 0x10000000u, 0x10040000u) ||
		ctx_within(p, n, 0x1fff0000u, 0x20000000u) ||
		ctx_within(p, n, (uintptr_t)__sdram_data_start__, (uintptr_t)__sdram_bss_end__));
}

typedef struct {
	char name[configMAX_TASK_NAME_LEN + 1];
	uintptr_t top, base;
	uint32_t pc, xpsr;
	int valid;
} ctx_snapshot_t;

static ctx_snapshot_t ctx_snapshot(void) {
	ctx_snapshot_t s = { .name = "?" };
	TaskHandle_t h = xTaskGetCurrentTaskHandle();
	uintptr_t a = (uintptr_t)h;
	if (a % _Alignof(ctx_tcb_t) || !ctx_readable(a, sizeof(ctx_tcb_t)))
		return s;
	const char *name = pcTaskGetTaskName(h);
	if ((uintptr_t)name != a + offsetof(ctx_tcb_t, pcTaskName))
		return s;
	for (size_t i = 0; i < configMAX_TASK_NAME_LEN; i++) {
		s.name[i] = name[i];
		if (!name[i])
			break;
	}
	const ctx_tcb_t *t = (const ctx_tcb_t *)h;
	s.top = (uintptr_t)t->pxTopOfStack;
	s.base = (uintptr_t)t->pxStack;
	if (s.top % sizeof(StackType_t) || s.base % sizeof(StackType_t) || s.top < s.base ||
		!ctx_readable(s.base, sizeof(StackType_t)) || !ctx_readable(s.top, 16 * sizeof(StackType_t)))
		return s;
	/* port.c PendSV saves r4-r11 (8 words) below the hardware frame:
	 * hardware PC = word 8+6, xPSR = word 8+7, not words 6/7. */
	const volatile StackType_t *top = (const volatile StackType_t *)s.top;
	s.pc = top[14];
	s.xpsr = top[15];
	s.valid = 1;
	return s;
}

static int ctx_recv(const char *name) {
	static const char prefix[] = "rtw_recv_";
	for (size_t i = 0; i < sizeof(prefix) - 1; i++)
		if (name[i] != prefix[i])
			return 0;
	return 1;
}

static int ctx_suspicious(const ctx_snapshot_t *s) {
	return !s->valid || s->pc == 0 || !(s->xpsr & 0x01000000u);
}

void __wrap_vTaskSwitchContext(void) {
	ctx_snapshot_t old = ctx_snapshot();
	__real_vTaskSwitchContext();
	ctx_snapshot_t next = ctx_snapshot();
	if (!ctx_recv(old.name) && !ctx_recv(next.name))
		return;
	ctx_transitions++;
	int suspicious = ctx_suspicious(&old) || ctx_suspicious(&next);
	/* Budget only valid samples: later corrupt frames must always be visible. */
	if (!suspicious) {
		if ((ctx_transitions % 32) != 0 || ctx_samples >= 32)
			return;
		ctx_samples++;
	}
	/* ROM DiagPrintf supports hex, not %u. A valid read is not a valid frame:
	 * pc=0 / xpsr without bit 24 must remain visible as raw evidence. */
	DiagPrintf("[amb1] ctx seq=%08x suspicious=%d %s top=%08x pc=%08x xpsr=%08x stack=%08x read=%d -> %s top=%08x pc=%08x xpsr=%08x stack=%08x read=%d\r\n",
		ctx_transitions, suspicious,
		old.name, (unsigned)old.top, (unsigned)old.pc, (unsigned)old.xpsr, (unsigned)old.base, old.valid,
		next.name, (unsigned)next.top, (unsigned)next.pc, (unsigned)next.xpsr, (unsigned)next.base, next.valid);
}
#else
/* The linker wrap remains stable, but production never inspects task stacks. */
extern void __real_vTaskSwitchContext(void);
void __wrap_vTaskSwitchContext(void) {
	__real_vTaskSwitchContext();
}
#endif
