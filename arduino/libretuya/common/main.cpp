/* Copyright (c) Kuba Szczodrzyński 2022-06-19. */

#include <Arduino.h>
#include <api/HardwareSerial.h>

using namespace arduino;

extern "C" {
#include <fal.h>
fal_partition_t fal_root_part = NULL;
}

// Arduino framework initialization.
// May be redefined by family files.
void initArduino() __attribute__((weak));

// Weak empty variant initialization function.
// May be redefined by variant files.
void initVariant() __attribute__((weak));

// Initialize C library
extern "C" void __libc_init_array(void);

void mainTask(const void *arg) {
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
	initArduino();
	// initialize C library
	__libc_init_array();
	// optionally initialize per-variant code
	initVariant();
	// initialize FAL
	fal_init();
	// provide root partition
	fal_root_part = (fal_partition_t)fal_partition_find("root");
	// start the main task and OS kernel
	if (!startMainTask()) {
		LT_E("Couldn't start the main task");
	}

	while (1) {}
	return 0;
}
