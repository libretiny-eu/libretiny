/**
 ******************************************************************************
 *This file contains general configurations for ameba platform
 ******************************************************************************
*/

#ifndef __PLATFORM_OPTS_H__
#define __PLATFORM_OPTS_H__

/*For MP mode setting*/
#define SUPPORT_MP_MODE		0

/**
 * For AT cmd Log service configurations
 */
#define SUPPORT_LOG_SERVICE	1
#if SUPPORT_LOG_SERVICE
#define LOG_SERVICE_BUFLEN     100 //can't larger than UART_LOG_CMD_BUFLEN(127)
#define CONFIG_LOG_HISTORY	0
#if CONFIG_LOG_HISTORY
#define LOG_HISTORY_LEN    5
#endif
#define SUPPORT_INTERACTIVE_MODE	0//on/off wifi_interactive_mode
#define CONFIG_LOG_SERVICE_LOCK		0
#if SUPPORT_MP_MODE
#define CONFIG_ATCMD_MP				1 //support MP AT command
#endif
#define USE_MODE                    1 //for test
#endif

/**
 * For interactive mode configurations, depends on log service
 */
#if SUPPORT_INTERACTIVE_MODE
#define CONFIG_INTERACTIVE_MODE		1
#define CONFIG_INTERACTIVE_EXT		0
#else
#define CONFIG_INTERACTIVE_MODE		0
#define CONFIG_INTERACTIVE_EXT		0
#endif

/**
 * For FreeRTOS tickless configurations
 */
#define FREERTOS_PMU_TICKLESS_PLL_RESERVED   0   // In sleep mode, 0: close PLL clock, 1: reserve PLL clock
#define FREERTOS_PMU_TICKLESS_SUSPEND_SDRAM  1   // In sleep mode, 1: suspend SDRAM, 0: no act

/******************************************************************************/

/**
* For common flash usage
*/
#define AP_SETTING_SECTOR		0x000FE000
#define UART_SETTING_SECTOR		0x000FC000
#define FAST_RECONNECT_DATA 	(0x80000 - 0x1000)
#define FLASH_SECTOR_SIZE       0x1000

/**
 * For Wlan configurations
 */
#define CONFIG_WLAN		1
#if CONFIG_WLAN
#define CONFIG_LWIP_LAYER	1
#define CONFIG_INIT_NET		1 //init lwip layer when start up
#define CONFIG_WIFI_IND_USE_THREAD	0	// wifi indicate worker thread
#if (SUPPORT_MP_MODE == 0)
#define CONFIG_ENABLE_AP_POLLING_CLIENT_ALIVE 1 // on or off AP POLLING CLIENT
#endif

//on/off relative commands in log service
#define CONFIG_SSL_CLIENT	0
#define CONFIG_WEBSERVER	0
#define CONFIG_OTA_UPDATE	0
#define CONFIG_BSD_TCP		0//NOTE : Enable CONFIG_BSD_TCP will increase about 11KB code size
#define CONFIG_AIRKISS			0//on or off tencent airkiss
#define CONFIG_UART_SOCKET	0
#define CONFIG_AIRKISS_CLOUD	0//on or off for weixin hardware cloud
#define CONFIG_UART_YMODEM	0//support uart ymodem upgrade or not
#define CONFIG_TRANSPORT	0//on or off the at command for transport socket

/* For WPS and P2P */
#define CONFIG_ENABLE_WPS		0
#if CONFIG_ENABLE_WPS
#define CONFIG_ENABLE_WPS_DISCOVERY	1
#endif
#if CONFIG_ENABLE_P2P
#define CONFIG_ENABLE_WPS_AP		1
#undef CONFIG_WIFI_IND_USE_THREAD
#define CONFIG_WIFI_IND_USE_THREAD	1
#endif
#if (CONFIG_ENABLE_P2P && ((CONFIG_ENABLE_WPS_AP == 0) || (CONFIG_ENABLE_WPS == 0)))
#error "If CONFIG_ENABLE_P2P, need to define CONFIG_ENABLE_WPS_AP 1"
#endif

/* For SSL/TLS */
#define CONFIG_USE_POLARSSL     0
#define CONFIG_USE_MBEDTLS      1
#if ((CONFIG_USE_POLARSSL == 0) && (CONFIG_USE_MBEDTLS == 0)) || ((CONFIG_USE_POLARSSL == 1) && (CONFIG_USE_MBEDTLS == 1))
#undef CONFIG_USE_POLARSSL
#define CONFIG_USE_POLARSSL 1
#undef CONFIG_USE_MBEDTLS
#define CONFIG_USE_MBEDTLS 0
#endif

/* For Simple Link */
#define CONFIG_INCLUDE_SIMPLE_CONFIG		0

/*For fast reconnection*/
#define CONFIG_EXAMPLE_WLAN_FAST_CONNECT	0


