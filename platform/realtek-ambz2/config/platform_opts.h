/**
 ******************************************************************************
 *This file contains general configurations for ameba platform
 ******************************************************************************
 */

#ifndef __PLATFORM_OPTS_H__
#define __PLATFORM_OPTS_H__

/*For MP mode setting*/
#define SUPPORT_MP_MODE 1

/**
 * For AT cmd Log service configurations
 */
#define SUPPORT_LOG_SERVICE 1
#if SUPPORT_LOG_SERVICE
#define LOG_SERVICE_BUFLEN 100 // can't larger than UART_LOG_CMD_BUFLEN(127)
#define CONFIG_LOG_HISTORY 0
#if CONFIG_LOG_HISTORY
#define LOG_HISTORY_LEN 5
#endif
#define SUPPORT_INTERACTIVE_MODE 0 // on/off wifi_interactive_mode
#define CONFIG_LOG_SERVICE_LOCK	 0
#define CONFIG_ATCMD_MP			 0 // support MP AT command
#define USE_MODE				 1 // for test
#endif

/**
 * For interactive mode configurations, depends on log service
 */
#if defined(SUPPORT_INTERACTIVE_MODE) && SUPPORT_INTERACTIVE_MODE
#define CONFIG_INTERACTIVE_MODE 1
#define CONFIG_INTERACTIVE_EXT	0
#else
#define CONFIG_INTERACTIVE_MODE 0
#define CONFIG_INTERACTIVE_EXT	0
#endif

/**
 * For FreeRTOS tickless configurations
 */
#define FREERTOS_PMU_TICKLESS_PLL_RESERVED	0 // In sleep mode, 0: close PLL clock, 1: reserve PLL clock
#define FREERTOS_PMU_TICKLESS_SUSPEND_SDRAM 1 // In sleep mode, 1: suspend SDRAM, 0: no act

/******************************************************************************/

/**
 * For common flash usage
 */
#define FLASH_BAKEUP_SECTOR (0x3000)
#define FAST_RECONNECT_DATA (0x200000 - 0x1000)
#define BT_FTL_PHY_ADDR0	(0x200000 - 0x2000)
#define BT_FTL_PHY_ADDR1	(0x200000 - 0x3000)
#define BT_FTL_BKUP_ADDR	(0x200000 - 0x4000)
#define UART_SETTING_SECTOR (0x200000 - 0x5000)
#define DCT_BEGIN_ADDR                                                                                                 \
	(0x200000 - 0x29000) /*!< DCT begin address of flash, ex: 0x200000 = 2M, the default size of DCT is 24K; ; if      \
							backup enabled, the size is 48k; if wear leveling enabled, the size is 144k*/
#define FLASH_APP_BASE                                                                                                 \
	(0x200000 - 0xA9000                                                                                                \
	) /*!< FATFS begin address, default size used is 512KB (can be adjusted based on user requirement)*/
#define BT_WHITELIST_BASE_1	   (0x200000 - 0xA000)
#define BT_WHITELIST_PAGE_SIZE (0x1000)
#define BT_WHITELIST_BASE_2	   (BT_WHITELIST_BASE_1 + BT_WHITELIST_PAGE_SIZE)
/**
 * For Wlan configurations
 */
#define CONFIG_WLAN			   1
#if CONFIG_WLAN
#define CONFIG_LWIP_LAYER					  1
#define CONFIG_INIT_NET						  1 // init lwip layer when start up
#define CONFIG_WIFI_IND_USE_THREAD			  0 // wifi indicate worker thread
#define CONFIG_ENABLE_AP_POLLING_CLIENT_ALIVE 0 // on or off AP POLLING CLIENT

// on/off relative commands in log service
#define CONFIG_SSL_CLIENT	 0
#define CONFIG_OTA_UPDATE	 1
#define CONFIG_BSD_TCP		 0 // NOTE : Enable CONFIG_BSD_TCP will increase about 11KB code size
#define CONFIG_AIRKISS		 0 // on or off tencent airkiss
#define CONFIG_UART_SOCKET	 0
#define CONFIG_JD_SMART		 0 // on or off for jdsmart
#define CONFIG_JOYLINK		 0 // on or off for jdsmart2.0
#define CONFIG_QQ_LINK		 0 // on or off for qqlink
#define CONFIG_AIRKISS_CLOUD 0 // on or off for weixin hardware cloud
#define CONFIG_UART_YMODEM	 0 // support uart ymodem upgrade or not
#define CONFIG_TRANSPORT	 0 // on or off the at command for transport socket
#define CONFIG_ALINK		 0 // on or off for alibaba alink
#define CONFIG_MIIO			 0 // on or off for alibaba alink
#define CONFIG_RIC			 0 // on or off for RICloud
#define CONFIG_LINKKIT_AWSS	 0 // on or off for ali feiyan cloud

