/* Copyright (c) Kuba Szczodrzyński 2023-03-02. */

// fix conflicting declaration with ArduinoCore-API
#ifdef ARDUINO
#define boolean boolean_rtl
#endif
#include_next "basic_types.h"
#undef boolean
