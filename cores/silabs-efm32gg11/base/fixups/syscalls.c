/* Copyright (c) WGM160P-LibreTiny port 2026-05-26.
 *
 * Minimal newlib-nano syscall stubs for the silabs-efm32gg11 family.
 *
 * newlib-nano's libg_nano.a pulls in abort() / exit() / signal handlers that
 * reference _exit/_kill/_getpid. The Phase 1 firmware is freestanding (no OS),
 * so these system calls have no real meaning — but we still need to provide
 * definitions to satisfy the linker.
 *
 * Strategy: treat any "exit" as a hardware reset, and stub the rest to fail
 * with ENOSYS. This is the same pattern Silabs uses in their CMSIS bring-up
 * examples. Phase 2+ may replace these with FreeRTOS-aware versions.
 */

#include "em_device.h"
#include <errno.h>
#include <sys/stat.h>
#include <sys/types.h>

/* exit/abort -> hardware reset. Marked noreturn so newlib's abort path is
 * happy and so callers don't pile up dead code after a known-fatal call. */
__attribute__((noreturn)) void _exit(int status) {
	(void)status;
	NVIC_SystemReset();
	while (1) {}
}

int _kill(int pid, int sig) {
	(void)pid;
	(void)sig;
	errno = ENOSYS;
	return -1;
}

int _getpid(void) {
	/* Conventionally 1 for a single-process freestanding environment. */
	return 1;
}
