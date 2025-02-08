#ifndef _PROJ_CONFIG_H_
#define _PROJ_CONFIG_H_

#define DISABLE (0)
#define ENABLE	(1)

#define __CONFIG_OS_KERNEL RTOS_FREERTOS

/*
 * Clock settings section
 * Note:
 *
 */
#define XTAL_CLOCK (40000000)
#define RCO_CLOCK  (32000)
#define PLL_CLOCK  (160000000)

// TODO: SystemCoreClock
#define SYSTEM_CLOCK (160000000)

/*
 * Module enable/disable control
 */
#define FLASH_XIP	  ENABLE
#define LN_ASSERT_EN  ENABLE
#define HAL_ASSERT_EN ENABLE
#define PRINTF_OMIT	  DISABLE // when release software, set 1 to omit all printf logs
#define OS_TICK_COMPENSATE

// Check big and endian mode
#if defined(__CC_ARM)
#if defined(__BIG_ENDIAN)
#error "Please set the compiler to little-endian mode"
#endif
#elif defined(__GNUC__)
#if (__BYTE_ORDER__ != __ORDER_LITTLE_ENDIAN__)
#error "Please set the compiler to little-endian mode"
#endif // __BYTE_ORDER__
#else
#error "Unsupported compiler"
#endif

#ifndef LITTLE_ENDIAN
#define LITTLE_ENDIAN 1234
#endif

/*
 * flash image settings
 */
#define FLASH_IMAGE_VER_MAJOR 0
#define FLASH_IMAGE_VER_MINOR 1
#define SOC_CRP_FLAG		  0

/*
 * Hardware config
 */
#define CFG_UART0_TX_BUF_SIZE 256
#define CFG_UART0_RX_BUF_SIZE 256
#define CFG_UART1_TX_BUF_SIZE 128
#define CFG_UART1_RX_BUF_SIZE 256
#define CFG_UART2_TX_BUF_SIZE 128
#define CFG_UART2_RX_BUF_SIZE 256

#define CFG_UART_BAUDRATE_LOG	  115200
#define CFG_UART_BAUDRATE_CONSOLE 115200

/*
 * FreeRTOS config
 */

#define configTOTAL_HEAP_SIZE ((size_t)(160 * 1024))

/*
 * log
 */
#ifndef __cplusplus
#define hexdump ln_hexdump
#endif // __cplusplus

#define mbedtls_platform_set_calloc_free(calloc, free)

#endif /* _PROJ_CONFIG_H_ */
