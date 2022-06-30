/* Copyright (c) Kuba Szczodrzyński 2022-06-19. */

#include <Arduino.h>

extern "C" {
#include <rtos_pub.h>
#include <sys_rtos.h>
}

beken_thread_t mainThread;

bool startMainTask() {
	OSStatus ret = rtos_create_thread(
		&mainThread,
		THD_APPLICATION_PRIORITY,
		"main",
		(beken_thread_function_t)main_task,
		8192,
		NULL
	);
	if (ret != kNoErr)
		return false;
	vTaskStartScheduler();
	return true;
}
