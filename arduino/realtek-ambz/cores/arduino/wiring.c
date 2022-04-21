/*
  Copyright (c) 2011 Arduino.  All right reserved.

  This library is free software; you can redistribute it and/or
  modify it under the terms of the GNU Lesser General Public
  License as published by the Free Software Foundation; either
  version 2.1 of the License, or (at your option) any later version.

  This library is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
  See the GNU Lesser General Public License for more details.

  You should have received a copy of the GNU Lesser General Public
  License along with this library; if not, write to the Free Software
  Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
*/

#include "Arduino.h"

#ifdef __cplusplus
extern "C" {
#endif

#include "cmsis_os.h"

#ifndef portNVIC_SYSTICK_CURRENT_VALUE_REG
#define portNVIC_SYSTICK_CURRENT_VALUE_REG	( * ( ( volatile uint32_t * ) 0xe000e018 ) )
#endif

extern uint32_t xTaskGetTickCount();
extern uint32_t xTaskGetTickCountFromISR();

static __inline uint32_t __get_ipsr__(void)
{
  volatile uint32_t __regIPSR          __asm("ipsr");
  return(__regIPSR);
}

void delay( uint32_t ms )
{
	osStatus ret;

	ret = osDelay(ms);
	if ( (ret != osEventTimeout) && (ret != osOK) ) {
		//printf("delay : ERROR : 0x%x \n", ret);
	}
}

void delayMicroseconds(uint32_t us)
{
    int i;
    uint32_t t0, tn;
    int dfactor = 0;

#if defined(BOARD_RTL8710)
//    dfactor = 10 * us - 10 + (40 * us / 100);
    dfactor = 20 * us - 10 + (81 * us / 100);
#elif defined(BOARD_RTL8195A)
    dfactor = 20 * us - 10 + (81 * us / 100);
#else
    dfactor = 20 * us - 10 + (81 * us / 100);
#endif

    if ( us > 100 ) {
        t0 = micros();
        do {
            tn = micros();
        } while ( tn >= t0 && tn < (t0 + us - 1) );
    } else {
        for (i=0; i<dfactor; i++) {
            asm("nop");
        }
    }
}

uint32_t millis( void )
{
    return (__get_ipsr__() == 0) ? xTaskGetTickCount() : xTaskGetTickCountFromISR();
}

uint32_t micros( void ) 
{
    uint32_t tick1, tick2;
    uint32_t us;
    uint32_t tick_per_us;

#if defined(BOARD_RTL8710)
    tick_per_us = 166666; // 83333;
#elif defined(BOARD_RTL8195A)
    tick_per_us = 166666;
#else
    tick_per_us = 166666;
#endif

    if (__get_ipsr__() == 0) {
        tick1 = xTaskGetTickCount();
        us = portNVIC_SYSTICK_CURRENT_VALUE_REG;
        tick2 = xTaskGetTickCount();
    } else {
        tick1 = xTaskGetTickCountFromISR();
        us = portNVIC_SYSTICK_CURRENT_VALUE_REG;
        tick2 = xTaskGetTickCountFromISR();
    }

    if (tick1 == tick2) {
        return tick1 * 1000 - us*1000 / tick_per_us;
    } else if( (us*1000 / tick_per_us) < 500 ) {
        return tick1 * 1000 - us*1000 / tick_per_us;
    } else {
        return tick1 * 1000;
    }
}

#ifdef __cplusplus
}
#endif
