/* Copyright (c) Kuba Szczodrzyński 2022-06-19. */

#include <Arduino.h>

extern "C" {

#include <rtos_pub.h>
#include <sys_rtos.h>

beken_thread_t mainThread;

#if LT_AUTO_DOWNLOAD_REBOOT && defined(PIN_SERIAL1_RX) && defined(PIN_SERIAL1_TX)
void lt_init_arduino() {
	// initialize auto-download-reboot parser
	Serial1.begin(115200);
}
#endif

bool startMainTask() {
	OSStatus ret = rtos_create_thread(
		&mainThread,
		THD_APPLICATION_PRIORITY,
		"main",
		(beken_thread_function_t)mainTask,
		8192,
		NULL
	);
	if (ret != kNoErr)
		return false;
	vTaskStartScheduler();
	return true;
}

} // extern "C"
