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
#include "driver_pub.h"
#include "func_pub.h"
#include "include.h"
#include "start_type_pub.h"

extern void user_main_entry(void);

#if CFG_SUPPORT_BOOTLOADER
void entry_set_world_flag(void) {
	*(volatile uint32_t *)0x00400000 = 1;
}
#endif // CFG_SUPPORT_BOOTLOADER

extern void main(void);

void entry_main(void) {
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
	// init drivers
	func_init_basic();
	func_init_extended();
	// run core initialization tasks
	app_pre_start();
	// run the app
	main();
}
