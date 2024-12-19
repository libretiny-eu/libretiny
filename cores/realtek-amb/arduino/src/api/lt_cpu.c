/* Copyright (c) Kuba Szczodrzyński 2023-03-10. */

#include <ArduinoPrivate.h>

uint32_t lt_cpu_get_cycle_count() {
	return microsecondsToClockCycles(micros());
}