/* For WPS and P2P */
#define CONFIG_ENABLE_WPS 0
#define CONFIG_ENABLE_P2P 0
#if CONFIG_ENABLE_WPS
#define CONFIG_ENABLE_WPS_DISCOVERY 1
#endif
#if CONFIG_ENABLE_P2P
#define CONFIG_ENABLE_WPS_AP 1
#undef CONFIG_WIFI_IND_USE_THREAD
#define CONFIG_WIFI_IND_USE_THREAD 1
#endif
#if (CONFIG_ENABLE_P2P && ((CONFIG_ENABLE_WPS_AP == 0) || (CONFIG_ENABLE_WPS == 0)))
#error "If CONFIG_ENABLE_P2P, need to define CONFIG_ENABLE_WPS_AP 1"
#endif

/* For Simple Link */
#define CONFIG_INCLUDE_SIMPLE_CONFIG 1
#define CONFIG_INCLUDE_DPP_CONFIG	 0
/*For fast reconnection*/
#ifdef PLATFORM_OHOS
#define CONFIG_EXAMPLE_WLAN_FAST_CONNECT 0
#else
#define CONFIG_EXAMPLE_WLAN_FAST_CONNECT 1
#endif
#if CONFIG_EXAMPLE_WLAN_FAST_CONNECT
#define CONFIG_FAST_DHCP 1
#else
#define CONFIG_FAST_DHCP 0
#endif

/*For wowlan service settings*/
#define CONFIG_WOWLAN_SERVICE 0

#define CONFIG_GAGENT 0
/*Disable CONFIG_EXAMPLE_WLAN_FAST_CONNECT when CONFIG_GAGENT is enabled,because
	reconnect to previous AP is not suitable when re-configuration.
*/
#if CONFIG_GAGENT
#define CONFIG_EXAMPLE_WLAN_FAST_CONNECT 0
#endif

#endif // end of #if CONFIG_WLAN
/*******************************************************************************/

/* For SSL/TLS */
#define CONFIG_USE_POLARSSL 0 // polarssl is no longer suppported for AmebaZ2
#define CONFIG_USE_MBEDTLS	1
#if ((CONFIG_USE_POLARSSL == 0) && (CONFIG_USE_MBEDTLS == 0)) ||                                                       \
	((CONFIG_USE_POLARSSL == 1) && (CONFIG_USE_MBEDTLS == 1))
#undef CONFIG_USE_POLARSSL
#define CONFIG_USE_POLARSSL 1
#undef CONFIG_USE_MBEDTLS
#define CONFIG_USE_MBEDTLS 0
#endif
#define CONFIG_SSL_CLIENT_PRIVATE_IN_TZ 1

/* For LWIP configuration */
#define CONFIG_LWIP_DHCP_COARSE_TIMER 60

/*Enable CONFIG_LWIP_DHCP_FINE_TIMEOUT if lease time is less than or equal to CONFIG_LWIP_DHCP_COARSE_TIMER
 * replace dhcp_coarse_tmr with dhcp_fine_tmr to manage and check for lease timeout
 */
#define CONFIG_LWIP_DHCP_FINE_TIMEOUT 0

/**
 * For Ethernet configurations
 */
#define CONFIG_ETHERNET 0
#if CONFIG_ETHERNET

#define CONFIG_LWIP_LAYER 1
#define CONFIG_INIT_NET	  1 // init lwip layer when start up

// on/off relative commands in log service
#define CONFIG_SSL_CLIENT 0
#define CONFIG_BSD_TCP	  0 // NOTE : Enable CONFIG_BSD_TCP will increase about 11KB code size

#endif

/* For Azure Examples */
#define CONFIG_USE_AZURE_EMBEDDED_C 1
#if CONFIG_USE_AZURE_EMBEDDED_C
/* For Azure embedded iot examples*/
#define CONFIG_EXAMPLE_AZURE 0
#if CONFIG_EXAMPLE_AZURE
#undef WAIT_FOR_ACK
#define WAIT_FOR_ACK
#endif
#else
/* For Azure iot hub telemetry example*/
#define CONFIG_EXAMPLE_AZURE_IOTHUB_TELEMETRY 0
/* For Azure iot hub x509 example*/
#define CONFIG_EXAMPLE_AZURE_IOTHUB_X509	  0
#endif

