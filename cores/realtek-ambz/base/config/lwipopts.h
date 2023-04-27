/* Copyright (c) Kuba Szczodrzyński 2023-03-02. */

#pragma once

#include_next "lwipopts.h"

// - 2022-05-23 set LWIP_NUM_NETIF_CLIENT_DATA to 1
#define LWIP_NUM_NETIF_CLIENT_DATA 1
// - 2022-05-23 set MEMP_NUM_UDP_PCB to 7
#undef MEMP_NUM_UDP_PCB
#define MEMP_NUM_UDP_PCB 7

// LWIP_COMPAT_MUTEX cannot prevent priority inversion. It is recommended to implement priority-aware mutexes. (Define
// LWIP_COMPAT_MUTEX_ALLOWED to disable this error.)
#define LWIP_COMPAT_MUTEX_ALLOWED 1

#define LWIP_TCPIP_TIMEOUT 1