#define CONFIG_GAGENT			0
/*Disable CONFIG_EXAMPLE_WLAN_FAST_CONNECT when CONFIG_GAGENT is enabled,because
	reconnect to previous AP is not suitable when re-configuration.
*/
#if CONFIG_GAGENT
#define CONFIG_EXAMPLE_WLAN_FAST_CONNECT 0
#endif


#endif //end of #if CONFIG_WLAN
/*******************************************************************************/

/**
 * For Ethernet configurations
 */
#define CONFIG_ETHERNET 0
#if CONFIG_ETHERNET

#define CONFIG_LWIP_LAYER	1
#define CONFIG_INIT_NET         1 //init lwip layer when start up

//on/off relative commands in log service
#define CONFIG_SSL_CLIENT	0
#define CONFIG_BSD_TCP		0//NOTE : Enable CONFIG_BSD_TCP will increase about 11KB code size

#endif


/**
 * For iNIC configurations
 */
#ifdef CONFIG_INIC //this flag is defined in IAR project
#define CONFIG_INIC_EN 1//enable iNIC mode
#undef CONFIG_ENABLE_WPS
#define CONFIG_ENABLE_WPS 1
#undef CONFIG_INCLUDE_SIMPLE_CONFIG
#define CONFIG_INCLUDE_SIMPLE_CONFIG	1
#undef CONFIG_WOWLAN_SERVICE
#define CONFIG_WOWLAN_SERVICE 1
#undef LOG_SERVICE_BUFLEN
#define LOG_SERVICE_BUFLEN 256
#undef CONFIG_LWIP_LAYER
#define CONFIG_LWIP_LAYER	0
#undef CONFIG_OTA_UPDATE
#define CONFIG_OTA_UPDATE 0
#undef CONFIG_EXAMPLE_WLAN_FAST_CONNECT
#define CONFIG_EXAMPLE_WLAN_FAST_CONNECT 0
#define CONFIG_INIC_SDIO_HCI	1 //for SDIO or USB iNIC
#define CONFIG_INIC_USB_HCI	0
#define CONFIG_INIC_CMD_RSP     1 //need to return msg to host
#endif
/******************End of iNIC configurations*******************/


/* For WIFI GET BEACON FRAME example */
#define CONFIG_EXAMPLE_GET_BEACON_FRAME  0

/* For WIFI MAC MONITOR example */
#define CONFIG_EXAMPLE_WIFI_MAC_MONITOR  0

/* For HTTP CLIENT example */
#define CONFIG_EXAMPLE_HTTP_CLIENT  0

/* For MQTT example */
#define CONFIG_EXAMPLE_MQTT				0

/* For mDNS example */
#define CONFIG_EXAMPLE_MDNS				0

/* For multicast example */
#define CONFIG_EXAMPLE_MCAST			0

/* For XML example */
#define CONFIG_EXAMPLE_XML				0

/* For socket select example */
#define CONFIG_EXAMPLE_SOCKET_SELECT	0

/* For socket nonblocking connect example */
#define CONFIG_EXAMPLE_NONBLOCK_CONNECT	0

/* For socket TCP bidirectional transmission example */
#define CONFIG_EXAMPLE_SOCKET_TCP_TRX	0

/* For ssl download example */
#define CONFIG_EXAMPLE_SSL_DOWNLOAD      0

/* For http download example */
#define CONFIG_EXAMPLE_HTTP_DOWNLOAD	0

/* For httpc example */
#define CONFIG_EXAMPLE_HTTPC			0

/* For httpd example */
#define CONFIG_EXAMPLE_HTTPD			0

/* For tcp keepalive example */
#define CONFIG_EXAMPLE_TCP_KEEPALIVE	0

/* For sntp show time example */
#define CONFIG_EXAMPLE_SNTP_SHOWTIME	0


/* For websocket client example */
#define CONFIG_EXAMPLE_WEBSOCKET		0


/* For UART Module AT command example */
#define CONFIG_EXAMPLE_UART_ATCMD	0
#if CONFIG_EXAMPLE_UART_ATCMD
#undef FREERTOS_PMU_TICKLESS_PLL_RESERVED
#define FREERTOS_PMU_TICKLESS_PLL_RESERVED  1
#undef CONFIG_OTA_UPDATE
#define CONFIG_OTA_UPDATE 1
#undef CONFIG_TRANSPORT
#define CONFIG_TRANSPORT 1
#undef LOG_SERVICE_BUFLEN
#define LOG_SERVICE_BUFLEN 1600
#undef CONFIG_LOG_SERVICE_LOCK
#define CONFIG_LOG_SERVICE_LOCK 1
#undef CONFIG_EXAMPLE_WLAN_FAST_CONNECT
#define CONFIG_EXAMPLE_WLAN_FAST_CONNECT  0
#endif


/* For DCT example*/
#define CONFIG_EXAMPLE_DCT				0

/****************** For EAP method example *******************/
#define CONFIG_EXAMPLE_EAP	0

