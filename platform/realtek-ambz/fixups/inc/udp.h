/* Copyright (c) Kuba Szczodrzyński 2022-05-23. */

#pragma once

#include <lwip/udp.h>

// this is included only by wifi_simple_config.c
// which uses lwip_ntohl without parentheses
// so the #define from lwip/def.h doesn't work
#define lwip_ntohl lwip_htonl