/* for CoAP example*/
#define CONFIG_EXAMPLE_COAP 0

/* for lib CoAP example*/
#define CONFIG_EXAMPLE_COAP_SERVER 0
#define CONFIG_EXAMPLE_COAP_CLIENT 0

/* For WIFI GET BEACON FRAME example */
#define CONFIG_EXAMPLE_GET_BEACON_FRAME 0

/* For WIFI MAC MONITOR example */
#define CONFIG_EXAMPLE_WIFI_MAC_MONITOR 0

/* For HTTP CLIENT example */
#define CONFIG_EXAMPLE_HTTP_CLIENT 0

/* For MQTT example */
#define CONFIG_EXAMPLE_MQTT 0

/* For multicast example */
#define CONFIG_EXAMPLE_MCAST 0

/* For XML example */
#define CONFIG_EXAMPLE_XML 0

/* For JSON example */
#define CONFIG_EXAMPLE_CJSON 0

/* For socket select example */
#define CONFIG_EXAMPLE_SOCKET_SELECT 0

/* For socket nonblocking connect example */
#define CONFIG_EXAMPLE_NONBLOCK_CONNECT 0

/* For socket TCP bidirectional transmission example */
#define CONFIG_EXAMPLE_SOCKET_TCP_TRX 0

/* For ssl download example */
#define CONFIG_EXAMPLE_SSL_DOWNLOAD 0

/* For http download example */
#define CONFIG_EXAMPLE_HTTP_DOWNLOAD 0

/* For httpc example */
#define CONFIG_EXAMPLE_HTTPC 0

/* For Amazon ffs example */
#define CONFIG_EXAMPLE_FFS 0
#if CONFIG_EXAMPLE_FFS
#undef CONFIG_EXAMPLE_WLAN_FAST_CONNECT
#define CONFIG_EXAMPLE_WLAN_FAST_CONNECT 0
#undef CONFIG_FAST_DHCP
#define CONFIG_FAST_DHCP 0
#endif

/* For httpd example */
#define CONFIG_EXAMPLE_HTTPD 0

/* For tcp keepalive example */
#define CONFIG_EXAMPLE_TCP_KEEPALIVE 0

/* For sntp show time example */
#define CONFIG_EXAMPLE_SNTP_SHOWTIME 0

/* For websocket client example */
#define CONFIG_EXAMPLE_WEBSOCKET_CLIENT 0

/* For DCT example*/
#define CONFIG_EXAMPLE_DCT 0

/****************** For EAP method example *******************/
#define CONFIG_EXAMPLE_EAP 0

// on/off specified eap method
#define CONFIG_ENABLE_PEAP 0
#define CONFIG_ENABLE_TLS  0
#define CONFIG_ENABLE_TTLS 0

// optional feature: whether to verify the cert of radius server
#define ENABLE_EAP_SSL_VERIFY_SERVER 0

#if CONFIG_ENABLE_PEAP || CONFIG_ENABLE_TLS || CONFIG_ENABLE_TTLS
#define CONFIG_ENABLE_EAP
#undef CONFIG_EXAMPLE_WLAN_FAST_CONNECT
#undef CONFIG_FAST_DHCP
#define CONFIG_EXAMPLE_WLAN_FAST_CONNECT 0
#define CONFIG_FAST_DHCP				 0
#endif

#if CONFIG_ENABLE_TLS
#define ENABLE_EAP_SSL_VERIFY_CLIENT 1
#else
#define ENABLE_EAP_SSL_VERIFY_CLIENT 0
#endif
/******************End of EAP configurations*******************/

/* For FATFS example*/
#define CONFIG_EXAMPLE_FATFS 0
#if CONFIG_EXAMPLE_FATFS
#define CONFIG_FATFS_EN 1
#if CONFIG_FATFS_EN
// fatfs version
#define FATFS_R_10C
// fatfs disk interface
#define FATFS_DISK_USB	 0
#define FATFS_DISK_SD	 0
#define FATFS_DISK_FLASH 1
#endif
#endif

/* For iNIC host example*/
#ifdef CONFIG_INIC_GSPI_HOST // this flag is defined in IAR project
#define CONFIG_EXAMPLE_INIC_GSPI_HOST 1
#if CONFIG_EXAMPLE_INIC_GSPI_HOST

#define CONFIG_INIC_HOST 1

