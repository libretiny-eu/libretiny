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
#include "BkDriverFlash.h"
#include "BkDriverWdg.h"
#include "driver_pub.h"
#include "func_pub.h"
#include "include.h"
#include "intc.h"
#include "param_config.h"
#include "start_type_pub.h"

#if CFG_SUPPORT_BOOTLOADER
void entry_set_world_flag(void) {
	*(volatile uint32_t *)0x00400000 = 1;
}
#endif // CFG_SUPPORT_BOOTLOADER

extern void lt_main(void);

// declare as weak to override with Arduino framework
__attribute__((weak)) void __wrap_bk_printf_disable();
__attribute__((weak)) void __wrap_bk_printf_enable();

unsigned char __bk_rf_is_init = 0;

void entry_main(void) {
	// compatibility with BK7231S_1.0.5
#if CFG_SUPPORT_BOOTLOADER
	entry_set_world_flag();
#endif
	// suppress all output during initialization
	__wrap_bk_printf_disable();
	// read reboot cause into bk_misc_get_start_type()
	bk_misc_init_start_type();
	// register all sctrl drivers (driver/common/dd.c dd_init_tbl[])
	driver_init();
	// reboot the board if start_type == RESET_SOURCE_CRASH_PER_XAT0
	bk_misc_check_start_type();
	// init drivers
	intc_init();
	hal_flash_init();
	cfg_param_init();
	// enable watchdog
#if CFG_INT_WDG_ENABLED
	bk_wdg_initialize(CFG_INT_WDG_PERIOD_MS);
	bk_wdg_reload();
#endif
	// enable bk_printf output again
	__wrap_bk_printf_enable();
	// run the app
	lt_main();
}
