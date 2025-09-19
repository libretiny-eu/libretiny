/* Copyright (c) Kuba Szczodrzyński 2023-03-14. */

#include_next "include.h"

#pragma once

// force including fixups/generic.h, even by BDK/include.h
#include "generic.h"

#if CFG_SOC_NAME == SOC_BK7231
#include "bk7231q.h"
#endif
