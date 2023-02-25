// CHANGES:
// - 2022-05-08 undefine LWIP_PROVIDE_ERRNO
// - 2022-05-23 enable LWIP_MDNS_RESPONDER
// - 2022-05-23 set LWIP_NUM_NETIF_CLIENT_DATA to 1
// - 2022-05-23 set MEMP_NUM_UDP_PCB to 7


/**
  ******************************************************************************
  * @file    lwipopts.h
  * @author  MCD Application Team
  * @version V1.1.0
  * @date    07-October-2011
  * @brief   lwIP Options Configuration.
  *          This file is based on Utilities\lwip_v1.3.2\src\include\lwip\opt.h 
  *          and contains the lwIP configuration for the STM32F2x7 demonstration.
  ******************************************************************************
  * @attention
  *
  * THE PRESENT FIRMWARE WHICH IS FOR GUIDANCE ONLY AIMS AT PROVIDING CUSTOMERS
  * WITH CODING INFORMATION REGARDING THEIR PRODUCTS IN ORDER FOR THEM TO SAVE
  * TIME. AS A RESULT, STMICROELECTRONICS SHALL NOT BE HELD LIABLE FOR ANY
  * DIRECT, INDIRECT OR CONSEQUENTIAL DAMAGES WITH RESPECT TO ANY CLAIMS ARISING
  * FROM THE CONTENT OF SUCH FIRMWARE AND/OR THE USE MADE BY CUSTOMERS OF THE
  * CODING INFORMATION CONTAINED HEREIN IN CONNECTION WITH THEIR PRODUCTS.
  *
  * <h2><center>&copy; COPYRIGHT 2011 STMicroelectronics</center></h2>
  ******************************************************************************
  */

#ifndef __LWIPOPTS_H__
#define __LWIPOPTS_H__

#include_next "lwipopts.h"

#include <platform/platform_stdlib.h>
#include "platform_opts.h"
#define WIFI_LOGO_CERTIFICATION_CONFIG 0    //for ping 10k test buffer setting

/**
 * SYS_LIGHTWEIGHT_PROT==1: if you want inter-task protection for certain
 * critical regions during buffer allocation, deallocation and memory
 * allocation and deallocation.
 */
#define SYS_LIGHTWEIGHT_PROT    1

/* Define LWIP_COMPAT_MUTEX if the port has no mutexes and binary semaphores
 should be used instead */
#define LWIP_COMPAT_MUTEX         1
#define LWIP_COMPAT_MUTEX_ALLOWED 1

#define LWIP_TCPIP_TIMEOUT      1

#define ETHARP_TRUST_IP_MAC     0
#define IP_REASSEMBLY           1
#define IP_FRAG                 1
#define ARP_QUEUEING            0

/**
 * NO_SYS==1: Provides VERY minimal functionality. Otherwise,
 * use lwIP facilities.
 */
#define NO_SYS                  0

#ifndef CONFIG_DYNAMIC_TICKLESS
#define CONFIG_DYNAMIC_TICKLESS 0
#endif

/* ---------- Memory options ---------- */
/* MEM_ALIGNMENT: should be set to the alignment of the CPU for which
   lwIP is compiled. 4 byte alignment -> define MEM_ALIGNMENT to 4, 2
   byte alignment -> define MEM_ALIGNMENT to 2. */
#define MEM_ALIGNMENT           4

/* MEM_SIZE: the size of the heap memory. If the application will send
a lot of data that needs to be copied, this should be set high. */
#if WIFI_LOGO_CERTIFICATION_CONFIG
    #define MEM_SIZE                (10*1024) //for ping 10k test
#elif CONFIG_ETHERNET
	#define MEM_SIZE				(6*1024)  //for iperf test
#else
    #define MEM_SIZE                (5*1024)
#endif

/* MEMP_NUM_PBUF: the number of memp struct pbufs. If the application
   sends a lot of data out of ROM (or other static memory), this
   should be set high. */
#define MEMP_NUM_PBUF           100
/* MEMP_NUM_UDP_PCB: the number of UDP protocol control blocks. One
   per active UDP "connection". */
#define MEMP_NUM_UDP_PCB        7
/* MEMP_NUM_TCP_PCB: the number of simulatenously active TCP
   connections. */
#define MEMP_NUM_TCP_PCB        10
/* MEMP_NUM_TCP_PCB_LISTEN: the number of listening TCP
   connections. */
#define MEMP_NUM_TCP_PCB_LISTEN 5
/* MEMP_NUM_TCP_SEG: the number of simultaneously queued TCP
   segments. */
