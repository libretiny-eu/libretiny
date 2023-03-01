/* Copyright (c) Kuba Szczodrzyński 2023-03-01. */

#include_next "generic.h"

#pragma once

// allow lwIP to define these (Beken, why)
#undef htons
#undef ntohs
#undef htonl
#undef ntohl
