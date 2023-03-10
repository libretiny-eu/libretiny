/* Copyright (c) Kuba Szczodrzyński 2023-03-10. */

#include <Arduino.h>
#include <sdk_private.h>

uint32_t lt_get_cpu_cycle_count() {
	return microsecondsToClockCycles(micros());
}
