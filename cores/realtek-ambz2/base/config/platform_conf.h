/* Copyright (c) Kuba Szczodrzyński 2023-05-23. */

#pragma once

#include_next "platform_conf.h"

#undef CONFIG_DEBUG_LOG
#undef CONFIG_DEBUG_ERROR
#undef CONFIG_DEBUG_WARN
#undef CONFIG_DEBUG_INFO
#define CONFIG_DEBUG_LOG   0
#define CONFIG_DEBUG_ERROR 0
#define CONFIG_DEBUG_WARN  0
#define CONFIG_DEBUG_INFO  0

// diag.h doesn't define this if CONFIG_DEBUG_LOG is 0
#define DBG_SCE_ERR(...)
#define DBG_SCE_WARN(...)
#define DBG_SCE_INFO(...)

// ...?
#define CONFIG_ADC_EN
