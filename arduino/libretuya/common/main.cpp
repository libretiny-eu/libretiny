/* Copyright (c) Kuba Szczodrzyński 2022-06-19. */

#include <Arduino.h>
#include <api/HardwareSerial.h>

using namespace arduino;

extern "C" {
#include <fal.h>
fal_partition_t fal_root_part = NULL;
}

// Weak empty variant initialization function.
// May be redefined by variant files.
void initVariant() __attribute__((weak));

// Initialize C library
extern "C" void __libc_init_array(void);

void main_task(const void *arg) {
	setup();

	for (;;) {
		loop();
		if (serialEventRun)
			serialEventRun();
		yield();
	}
}

int main(void) {
	// print a startup banner
	LT_BANNER();
	// initialize Arduino framework
	init();
	// initialize C library
	__libc_init_array();
	// optionally initialize per-variant code
	initVariant();
	// initialize FAL
	fal_init();
	// provide root partition
	fal_root_part = (fal_partition_t)fal_partition_find("root");
	// start the main task and OS kernel
	startMainTask();

	while (1) {}
	return 0;
}
