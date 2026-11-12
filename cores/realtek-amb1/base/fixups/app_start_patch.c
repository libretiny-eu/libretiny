/* Ameba1 InfraStart performs platform init, then calls _AppStart.
 * The SDRAM linker retains constructor arrays consumed by lt_main. */
#include "ameba_soc.h"
#include "FreeRTOS.h"
#include "task.h"

extern int lt_main(void);
extern void vPortSVCHandler(void);
extern void xPortPendSVHandler(void);
extern void xPortSysTickHandler(void);

/* Boot-crash instrumentation, see fixups/boot_trace.c (temporary). */
extern void lt_amb1_trace(const char *tag);
extern void lt_amb1_trace2(const char *tag, uint32_t value);
extern void lt_amb1_trace_init(void);
extern void lt_amb1_trace_tcm(const char *tag);
extern void lt_amb1_app_log_test(const char *tag);
extern void lt_amb1_fault_dump(void *frame);
/* lt_amb1_heartbeat_init() is deliberately NOT called from this file: it calls
 * xTimerCreate/xTimerStart, which allocate from the FreeRTOS heap in .sdr_bss
 * that is still garbage until the _memset calls below. It is called from
 * fal_flash_amb1_port.c's init() hook (post-BSS, post-__libc_init_array). */

/* The shared lt_init.h declares lt_init_family() weak, and
 * libcore_common.a(lt_main.c.o) ships an empty weak stub for it, so the amb1
 * implementation in libcore_amb1.a(lt_init.c.o) is never pulled in
 * (candidate001 ELF has no lt_init_family symbol at all). This strong
 * reference from an always-linked object (--undefined=_AppStart) forces the
 * archive member in; the strong definition then wins over the weak stub. */
extern void lt_init_family(void);
__attribute__((used)) static void (*const lt_amb1_keep_lt_init_family)(void) = lt_init_family;

extern unsigned char __sdram_bss_start__[], __sdram_bss_end__[];
extern unsigned char __tcm_bss_start__[], __tcm_bss_end__[];

/* Strong override of the weak hook in lib_platform.a(startup.o), which is
 * called by HalHardFaultHandler_Patch_c before the ROM HalHardFaultHandler.
 * Defining it in this file guarantees the hook is linked: app_start_patch.o is
 * force-linked via -Wl,--undefined=_AppStart, and its reference to
 * lt_amb1_fault_dump pulls in boot_trace.c.o as well. */
void HalHardFaultHandler_user_define(void *frame) {
	lt_amb1_fault_dump(frame);
}

/* Both symbols reside in SDK app_start.o; providing both keeps its default
 * main/_AppStart implementation out of the link. */
void ShowRamBuildInfo(void) {}

void lt_amb1_app_start(void) __attribute__((noreturn));

/* Align before a C prologue: changing SP inside a normal function corrupts
 * its saved registers when the loader supplied only 4-byte alignment. */
__attribute__((naked, noreturn)) void _AppStart(void) {
	__asm volatile("mov r0, sp\n"
		"bic r0, r0, #7\n"
		"mov sp, r0\n"
		"b lt_amb1_app_start\n");
}

void lt_amb1_app_start(void) {
	lt_amb1_trace_init();
	lt_amb1_app_log_test("early, right after log-UART IER mask");
	lt_amb1_trace("_AppStart -> lt_amb1_app_start");
	/* ORDERING RULE: nothing here may call a timer/heap/GPIO-SDK API. The SDRAM
	 * BSS (including the FreeRTOS heap and its bookkeeping) is still garbage
	 * until the _memset calls below; candidate005 created a software timer here
	 * and hard-faulted inside the allocator before "sdram bss cleared"
	 * (pc=0x3001cbec, BFAR=0x54555559). The heartbeat timer is created from the
	 * FAL flash0 init hook instead (after these clears and after
	 * __libc_init_array), and the LED is initialised from the timer callback
	 * once the scheduler is running. tests/test_startup_ordering.py guards this. */
	/* InfraStart clears SRAM BSS only; retain the SDK's other BSS clears. */
	_memset(__sdram_bss_start__, 0, __sdram_bss_end__ - __sdram_bss_start__);
	lt_amb1_trace("sdram bss cleared");
	_memset(__tcm_bss_start__, 0, __tcm_bss_end__ - __tcm_bss_start__);
	lt_amb1_trace("tcm bss cleared");
	/* Observational only - no allocation is attempted from the SDK TCM heap. */
	lt_amb1_trace_tcm("tcm after bss clear");
	InterruptForOSInit((VOID *)vPortSVCHandler, (VOID *)xPortPendSVHandler, (VOID *)xPortSysTickHandler);
	lt_amb1_trace("os vectors set -> lt_main");
	lt_main();
	/* Arduino starts the scheduler; never return into the ROM loader. */
	for (;;) {}
}
