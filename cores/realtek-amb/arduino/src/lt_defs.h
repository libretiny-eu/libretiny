#pragma once

#error "Don't include this file directly"

#define LT_ARD_HAS_WIFI		  1
#define LT_ARD_HAS_SOFTSERIAL 1
#define LT_ARD_HAS_SERIAL	  1

#define LT_ARD_MD5_POLARSSL 1

#define ARDUINO_AMEBA

// the SDK declares bool if not defined before
// which conflicts with C++ built-in bool
// so it's either -fpermissive or this:
#define bool bool
