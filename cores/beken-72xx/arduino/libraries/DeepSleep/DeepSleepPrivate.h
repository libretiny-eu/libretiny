/* Copyright (c) Peter Sarkozi 2023-06-13. */

#pragma once

#include <DeepSleep.h>

extern "C" {
  typedef struct {
	uint32_t gpioMap;
        bool onHigh;
  } DeepSleepData;



#define DATA  ((DeepSleepData *)data)
}
