/* Copyright (c) Peter Sarkozi 2023-06-13. */

#pragma once

#include <Arduino.h>

/**
 * @brief Deep Sleep helper class.
 *
 * The class is accessible using the `DeepSleep` global object.
 */
class LibreTinyDeepSleep {
  public:
        void *data;

  public: /* DeepSleep.cpp */
        LibreTinyDeepSleep();
        ~LibreTinyDeepSleep();
        void dataInitialize();
        void dataFree();

  public:
    void enableTimerWakeup(uint32_t duration);
    void enableGpioWakeup(uint32_t gpioIndexMap, bool onHigh);
    void enter();
};

extern LibreTinyDeepSleep DeepSleep;
