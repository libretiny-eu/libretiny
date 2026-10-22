/* Copyright (c) Kuba Szczodrzyński 2022-06-13. */

#pragma once

// This is an attempt to bring at least some order to <errno.h>, as
// it's generally a source of problems everywhere.
// The idea is that all units will try to import this errno.h first,
// which means it won't use lwIP's error codes.
// The code below was moved from realtek-ambz/fixups during
// porting of BK72XX SDK, when the errno stroke again.

// There are two different errno's:
// - first is just an int
// - second is a macro that calls __errno()
// Here the first option is ensured in the entire project.
#include <sys/errno.h> // use system __errno() & error codes
#undef errno		   // undefine __errno() macro
extern int errno;	   // use a global errno variable
#define errno errno	   // for #ifdef errno in lwIP

// make sure lwIP never defines its own error codes
#undef LWIP_PROVIDE_ERRNO
