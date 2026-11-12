#pragma once
#include_next "lwipopts.h"

/* netif additions and DHCP commands execute on the tcpip thread. */
#undef LWIP_NETIF_API
#define LWIP_NETIF_API 1
