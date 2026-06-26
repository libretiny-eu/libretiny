/* Copyright (c) WGM160P-LibreTiny port 2026-05-26.
 *
 * Phase 1 stand-alone fault dumper.
 * LibreTiny's cores/common/ does NOT expose a shared lt_panic/lt_fault hook
 * (verified by grep against v1.12.1) — bk72xx and rtl87xx don't ship their
 * own either, relying on SDK weak defaults. We provide ours explicitly to
 * surface stack frames during early bring-up.
 *
 * Two constraints shape this file:
 *
 * 1. The handlers MUST be naked. At exception entry LR holds EXC_RETURN
 *    (bit 2 selects MSP vs PSP) and the active SP points at the hardware-
 *    pushed frame {R0-R3, R12, LR, PC, xPSR}. A normal C function pushes a
 *    prologue and `bl`s away — destroying both before they can be read.
 *
 * 2. Nothing references these symbols: the vendor startup object (always
 *    linked — it owns __Vectors) provides weak `b .` defaults, so the linker
 *    never extracts this object from libcore_efm32gg11.a on its own. The
 *    family builder passes -Wl,--undefined=HardFault_Handler to force it.
 */

#include "lt_family.h"

#include <stdio.h>

static const char *const fault_names[] = {
	"HardFault",
	"MemManage",
	"BusFault",
	"UsageFault",
};

void __attribute__((used)) lt_fault_dump(uint32_t *sp, uint32_t kind) {
	printf("\r\n*** %s ***\r\n", fault_names[kind]);
	printf("  R0    = 0x%08lx\r\n", (unsigned long)sp[0]);
	printf("  R1    = 0x%08lx\r\n", (unsigned long)sp[1]);
	printf("  R2    = 0x%08lx\r\n", (unsigned long)sp[2]);
	printf("  R3    = 0x%08lx\r\n", (unsigned long)sp[3]);
	printf("  R12   = 0x%08lx\r\n", (unsigned long)sp[4]);
	printf("  LR    = 0x%08lx\r\n", (unsigned long)sp[5]);
	printf("  PC    = 0x%08lx\r\n", (unsigned long)sp[6]);
	printf("  xPSR  = 0x%08lx\r\n", (unsigned long)sp[7]);
	printf("  CFSR  = 0x%08lx\r\n", (unsigned long)SCB->CFSR);
	printf("  HFSR  = 0x%08lx\r\n", (unsigned long)SCB->HFSR);
	printf("  BFAR  = 0x%08lx\r\n", (unsigned long)SCB->BFAR);
	printf("  MMFAR = 0x%08lx\r\n", (unsigned long)SCB->MMFAR);
	NVIC_SystemReset();
}

/* Pick the faulting stack from EXC_RETURN bit 2, pass the frame pointer and
 * a fault_names index, and tail-branch into the C dumper. No prologue, no
 * calls — LR and SP are still exactly as the exception entry left them. */
#define LT_FAULT_HANDLER(name, idx)                 \
	void __attribute__((naked)) name(void) {        \
		__asm volatile("tst lr, #4            \n"   \
					   "ite eq                \n"   \
					   "mrseq r0, msp         \n"   \
					   "mrsne r0, psp         \n"   \
					   "movs r1, #" #idx "    \n"   \
					   "b lt_fault_dump       \n"); \
	}

LT_FAULT_HANDLER(HardFault_Handler, 0)
LT_FAULT_HANDLER(MemManage_Handler, 1)
LT_FAULT_HANDLER(BusFault_Handler, 2)
LT_FAULT_HANDLER(UsageFault_Handler, 3)
