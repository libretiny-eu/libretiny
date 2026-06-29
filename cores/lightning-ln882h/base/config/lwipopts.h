/* Copyright (c) Etienne Le Cousin 2024-03-10. */

#pragma once

#include_next "lwipopts.h"

#if (!defined(LWIP_IPV4) || LWIP_IPV4) && !LWIP_IPV6
#define ip_addr	  ip4_addr	 // LwIP 2.0.x compatibility
#define ip_addr_t ip4_addr_t // LwIP 2.0.x compatibility
#endif
#if !LWIP_IPV4 && LWIP_IPV6
#define ip_addr	  ip6_addr	 // LwIP 2.0.x compatibility
#define ip_addr_t ip6_addr_t // LwIP 2.0.x compatibility
#endif
#define in_addr_t		  u32_t
#define IN_ADDR_T_DEFINED 1

#ifndef INT_MAX
#define INT_MAX 2147483647 // for RECV_BUFSIZE_DEFAULT
#endif

#define LWIP_NUM_NETIF_CLIENT_DATA	   1
#define LWIP_NETIF_EXT_STATUS_CALLBACK 1
#undef MEMP_NUM_UDP_PCB
#define MEMP_NUM_UDP_PCB 7

// LWIP_COMPAT_MUTEX cannot prevent priority inversion. It is recommended to implement priority-aware mutexes. (Define
// LWIP_COMPAT_MUTEX_ALLOWED to disable this error.)
#define LWIP_COMPAT_MUTEX_ALLOWED 1

#define LWIP_TCPIP_TIMEOUT 1
