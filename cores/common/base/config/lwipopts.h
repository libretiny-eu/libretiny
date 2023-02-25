/* Copyright (c) Kuba Szczodrzyński 2022-08-26. */

#define LWIP_TIMEVAL_PRIVATE 0
#define LWIP_NETIF_HOSTNAME	 1 // to support hostname changing
#define LWIP_SO_RCVBUF		 1 // for ioctl(FIONREAD)

#define LWIP_MDNS_RESPONDER 1
#define MDNS_MAX_SERVICES	10

#include_next "lwipopts.h"

// set lwIP debugging options according to LT config
#if LT_DEBUG_LWIP
#undef LWIP_DEBUG
#define LWIP_DEBUG 1
// make lwIP use printf() library
#include <stdio.h>
#undef LWIP_PLATFORM_DIAG
// clang-format off
#define LWIP_PLATFORM_DIAG(x) do { printf x; } while (0)
// clang-format on
#endif

#if LT_DEBUG_LWIP_ASSERT
#undef LWIP_NOASSERT
#undef LWIP_PLATFORM_ASSERT
// clang-format off
#define LWIP_PLATFORM_ASSERT(x) do { printf("ASSERT \"%s\" - %s:%d\n", x, __FILE__, __LINE__); while (1) {}; } while (0)
// clang-format on
#endif

// remove family-defined debugging options (use lwIP defaults, or user-defined)
#undef ETHARP_DEBUG
#undef NETIF_DEBUG
#undef PBUF_DEBUG
#undef API_LIB_DEBUG
#undef API_MSG_DEBUG
#undef SOCKETS_DEBUG
#undef ICMP_DEBUG
#undef IGMP_DEBUG
#undef INET_DEBUG
#undef IP_DEBUG
#undef IP_REASS_DEBUG
#undef RAW_DEBUG
#undef MEM_DEBUG
#undef MEMP_DEBUG
#undef SYS_DEBUG
#undef TIMERS_DEBUG
#undef TCP_DEBUG
#undef TCP_INPUT_DEBUG
#undef TCP_FR_DEBUG
#undef TCP_RTO_DEBUG
#undef TCP_CWND_DEBUG
#undef TCP_WND_DEBUG
#undef TCP_OUTPUT_DEBUG
#undef TCP_RST_DEBUG
#undef TCP_QLEN_DEBUG
#undef UDP_DEBUG
#undef TCPIP_DEBUG
#undef SLIP_DEBUG
#undef DHCP_DEBUG
#undef AUTOIP_DEBUG
#undef DNS_DEBUG
#undef IP6_DEBUG
#undef MDNS_DEBUG


/** Set this to 1 to support DNS names (or IP address strings) to set sntp servers
 * One server address/name can be defined as default if SNTP_SERVER_DNS == 1:
 * \#define SNTP_SERVER_ADDRESS "pool.ntp.org"
 */
#define SNTP_SERVER_DNS            1

#define SNTP_SET_SYSTEM_TIME_US(sec, us)  \
    do { \
        struct timeval tv = { .tv_sec = sec, .tv_usec = us }; \
        settimeofday(&tv, NULL); \
    } while (0);

#define SNTP_GET_SYSTEM_TIME(sec, us) \
    do { \
        struct timeval tv = { .tv_sec = 0, .tv_usec = 0 }; \
        gettimeofday(&tv, NULL); \
        (sec) = tv.tv_sec;  \
        (us) = tv.tv_usec; \
    } while (0);

