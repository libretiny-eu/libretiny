#pragma once

#include "wiring_os.h"
#include "wiring_watchdog.h"

#ifdef __cplusplus
extern "C" {
#endif

#include <stddef.h>
#include <stdint.h>
#include <stdarg.h>
#define boolean boolean_rtl
#include "rtl_lib.h"
#undef boolean

/* moved from Arduino.h */
/*
 * \brief Set CPU CLK 166MHz
 * clk : 0 - 166666666 Hz, 1 - 83333333 Hz, 2 - 41666666 Hz, 3 - 20833333 Hz, 4 - 10416666 Hz, 5 - 4000000 Hz
 * baud: 38400,...
 */
extern void Init_CPU_CLK_UART(int clkn, int baud);
extern void sys_info(void);
/* HalGetChipId:
 * 0xff - RTL8711AM, 0xfe - RTL8195AM, 0xfd - RTL8711AF, 
 * 0xfc - RTL8710AF, 0xfb - RTL8711AN, 0xfa - RTL8710AM */
extern unsigned char HalGetChipId(void); 
extern unsigned int HalGetCpuClk(void);

extern void wait_us(int us);
#define delay_us	wait_us

extern void yield(void);

#ifndef printf
#define printf		rtl_printf
#endif
#ifndef sprintf
#define sprintf		rtl_sprintf
#endif

#define NOT_INITIAL  (1UL<<0)
#define PIO_GPIO     (1UL<<1)
#define PIO_PWM	     (1UL<<2)
#define PIO_I2C	     (1UL<<3)
#define PIO_ADC      (1UL<<4)
#define PIO_DAC      (1UL<<5)
#define PIO_GPIO_IRQ (1UL<<6)

#define PWM_MODE_ENABLED  1
#define PWM_MODE_DISABLED 0

/* Types used for the tables below */
typedef struct _PinDescription
{
  // HW PinNames
  uint32_t 	pinname;
  // Current Pin Type
  uint32_t 	ulPinType;
  // Supported Pin Function
  uint32_t  ulPinAttribute;
  // Current Pin Mode
  uint32_t  ulPinMode;
} PinDescription ;

/* Pins table to be instanciated into variant.cpp */
extern PinDescription g_APinDescription[];

extern bool pinInvalid(pin_size_t pinNumber);
extern void pinRemoveMode(pin_size_t pinNumber);

/* moved from wiring_digital.h */
/**************************** Extend API by RTK ***********************************/
extern uint32_t digitalPinToPort( uint32_t pinNumber );
extern uint32_t digitalPinToBitMask( uint32_t pinNumber );

/* moved from wiring_analog.h */
/*
 * \brief Set the resolution of analogRead return values. Default is 10 bits (range from 0 to 1023).
 *
 * \param res
 */
extern void analogReadResolution(int res);
/*
 * \brief Set the resolution of analogWrite parameters. Default is 8 bits (range from 0 to 255).
 *
 * \param res
 */
extern void analogWriteResolution(int res);
extern void analogOutputInit( void ) ;
extern void analogWritePeriod(int us);

/* moved from wiring_constants.h */
#define DEFAULT 1
#define EXTERNAL 0

#define round(x)     ((x)>=0?(long)((x)+0.5):(long)((x)-0.5))

extern uint32_t ulPortSetInterruptMask( void );
extern void vPortClearInterruptMask( uint32_t ulNewMask );

#ifdef __cplusplus
} // extern "C"
#endif

extern unsigned int GetFlashSize(void); // default 1048576 bytes (1 Mbytes)
/* FlashID: manufacturer ID : memory type : memory density
  0xC22015 - MXIC MX25L1606E, F11AMIM13 (RTL8711AM) module,
  0xC22014 - MXIC MX25L8006E, RTL00 (RTL8710AF) module */
extern unsigned int GetFlashId(void);
