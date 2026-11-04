/* Copyright (c) Bl00d-B0b 2026-08-07. */

// The AmebaD SDK's app_start() calls main() directly, so lt_main() never runs
// and FAL/family init are skipped. Wrapping main() inserts the LibreTiny
// startup without patching the vendor app_start (as realtek-ambz does).

#include <libretiny.h>

#include <fal.h>

extern int __real_main(void);
extern void __libc_init_array(void);
extern fal_partition_t fal_root_part;

int __wrap_main(void) {
	lt_init_family();
	lt_init_variant();
	LT_BANNER();
	__libc_init_array();
	LT_I("Reset reason: %s", lt_get_reboot_reason_name(0));
	fal_init();
	fal_root_part = (fal_partition_t)fal_partition_find("root");
	return __real_main();
}