#define MEMP_NUM_TCP_SEG        20
/* MEMP_NUM_SYS_TIMEOUT: the number of simulateously active
   timeouts. */
#define MEMP_NUM_SYS_TIMEOUT    10


/* ---------- Pbuf options ---------- */
/* PBUF_POOL_SIZE: the number of buffers in the pbuf pool. */
#if WIFI_LOGO_CERTIFICATION_CONFIG
    #define PBUF_POOL_SIZE          30 //for ping 10k test
#else
    #define PBUF_POOL_SIZE          20
#endif

/* IP_REASS_MAX_PBUFS: Total maximum amount of pbufs waiting to be reassembled.*/
#if WIFI_LOGO_CERTIFICATION_CONFIG
    #define IP_REASS_MAX_PBUFS              30 //for ping 10k test
#else
    #define IP_REASS_MAX_PBUFS              10
#endif

/* PBUF_POOL_BUFSIZE: the size of each pbuf in the pbuf pool. */
#define PBUF_POOL_BUFSIZE       500


/* ---------- TCP options ---------- */
#define LWIP_TCP                1
#define TCP_TTL                 255

/* Controls if TCP should queue segments that arrive out of
   order. Define to 0 if your device is low on memory. */
#define TCP_QUEUE_OOSEQ         1

/* TCP Maximum segment size. */
#define TCP_MSS                 (1500 - 40)	  /* TCP_MSS = (Ethernet MTU - IP header size - TCP header size) */

/* TCP sender buffer space (bytes). */
#define TCP_SND_BUF             (5*TCP_MSS)

/*  TCP_SND_QUEUELEN: TCP sender buffer space (pbufs). This must be at least
  as much as (2 * TCP_SND_BUF/TCP_MSS) for things to work. */

#define TCP_SND_QUEUELEN        (4* TCP_SND_BUF/TCP_MSS)

/* TCP receive window. */
#define TCP_WND                 (2*TCP_MSS)


/* ---------- ICMP options ---------- */
#define LWIP_ICMP                       1

/* ---------- ARP options ----------- */
#define LWIP_ARP                        1

/* ---------- DHCP options ---------- */
/* Define LWIP_DHCP to 1 if you want DHCP configuration of
   interfaces. DHCP is not implemented in lwIP 0.5.1, however, so
   turning this on does currently not work. */
#define LWIP_DHCP               1


/* ---------- UDP options ---------- */
#define LWIP_UDP                1
#define UDP_TTL                 255
/* ---------- DNS options ---------- */
#define LWIP_DNS                        1

/* ---------- UPNP options --------- */
#define LWIP_UPNP		0

/* Support Multicast */
#define LWIP_IGMP                   1
#define LWIP_RAND()                 rand()
#define LWIP_SRAND()                srand(sys_now())

/* Support TCP Keepalive */
#define LWIP_TCP_KEEPALIVE				1

/*LWIP_UART_ADAPTER==1: Enable LWIP_UART_ADAPTER when CONFIG_GAGENT is enabled, 
   because some GAGENT functions denpond on the following macro definitions.*/
#define LWIP_UART_ADAPTER                   0

#if LWIP_UART_ADAPTER || CONFIG_ETHERNET
#undef  LWIP_SO_SNDTIMEO        
#define LWIP_SO_SNDTIMEO                		1

#undef  SO_REUSE        
#define SO_REUSE                        			1

#undef MEMP_NUM_NETCONN                	
#define MEMP_NUM_NETCONN                	10

#undef TCP_WND                
#define TCP_WND                                       (4*TCP_MSS)

#define TCP_KEEPIDLE_DEFAULT			10000UL
#define TCP_KEEPINTVL_DEFAULT			1000UL
#define TCP_KEEPCNT_DEFAULT			10U
#endif

#if CONFIG_EXAMPLE_UART_ATCMD || CONFIG_EXAMPLE_SPI_ATCMD 
#undef  LWIP_SO_SNDTIMEO        
#define LWIP_SO_SNDTIMEO                		1

#undef  SO_REUSE        
#define SO_REUSE                        			1

#undef SO_REUSE_RXTOALL
#define SO_REUSE_RXTOALL				1

#undef MEMP_NUM_NETCONN                	
#define MEMP_NUM_NETCONN                	10

#undef MEMP_NUM_TCP_PCB
#define MEMP_NUM_TCP_PCB				(MEMP_NUM_NETCONN)

#undef MEMP_NUM_UDP_PCB
#define MEMP_NUM_UDP_PCB				(MEMP_NUM_NETCONN)

