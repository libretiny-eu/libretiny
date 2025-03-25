/* Copyright (c) Kuba Szczodrzyński 2024-05-18. */

#pragma once

#include_next "tls_config.h"

// allow more entropy sources
#undef MBEDTLS_ENTROPY_MAX_SOURCES
#define MBEDTLS_ENTROPY_MAX_SOURCES 10
