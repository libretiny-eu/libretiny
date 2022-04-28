#pragma once

#ifndef LT_VERSION
#define LT_VERSION "1.0.0"
#endif

#include <Arduino.h>

#include "LibreTuyaConfig.h"

#ifdef __cplusplus
extern "C" {
#endif // __cplusplus

#include "lt_logger.h"

#ifdef __cplusplus
} // extern "C"
#endif

#define LT_BANNER()                                                                                                    \
	LT_LOG(LT_LEVEL_INFO, "main.cpp", __LINE__, "LibreTuya v" LT_VERSION " compiled on " __DATE__ " " __TIME__)
