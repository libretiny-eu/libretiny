/* Copyright (c) Kuba Szczodrzyński 2023-03-02. */

// fix conflicting declaration with ArduinoCore-API
#define boolean boolean_rtl
#include_next "basic_types.h"
#undef boolean