#undef CONFIG_WLAN
#define CONFIG_WLAN 0
#undef CONFIG_INCLUDE_SIMPLE_CONFIG
#define CONFIG_INCLUDE_SIMPLE_CONFIG 0
#undef CONFIG_EXAMPLE_WLAN_FAST_CONNECT
#define CONFIG_EXAMPLE_WLAN_FAST_CONNECT 0
#undef CONFIG_LWIP_LAYER
#define CONFIG_LWIP_LAYER 1
#undef CONFIG_BSD_TCP
#define CONFIG_BSD_TCP 1

#endif
#endif

/* For wifi scenarios example (Wi-Fi, WPS enrollee, P2P GO) */
// also need to enable WPS and P2P
#define CONFIG_EXAMPLE_WLAN_SCENARIO 0

/* For broadcast example */
#define CONFIG_EXAMPLE_BCAST 0

/* For high-load memory use case memory usage */
#define CONFIG_EXAMPLE_HIGH_LOAD_MEMORY_USE 0

/* For rarp example */
#define CONFIG_EXAMPLE_RARP 0

/* For ssl server example */
#define CONFIG_EXAMPLE_SSL_SERVER 0

/*For secure boot example */
#define CONFIG_EXAMPLE_SECURE_BOOT 0

/*For secure storage example */
#define CONFIG_EXAMPLE_SECURE_STORAGE 0

/* For ota update http example */
#define CONFIG_EXAMPLE_OTA_HTTP 0

/*For wifi roaming example*/
#define CONFIG_EXAMPLE_WIFI_ROAMING 0

/*  for tickless roaming    */
#define CONFIG_EXAMPLE_TICKLESS_WIFI_ROAMING 0

/*For ipv6 example*/
#define CONFIG_EXAMPLE_IPV6 0

#if CONFIG_ENABLE_WPS
#define WPS_CONNECT_RETRY_COUNT	   4
#define WPS_CONNECT_RETRY_INTERVAL 5000 // in ms
#endif

#define AUTO_RECONNECT_COUNT	8
#define AUTO_RECONNECT_INTERVAL 5 // in sec

/*For trust zone example */
#define CONFIG_EXAMPLE_TRUST_ZONE 0

/* For Amazon FreeRTOS SDK example */
#define CONFIG_EXAMPLE_AMAZON_FREERTOS 0

#define CONFIG_EXAMPLE_AMAZON_AFQP_TESTS 0

#if (defined(CONFIG_EXAMPLE_AMAZON_FREERTOS) && CONFIG_EXAMPLE_AMAZON_FREERTOS) ||                                     \
	(defined(CONFIG_EXAMPLE_AMAZON_AFQP_TESTS) && CONFIG_EXAMPLE_AMAZON_AFQP_TESTS)
#undef CONFIG_INCLUDE_SIMPLE_CONFIG
#undef CONFIG_EXAMPLE_WLAN_FAST_CONNECT
#undef CONFIG_FAST_DHCP
#define CONFIG_INCLUDE_SIMPLE_CONFIG	 0
#define CONFIG_EXAMPLE_WLAN_FAST_CONNECT 0
#define CONFIG_FAST_DHCP				 0
#if defined(CONFIG_BUILD_SECURE)
#define ENABLE_AMAZON_COMMON
#endif
#if defined(CONFIG_BUILD_NONSECURE) && (CONFIG_SSL_CLIENT_PRIVATE_IN_TZ == 0)
#undef SUPPORT_LOG_SERVICE
#define SUPPORT_LOG_SERVICE 0
#endif
#endif

/* For UART Module AT command example */
#define CONFIG_EXAMPLE_UART_ATCMD 0
#if (defined(CONFIG_EXAMPLE_UART_ATCMD) && CONFIG_EXAMPLE_UART_ATCMD)
#define UART_SETTING_BACKUP_SECTOR (FLASH_BAKEUP_SECTOR)
#undef CONFIG_OTA_UPDATE
#define CONFIG_OTA_UPDATE 1
#undef CONFIG_TRANSPORT
#define CONFIG_TRANSPORT 1
#undef LOG_SERVICE_BUFLEN
#define LOG_SERVICE_BUFLEN 1600
#undef CONFIG_LOG_SERVICE_LOCK
#define CONFIG_LOG_SERVICE_LOCK 1
#undef CONFIG_EXAMPLE_WLAN_FAST_CONNECT
#define CONFIG_EXAMPLE_WLAN_FAST_CONNECT 0
#endif

#if (SUPPORT_MP_MODE && CONFIG_MIIO)
#define CONFIG_MIIO_MP 0 // miio mp test and rw private data
#endif

#endif