#undef TCP_WND                
#define TCP_WND                                       	(4*TCP_MSS)

#define TCP_KEEPIDLE_DEFAULT			10000UL
#define TCP_KEEPINTVL_DEFAULT			1000UL
#define TCP_KEEPCNT_DEFAULT			10U

#define ERRNO   1

#endif

/* ---------- Statistics options ---------- */
#define LWIP_STATS 0


/*
   --------------------------------------
   ---------- Checksum options ----------
   --------------------------------------
*/

/* 
The STM32F2x7 allows computing and verifying the IP, UDP, TCP and ICMP checksums by hardware:
 - To use this feature let the following define uncommented.
 - To disable it and process by CPU comment the  the checksum.
*/
//Do checksum by lwip - WLAN nic does not support Checksum offload
//#define CHECKSUM_BY_HARDWARE 


#ifdef CHECKSUM_BY_HARDWARE
  /* CHECKSUM_GEN_IP==0: Generate checksums by hardware for outgoing IP packets.*/
  #define CHECKSUM_GEN_IP                 0
  /* CHECKSUM_GEN_UDP==0: Generate checksums by hardware for outgoing UDP packets.*/
  #define CHECKSUM_GEN_UDP                0
  /* CHECKSUM_GEN_TCP==0: Generate checksums by hardware for outgoing TCP packets.*/
  #define CHECKSUM_GEN_TCP                0 
  /* CHECKSUM_CHECK_IP==0: Check checksums by hardware for incoming IP packets.*/
  #define CHECKSUM_CHECK_IP               0
  /* CHECKSUM_CHECK_UDP==0: Check checksums by hardware for incoming UDP packets.*/
  #define CHECKSUM_CHECK_UDP              0
  /* CHECKSUM_CHECK_TCP==0: Check checksums by hardware for incoming TCP packets.*/
  #define CHECKSUM_CHECK_TCP              0
#else
  /* CHECKSUM_GEN_IP==1: Generate checksums in software for outgoing IP packets.*/
  #define CHECKSUM_GEN_IP                 1
  /* CHECKSUM_GEN_UDP==1: Generate checksums in software for outgoing UDP packets.*/
  #define CHECKSUM_GEN_UDP                1
  /* CHECKSUM_GEN_TCP==1: Generate checksums in software for outgoing TCP packets.*/
  #define CHECKSUM_GEN_TCP                1
  /* CHECKSUM_CHECK_IP==1: Check checksums in software for incoming IP packets.*/
  #define CHECKSUM_CHECK_IP               1
  /* CHECKSUM_CHECK_UDP==1: Check checksums in software for incoming UDP packets.*/
  #define CHECKSUM_CHECK_UDP              1
  /* CHECKSUM_CHECK_TCP==1: Check checksums in software for incoming TCP packets.*/
  #define CHECKSUM_CHECK_TCP              1
#endif


/*
   ----------------------------------------------
   ---------- Sequential layer options ----------
   ----------------------------------------------
*/
/**
 * LWIP_NETCONN==1: Enable Netconn API (require to use api_lib.c)
 */
#define LWIP_NETCONN                    1

/*
   ------------------------------------
   ---------- Socket options ----------
   ------------------------------------
*/
/**
 * LWIP_SOCKET==1: Enable Socket API (require to use sockets.c)
 */
#define LWIP_SOCKET                     1	

/*
   -----------------------------------
   ---------- DEBUG options ----------
   -----------------------------------
*/

#define LWIP_DEBUG                      0


/*
   ---------------------------------
   ---------- OS options ----------
   ---------------------------------
*/

#define TCPIP_THREAD_STACKSIZE          1000
#define TCPIP_MBOX_SIZE                 6
#define DEFAULT_UDP_RECVMBOX_SIZE       6
#define DEFAULT_TCP_RECVMBOX_SIZE       6
#define DEFAULT_RAW_RECVMBOX_SIZE		6
#define DEFAULT_ACCEPTMBOX_SIZE         6
#define DEFAULT_THREAD_STACKSIZE        500
#define TCPIP_THREAD_PRIO               (configMAX_PRIORITIES - 2)

/* Added by Realtek */
#ifndef DNS_IGNORE_REPLY_ERR
#define DNS_IGNORE_REPLY_ERR   1
#endif /* DNS_IGNORE_REPLY_ERR */

// for mDNS support
#define LWIP_MDNS_RESPONDER 1
#define LWIP_NUM_NETIF_CLIENT_DATA 1

#endif /* __LWIPOPTS_H__ */

/******************* (C) COPYRIGHT 2011 STMicroelectronics *****END OF FILE****/
