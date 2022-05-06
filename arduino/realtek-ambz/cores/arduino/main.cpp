/*
  main.cpp - Main loop for Arduino sketches
  Copyright (c) 2005-2013 Arduino Team.  All right reserved.

  This library is free software; you can redistribute it and/or
  modify it under the terms of the GNU Lesser General Public
  License as published by the Free Software Foundation; either
  version 2.1 of the License, or (at your option) any later version.

  This library is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
  Lesser General Public License for more details.

  You should have received a copy of the GNU Lesser General Public
  License along with this library; if not, write to the Free Software
  Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
*/

#define ARDUINO_MAIN

#include <Arduino.h>
#include <cmsis_os.h>

// Weak empty variant initialization function.
// May be redefined by variant files.
void initVariant() __attribute__((weak));

void initVariant() {}

// Initialize C library
extern "C" void __libc_init_array(void);

osThreadId main_tid = 0;

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
	LT_BANNER();
	init();

	__libc_init_array();

	initVariant();

	osThreadDef(main_task, osPriorityRealtime, 1, 4096 * 4);
	main_tid = osThreadCreate(osThread(main_task), NULL);

	osKernelStart();

	while (1)
		;

	return 0;
}

void serialEvent() __attribute__((weak));
bool Serial_available() __attribute__((weak));

void serialEventRun(void) {
	if (Serial_available && serialEvent && Serial_available())
		serialEvent();
}

void wait_for_debug() {
	while (((CoreDebug->DHCSR) & CoreDebug_DHCSR_C_DEBUGEN_Msk) == 0) {
		asm("nop");
	}
	delay(1000);
}
