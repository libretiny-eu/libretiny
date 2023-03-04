/* Copyright (c) Kuba Szczodrzyński 2023-02-27. */

#include <Arduino.h>

// Arduino framework initialization.
// May be redefined by family files.
void initArduino() __attribute__((weak));

// Weak empty variant initialization function.
// May be redefined by variant files.
void initVariant() __attribute__((weak));

int main() {
	// initialize Arduino framework
	initArduino();
	// optionally initialize per-variant code
	initVariant();
	// start the main task and OS kernel
	if (!startMainTask()) {
		LT_F("Couldn't start the main task");
		return 1;
	}
	return 0;
}

/**
 * @brief Main setup() and loop() task.
 * Not to be called directly.
 */
void mainTask(const void *arg) {
	setup();

	for (;;) {
		loop();
		if (serialEventRun)
			serialEventRun();
		yield();
	}
}
