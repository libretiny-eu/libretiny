/* Copyright (c) Kuba Szczodrzyński 2022-06-19. */

#include <libretuya.h>

#include <fal.h>

fal_partition_t fal_root_part = NULL;

// Initialize C library
void __libc_init_array(void);
// Main app entrypoint
int main(void);

int lt_main(void) {
	// early initialize the family and variant
	lt_init_family();
	lt_init_variant();
	// print a startup banner
	LT_BANNER();
	// initialize C library
	__libc_init_array();
	// inform about the reset reason
	LT_I("Reset reason: %u", lt_get_reset_reason());
	// initialize FAL
	fal_init();
	// provide root partition
	fal_root_part = (fal_partition_t)fal_partition_find("root");

	// run the application
	return main();
}
