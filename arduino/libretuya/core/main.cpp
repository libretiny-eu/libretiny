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

static unsigned long periodicTasks[] = {0, 0};

void runPeriodicTasks() {
#if LT_LOG_HEAP
	if (millis() - periodicTasks[0] > 1000) {
		LT_HEAP_I();
		periodicTasks[0] = millis();
	}
#endif
#if LT_USE_TIME
	if (millis() - periodicTasks[1] > 10000) {
		gettimeofday(NULL, NULL);
		periodicTasks[1] = millis();
	}
#endif
}

int main(void) {
	// print a startup banner
	LT_BANNER();
	// initialize C library
	__libc_init_array();
	// inform about the reset reason
	LT_I("Reset reason: %u", LT.getResetReason());
	// initialize Arduino framework
	initArduino();
	// optionally initialize per-variant code
	initVariant();
	// initialize FAL
	fal_init();
	// provide root partition
	fal_root_part = (fal_partition_t)fal_partition_find("root");
	// start the main task and OS kernel
	if (!startMainTask()) {
		LT_F("Couldn't start the main task");
	}

	while (1) {}
	return 0;
}
