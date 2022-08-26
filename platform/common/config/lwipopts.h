/* Copyright (c) Kuba Szczodrzyński 2022-08-26. */

#define LWIP_TIMEVAL_PRIVATE 0
#define LWIP_NETIF_HOSTNAME	 1 // to support hostname changing
#define LWIP_SO_RCVBUF		 1 // for ioctl(FIONREAD)

#define LWIP_MDNS_RESPONDER 1
#define MDNS_MAX_SERVICES	10

#include_next "lwipopts.h"
