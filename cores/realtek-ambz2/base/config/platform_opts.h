/* Copyright (c) Kuba Szczodrzyński 2023-05-25. */

#pragma once

#include_next "platform_opts.h"

// this needs example_wlan_fast_connect.c
#undef CONFIG_EXAMPLE_WLAN_FAST_CONNECT
#undef CONFIG_FAST_DHCP
