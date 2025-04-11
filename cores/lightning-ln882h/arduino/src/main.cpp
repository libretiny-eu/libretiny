/* Copyright (c) Etienne Le Cousin 2024-02-17. */

#include <Arduino.h>
#include <sdk_private.h>

extern "C" {

#define MAIN_TASK_STACK_SIZE 8192

static OS_Thread_t g_mainTask_thread;

bool startMainTask() {
	OS_Status ret = OS_ThreadCreate(
		&g_mainTask_thread,
		"main",
		(OS_ThreadEntry_t)mainTask,
		NULL,
		OS_PRIORITY_BELOW_NORMAL,
		MAIN_TASK_STACK_SIZE
	);

	if (ret != OS_OK)
		return false;
	OS_ThreadStartScheduler();
	return true;
}

void wait_for_debug() {
	while (((CoreDebug->DHCSR) & CoreDebug_DHCSR_C_DEBUGEN_Msk) == 0) {
		asm("nop");
	}
	delay(1000);
}

} // extern "C"
