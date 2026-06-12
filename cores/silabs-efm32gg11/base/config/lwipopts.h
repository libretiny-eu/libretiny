/* Copyright (c) WGM160P-LibreTiny port 2026-06-10.
 *
 * lwIP configuration for silabs-efm32gg11 + WF200 (Full-MAC: lwIP sees plain
 * Ethernet frames; no 802.11 awareness needed). Self-contained: this family
 * compiles lwIP from the GSDK wrapper and has no vendor lwipopts.h beneath it.
 * Sized for 512 KB RAM with comfortable margins.
 *
 * Include-order note: this file is found FIRST on the include path (family
 * base/config is prepended after common base/config), so the common LT
 * lwipopts.h (with its #include_next) never applies here. The LT-required
 * options from that file are duplicated below.
 */
#pragma once

// TCPIP_THREAD_PRIO references configMAX_PRIORITIES, but lwIP TUs (tcpip.c)
// never include FreeRTOS headers themselves — pull the kernel config in here.
#include "FreeRTOSConfig.h"

// FreeRTOSConfig.h sets configENABLE_BACKWARD_COMPATIBILITY=0, so FreeRTOS.h
// does not provide the legacy portTICK_RATE_MS alias that the GSDK
// lwip-contrib freertos sys_arch.c still uses (sys_arch_msleep,
// sys_arch_sem_wait, sys_arch_mbox_fetch). Provide it here: lwipopts.h is
// included (via lwip/opt.h) before FreeRTOS.h in sys_arch.c, and expansion of
// portTICK_PERIOD_MS is lazy.
#ifndef portTICK_RATE_MS
#define portTICK_RATE_MS portTICK_PERIOD_MS
#endif

// --- OS mode / threading -----------------------------------------------------
#define NO_SYS										 0
#define SYS_LIGHTWEIGHT_PROT						 1
#define LWIP_TCPIP_CORE_LOCKING						 1
#define TCPIP_THREAD_NAME							 "tcpip"
#define TCPIP_THREAD_STACKSIZE						 1024 // words (FreeRTOS units via sys_arch)
// Make sys_thread_new() treat the stacksize above as FreeRTOS stack WORDS.
// The sys_arch.c default (0) divides by sizeof(StackType_t), i.e. byte units —
// which would silently quarter the tcpip thread stack to 256 words.
#define LWIP_FREERTOS_THREAD_STACKSIZE_IS_STACKWORDS 1
#define TCPIP_THREAD_PRIO							 (configMAX_PRIORITIES - 3)
#define TCPIP_MBOX_SIZE								 16
#define DEFAULT_RAW_RECVMBOX_SIZE					 8
#define DEFAULT_UDP_RECVMBOX_SIZE					 8
#define DEFAULT_TCP_RECVMBOX_SIZE					 8
#define DEFAULT_ACCEPTMBOX_SIZE						 4

// --- APIs --------------------------------------------------------------------
#define LWIP_NETCONN		  1
#define LWIP_SOCKET			  1
// LWIP_COMPAT_SOCKETS is deliberately NOT defined here. The common Arduino
// code needs BOTH behaviors per-TU: LwIPUdp.cpp uses unprefixed compat names
// (socket/sendto/close/..., default 1 from opt.h), while LwIPClient.cpp,
// LwIPRxBuffer.cpp and LwIPServer.cpp do `#define LWIP_COMPAT_SOCKETS 0`
// before including <lwip/sockets.h> (their read/write/connect/accept METHOD
// names would collide with the function-like compat macros). opt.h's default
// is `#if !defined` — overridable per TU; a plain #define here clobbers the
// TU opt-out via the opt.h include chain and breaks those three files.
#define LWIP_ERRNO_STDINCLUDE 1 // newlib <errno.h>; see config/arch/cc.h
#define LWIP_SO_RCVTIMEO	  1
#define LWIP_SO_SNDTIMEO	  1
#define LWIP_SO_RCVBUF		  1 // common LwIPClient: ioctl(FIONREAD)
#define LWIP_TCP_KEEPALIVE	  1

// --- Protocols -----------------------------------------------------------------
#define LWIP_ARP  1
#define LWIP_ICMP 1
#define LWIP_DHCP 1
#define LWIP_DNS  1
#define LWIP_IGMP 1
#define LWIP_IPV6 0

// --- Memory ----------------------------------------------------------------------
#define MEM_LIBC_MALLOC			0
#define MEM_ALIGNMENT			4
#define MEM_SIZE				(48 * 1024)
#define MEMP_NUM_PBUF			16
#define MEMP_NUM_TCP_PCB		8
#define MEMP_NUM_TCP_PCB_LISTEN 4
#define MEMP_NUM_UDP_PCB		8
#define MEMP_NUM_NETCONN		12
#define MEMP_NUM_SYS_TIMEOUT	12
// init.c hard-errors unless the TCP segment pool covers the send queue:
// TCP_SND_QUEUELEN defaults to (4 * TCP_SND_BUF)/TCP_MSS = 32 > the pool
// default of 16. A tcp_seg is ~24 B, so this costs ~768 B — cheap on 512 KB.
#define MEMP_NUM_TCP_SEG		TCP_SND_QUEUELEN
#define PBUF_POOL_SIZE			24
#define PBUF_POOL_BUFSIZE		LWIP_MEM_ALIGN_SIZE(TCP_MSS + 40 + PBUF_LINK_ENCAPSULATION_HLEN + PBUF_LINK_HLEN)

// --- TCP tuning ----------------------------------------------------------------------
#define TCP_MSS		1460
#define TCP_WND		(8 * TCP_MSS)
#define TCP_SND_BUF (8 * TCP_MSS)

// --- LibreTiny common-code requirements (duplicated from cores/common config) ---------
#define LWIP_TIMEVAL_PRIVATE	   0
#define LWIP_NETIF_HOSTNAME		   1
#define LWIP_NETIF_STATUS_CALLBACK 1
#define LWIP_NETIF_LINK_CALLBACK   1

// --- Misc ------------------------------------------------------------------------------
#define LWIP_STATS	0
#define LWIP_RAND() ((u32_t)rand())
#include <stdlib.h>
#include <sys/time.h>
