/* Copyright (c) Kuba Szczodrzyński 2022-06-19. */

#include <Arduino.h>

extern "C" {

#include <rtos_pub.h>
#include <sys_rtos.h>

extern int uart_print_port;

} // extern "C"

beken_thread_t mainThread;

void initArduino() {
	// set default UART output port
	uart_print_port = LT_UART_DEFAULT_PORT - 1;
}

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
