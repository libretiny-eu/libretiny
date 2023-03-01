/* Copyright (c) Kuba Szczodrzyński 2022-06-27. */

#pragma once

#include_next "lwipopts.h"

// mDNS support
#undef MEMP_NUM_UDP_PCB
#define LWIP_NUM_NETIF_CLIENT_DATA 2
#define MEMP_NUM_UDP_PCB		   (MAX_SOCKETS_UDP + 2 + 1)

#define ip_addr	  ip4_addr
#define ip_addr_t ip4_addr_t

// increase TCP/IP thread stack size (was 512)
#undef TCPIP_THREAD_STACKSIZE
#define TCPIP_THREAD_STACKSIZE 1024
