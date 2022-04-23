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

/* moved from wiring_digital.h */
extern void digitalChangeDir( uint32_t ulPin, uint8_t direction);
/**************************** Extend API by RTK ***********************************/
extern uint32_t digitalPinToPort( uint32_t ulPin );
extern uint32_t digitalPinToBitMask( uint32_t ulPin );
extern uint32_t digitalSetIrqHandler( uint32_t ulPin, void (*handler)(uint32_t id, uint32_t event) );
extern uint32_t digitalClearIrqHandler( uint32_t ulPin );
extern void pinRemoveMode(uint32_t ulPin);

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
// TODO use these correctly in wiring
// #define INPUT_PULLDOWN 				INPUT
// #define INPUT_PULLNONE 		0x03
#define INPUT_IRQ_FALL      0x05
#define INPUT_IRQ_RISE      0x06
#define INPUT_IRQ_LOW       0x07
#define INPUT_IRQ_HIGH      0x08

#define DEFAULT 1
#define EXTERNAL 0

// undefine stdlib's abs if encountered
/* #ifdef abs
#undef abs
#endif // abs

#define abs(x) ((x)>0?(x):-(x)) */
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
