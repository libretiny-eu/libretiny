/* Copyright (c) Kuba Szczodrzyński 2023-03-02. */

#pragma once

#include_next "platform_opts.h"

// - 2022-05-18 include lwip/init.h
#include <lwip/init.h>

// - 2022-05-08 change CONFIG_USE_POLARSSL to CONFIG_USE_MBEDTLS
#undef CONFIG_USE_POLARSSL
#undef CONFIG_USE_MBEDTLS
#define CONFIG_USE_POLARSSL 0
#define CONFIG_USE_MBEDTLS	1
