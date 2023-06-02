/* Copyright (c) Kuba Szczodrzyński 2022-09-03. */

#include <Arduino.h>
#include <errno.h>

static uint32_t reset_epoch	 = 0; // epoch corresponding to millis() == 0
static uint32_t reset_millis = 0; // millis() when epoch reset was performed

int __wrap_gettimeofday(struct timeval *tv, void *tz) {
	if (millis() < reset_millis) {
		// the clock overflowed
		reset_epoch += UINT32_MAX / 1000;
		reset_millis = millis();
	}
	if (!tv) {
		errno = EINVAL;
		return -1;
	}
	unsigned long m = millis();
	tv->tv_sec		= reset_epoch + (m / 1000);
	tv->tv_usec		= (m % 1000) * 1000;
	return 0;
}

int __wrap_settimeofday(const struct timeval *tv, const struct timezone *tz) {
	if (!tv) {
		errno = EINVAL;
		return -1;
	}
	unsigned long m = millis();
	reset_epoch		= tv->tv_sec - (m / 1000);
	reset_millis	= m;
	return 0;
}

int gettimeofday(struct timeval *tv, void *tz) {
	return __wrap_gettimeofday(tv, tz);
}

int settimeofday(const struct timeval *tv, const struct timezone *tz) {
	return __wrap_settimeofday(tv, tz);
}

int _gettimeofday(struct timeval *tv, void *tz) {
	return __wrap_gettimeofday(tv, tz);
}

int _settimeofday(const struct timeval *tv, const struct timezone *tz) {
	return __wrap_settimeofday(tv, tz);
}
