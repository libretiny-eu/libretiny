/*
  Arduino.h - Main include file for the Arduino SDK
  Copyright (c) 2005-2013 Arduino Team.  All right reserved.

  This library is free software; you can redistribute it and/or
  modify it under the terms of the GNU Lesser General Public
  License as published by the Free Software Foundation; either
  version 2.1 of the License, or (at your option) any later version.

  This library is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
  Lesser General Public License for more details.

  You should have received a copy of the GNU Lesser General Public
  License along with this library; if not, write to the Free Software
  Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
*/

#ifndef Arduino_h
#define Arduino_h

#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

#include "binary.h"

#ifdef __cplusplus
extern "C" {
#endif // __cplusplus

#include "wiring_constants.h"

extern uint32_t SystemCoreClock;

#define clockCyclesPerMicrosecond() ( SystemCoreClock / 1000000L )
#define clockCyclesToMicroseconds(a) ( ((a) * 1000L) / (SystemCoreClock / 1000L) )
#define microsecondsToClockCycles(a) ( (a) * (SystemCoreClock / 1000000L) )

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

extern uint32_t DiagPrintf(const char *fmt, ...);

extern int rtl_printf(const char *fmt, ...);
extern int rtl_sprintf(char* str, const char* fmt, ...);

#ifndef printf
#define printf		rtl_printf
#endif
#ifndef sprintf
#define sprintf		rtl_sprintf
#endif

/*
extern void *pvPortMalloc( size_t xWantedSize );
extern void vPortFree( void *pv );
extern void *pvPortReAlloc( void *pv,  size_t xWantedSize );
extern size_t xPortGetFreeHeapSize( void );
extern size_t xPortGetMinimumEverFreeHeapSize( void );

extern void *tcm_heap_malloc(int size);
extern void *tcm_heap_calloc(int size);
extern void tcm_heap_free(void * mem);
extern void tcm_heap_dump(void);
extern int tcm_heap_freeSpace(void);

#ifndef malloc
#define malloc                  pvPortMalloc
#endif
#ifndef free
#define free                    vPortFree
#endif
#ifndef realloc
#define realloc                 pvPortReAlloc
#endif
*/

/* sketch */
extern void setup( void );
extern void loop( void );

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

#ifdef __cplusplus
} // extern "C"

#include "WCharacter.h"
#include "WString.h"
#include "WMath.h"
#include "HardwareSerial.h"
#include "wiring_pulse.h"

#endif // __cplusplus

// Include board variant
#include "variant.h"

#include "wiring.h"
#include "wiring_digital.h"
#include "wiring_analog.h"
#include "WInterrupts.h"
#include "wiring_os.h"
#include "wiring_watchdog.h"
#include "wiring_shift.h"

extern void * malloc(size_t size);
extern void * zalloc(size_t size);
extern void * calloc(size_t count, size_t size);
extern void free(void *pv);
extern void * realloc(void *pv, size_t size);
extern void hexdump(void * ptr, int cnt);
extern void debug_on(void);

extern unsigned int GetFlashSize(void); // default 1048576 bytes (1 Mbytes)
/* FlashID: manufacturer ID : memory type : memory density
  0xC22015 - MXIC MX25L1606E, F11AMIM13 (RTL8711AM) module, 
  0xC22014 - MXIC MX25L8006E, RTL00 (RTL8710AF) module */
extern unsigned int GetFlashId(void); 

// C++ functions
#ifdef __cplusplus

// WMath prototypes
extern long random( long ) ;
extern long random( long, long ) ;
extern void randomSeed( uint32_t dwSeed ) ;
extern long map( long, long, long, long, long ) ;

void tone(uint32_t ulPin, unsigned int frequency, unsigned long duration = 0);

#endif

//
// custom defined
//

//#include "Thread.h"
//#include "DAC1.h"


#endif // Arduino_h
