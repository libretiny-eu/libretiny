/**
 ****************************************************************************************
 *
 * @file arch_main.c
 *
 * @brief Main loop of the application.
 *
 * Copyright (C) Beken Corp 2011-2020
 *
 ****************************************************************************************
 */
#include "app.h"
#include "ate_app.h"
#include "driver_pub.h"
#include "func_pub.h"
#include "include.h"
#include "start_type_pub.h"

beken_semaphore_t extended_app_sema = NULL;
uint32_t extended_app_stack_size	= 2048;

extern void user_main_entry(void);

#if CFG_SUPPORT_BOOTLOADER
void entry_set_world_flag(void) {
	*(volatile uint32_t *)0x00400000 = 1;
}
#endif // CFG_SUPPORT_BOOTLOADER

void extended_app_launch_over(void) {
	OSStatus ret;
	ret = rtos_set_semaphore(&extended_app_sema);
	(void)ret;
}

void extended_app_waiting_for_launch(void) {
	OSStatus ret;
	ret = rtos_get_semaphore(&extended_app_sema, BEKEN_WAIT_FOREVER);
	ASSERT(kNoErr == ret);
	(void)ret;
}

static void extended_app_task_handler(void *arg) {
	func_init_extended();
	if (get_ate_mode_state()) {
		ate_start();
	} else {
		app_start();
	}
	extended_app_launch_over();
	rtos_delete_thread(NULL);
}

void extended_app_init(void) {
	OSStatus ret;
	ret = rtos_init_semaphore(&extended_app_sema, 1);
	ASSERT(kNoErr == ret);
}

void extended_app_uninit(void) {
	OSStatus ret;
	ret = rtos_deinit_semaphore(&extended_app_sema);
	ASSERT(kNoErr == ret);
}

void extended_app_launch(void) {
	OSStatus ret;
	ret = rtos_create_thread(
		NULL,
		THD_EXTENDED_APP_PRIORITY,
		"extended_app",
		(beken_thread_function_t)extended_app_task_handler,
		extended_app_stack_size,
		(beken_thread_arg_t)0
	);
	ASSERT(kNoErr == ret);
}

void entry_main(void) {
	// check GPIO0 and start ATE app
	ate_app_init();
	// compatibility with BK7231S_1.0.5
#if CFG_SUPPORT_BOOTLOADER
	entry_set_world_flag();
#endif
	// read reboot cause into bk_misc_get_start_type()
	bk_misc_init_start_type();
	// clear driver registration arrays
	driver_init();
	// reboot the board if start_type == RESET_SOURCE_CRASH_PER_XAT0
	bk_misc_check_start_type();
	// init flash and intc
	func_init_basic();
	// init semaphore for extended_app_launch
	extended_app_init();
	// run task with init_app_thread
	user_main_entry();
	// run task with extended_app_task_handler
	extended_app_launch();
	// start FreeRTOS
#if (CFG_OS_FREERTOS)
	vTaskStartScheduler();
#endif
}
