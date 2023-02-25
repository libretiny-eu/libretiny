/* Copyright (c) Kuba Szczodrzyński 2022-07-20. */

#pragma once

#include_next "lwipopts.h"

#define ip_addr			  ip4_addr	 // LwIP 2.0.x compatibility
#define ip_addr_t		  ip4_addr_t // LwIP 2.0.x compatibility
#define in_addr_t		  u32_t
#define IN_ADDR_T_DEFINED 1

#ifndef INT_MAX
#define INT_MAX 2147483647 // for RECV_BUFSIZE_DEFAULT
#endif
