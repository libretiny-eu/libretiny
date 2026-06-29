/* Copyright (c) Kuba Szczodrzyński 2023-03-02. */

// fix conflicting declaration with ArduinoCore-API
#ifdef ARDUINO
#define boolean boolean_rtl
#endif

#include <stdbool.h>

// the SDK declares bool if not defined before
// which conflicts with C++ built-in bool
// so it's either -fpermissive or this:
#ifndef bool
#define bool bool
#endif

#include_next "basic_types.h"

#undef boolean
