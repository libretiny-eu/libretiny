/* Copyright (c) Kuba Szczodrzyński 2022-06-14. */

#pragma once

#include_next "param_config.h"

#undef WIFI_MAC_POS
#define WIFI_MAC_POS -1 // do not use stored MAC address by default