// on/off specified eap method
#define CONFIG_ENABLE_PEAP	0
#define CONFIG_ENABLE_TLS	0
#define CONFIG_ENABLE_TTLS	0

// optional feature: whether to verify the cert of radius server
#define ENABLE_EAP_SSL_VERIFY_SERVER	0

#if CONFIG_ENABLE_PEAP || CONFIG_ENABLE_TLS || CONFIG_ENABLE_TTLS
#define CONFIG_ENABLE_EAP
#define CONFIG_EXAMPLE_WLAN_FAST_CONNECT 0
#endif

#if CONFIG_ENABLE_TLS
#define ENABLE_EAP_SSL_VERIFY_CLIENT	1
#else
#define ENABLE_EAP_SSL_VERIFY_CLIENT	0
#endif
/******************End of EAP configurations*******************/

/* For iNIC host example*/
#ifdef CONFIG_INIC_GSPI_HOST //this flag is defined in IAR project
#define	CONFIG_EXAMPLE_INIC_GSPI_HOST	1
#if CONFIG_EXAMPLE_INIC_GSPI_HOST

#define CONFIG_INIC_HOST	1

#undef CONFIG_WLAN
#define CONFIG_WLAN		0
#undef	CONFIG_INCLUDE_SIMPLE_CONFIG
#define CONFIG_INCLUDE_SIMPLE_CONFIG	0
#undef	CONFIG_EXAMPLE_WLAN_FAST_CONNECT
#define CONFIG_EXAMPLE_WLAN_FAST_CONNECT	0
#undef CONFIG_LWIP_LAYER
#define CONFIG_LWIP_LAYER	1
#undef	CONFIG_BSD_TCP
#define CONFIG_BSD_TCP		1

#endif
#endif

/*For uart update example*/
#define CONFIG_UART_UPDATE				0
#if CONFIG_UART_UPDATE
#undef CONFIG_EXAMPLE_WLAN_FAST_CONNECT
#define CONFIG_EXAMPLE_WLAN_FAST_CONNECT 0
#endif



/* For uart adapter example */
/* Please also configure LWIP_UART_ADAPTER to 1
in lwip_opt.h for support uart adapter*/
#define CONFIG_EXAMPLE_UART_ADAPTER	0
#if CONFIG_EXAMPLE_UART_ADAPTER
#undef CONFIG_EXAMPLE_WLAN_FAST_CONNECT
#define CONFIG_EXAMPLE_WLAN_FAST_CONNECT	1
#undef CONFIG_EXAMPLE_MDNS
#define CONFIG_EXAMPLE_MDNS	1
#undef FREERTOS_PMU_TICKLESS_PLL_RESERVED
#define FREERTOS_PMU_TICKLESS_PLL_RESERVED  1
#endif
#if CONFIG_JD_SMART
#if (CONFIG_ENABLE_WPS == 1)
#define CONFIG_ENABLE_WPS	0
#endif
#if (CONFIG_INCLUDE_SIMPLE_CONFIG == 1)
#define CONFIG_INCLUDE_SIMPLE_CONFIG	0
#endif
#if (CONFIG_EXAMPLE_WLAN_FAST_CONNECT == 1)
#define CONFIG_EXAMPLE_WLAN_FAST_CONNECT	0
#endif
#endif

/* For wifi scenarios example (Wi-Fi, WPS enrollee, P2P GO) */
// also need to enable WPS and P2P
#define CONFIG_EXAMPLE_WLAN_SCENARIO	0

/* For broadcast example */
#define CONFIG_EXAMPLE_BCAST			0

/* For high-load memory use case memory usage */
#define CONFIG_EXAMPLE_HIGH_LOAD_MEMORY_USE		0

/* For rarp example */
#define CONFIG_EXAMPLE_RARP				0

/* For ssl server example */
#define CONFIG_EXAMPLE_SSL_SERVER		0


/* For ota update http example */
#define CONFIG_EXAMPLE_OTA_HTTP			0


#if CONFIG_ENABLE_WPS
#define WPS_CONNECT_RETRY_COUNT		4
#define WPS_CONNECT_RETRY_INTERVAL	5000 // in ms
#endif

#define AUTO_RECONNECT_COUNT	8
#define AUTO_RECONNECT_INTERVAL	5 // in sec

/* First-bench firmware must not enable SDK flash-backed provisioning paths. */
#undef CONFIG_OTA_UPDATE
#define CONFIG_OTA_UPDATE 0
#undef CONFIG_INCLUDE_SIMPLE_CONFIG
#define CONFIG_INCLUDE_SIMPLE_CONFIG 0
#undef CONFIG_EXAMPLE_WLAN_FAST_CONNECT
#define CONFIG_EXAMPLE_WLAN_FAST_CONNECT 0
#undef CONFIG_JD_SMART
#define CONFIG_JD_SMART 0
#undef CONFIG_FAST_DHCP
#define CONFIG_FAST_DHCP 0

#endif
