/* Copyright (c) Kuba Szczodrzyński 2022-07-21. */

#pragma once

#include_next "time64.h"

// GCC 10.3.1 does not provide these structs by default
#include <sys/_tz_structs.h>
