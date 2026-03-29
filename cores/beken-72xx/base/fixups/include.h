/* Copyright (c) Kuba Szczodrzyński 2023-03-14. */

#pragma once

// BDK will always redefine these in "generic.h"
#undef htons
#undef ntohs
#undef htonl
#undef ntohl

#include_next "include.h"

// force including fixups/generic.h, even by BDK/include.h
#include "generic.h"

#if CFG_SOC_NAME == SOC_BK7231
#include "bk7231q.h"
#endif
