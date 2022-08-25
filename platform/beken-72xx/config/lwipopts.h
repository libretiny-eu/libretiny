/* Copyright (c) Kuba Szczodrzyński 2022-06-27. */

#pragma once

#include "lwip-2.0.2/port/lwipopts.h"

#include <sys/time.h>

#define LWIP_MDNS_RESPONDER		   1
#define LWIP_NUM_NETIF_CLIENT_DATA 1
#define LWIP_SO_RCVBUF			   1 // for ioctl(FIONREAD)

#define ip_addr	  ip4_addr
#define ip_addr_t ip4_addr_t
