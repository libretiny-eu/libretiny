#pragma once

#error "Don't include this file directly"

#define LT_ARD_HAS_WIFI		  1
#define LT_ARD_HAS_MD5		  1
#define LT_ARD_HAS_SOFTSERIAL 1

#define ARDUINO_AMEBA
#define ARDUINO_ARCH_AMBZ

// the SDK declares bool if not defined before
// which conflicts with C++ built-in bool
// so it's either -fpermissive or this:
#define bool bool
