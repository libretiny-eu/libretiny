/*
  main.cpp - Main loop for Arduino sketches
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

#define ARDUINO_MAIN
#include "Arduino.h"
#include "cmsis_os.h"

#ifdef __cplusplus
extern "C" {
#endif // __cplusplus

#include "rtl8710b.h"
#include "FreeRTOS.h"
#include "task.h"
#include "diag.h"
extern void HalCpuClkConfig(u8 CpuType);
extern void SystemCoreClockUpdate(void);
extern void En32KCalibration(void);
extern int tcm_heap_freeSpace(void);
extern void console_init(void);

#ifdef __cplusplus
} // extern "C"
#endif

osThreadId main_tid = 0;

// Weak empty variant initialization function.
// May be redefined by variant files.
void initVariant() __attribute__((weak));
void initVariant() { }

// Initialize C library
extern "C" void __libc_init_array(void);

/*
 * \brief Init Random()
 * \note Use in void __low_level_init(void) { Init_Rand(); } !
 */
void Init_Rand(void)
{
	extern u32 _rand_z1, _rand_z2, _rand_z3, _rand_z4, _rand_first;
	u32 *p = (u32 *)0x1FFFFF00;
	while(p < (u32 *)0x20000000) _rand_z1 ^= *p++;
	_rand_z1 ^= (*((u32 *)0x40002018) << 24) ^ (*((u32 *)0x40002118) << 16) ^ (*((u32 *)0x40002218) << 8) ^ *((u32 *)0x40002318);
	_rand_z2 = ((_rand_z1 & 0x007F00FF) << 7) ^	((_rand_z1 & 0x0F80FF00) >> 8);
	_rand_z3 = ((_rand_z2 & 0x007F00FF) << 7) ^	((_rand_z2 & 0x0F80FF00) >> 8);
	_rand_z4 = ((_rand_z3 & 0x007F00FF) << 7) ^	((_rand_z3 & 0x0F80FF00) >> 8);
	_rand_first = 1;
}

/*
 * \brief handle sketch
 */
void main_task( void const *arg )
{
    setup();

    for (;;)
    {
        loop();
        if (serialEventRun) serialEventRun();
        osThreadYield();
    }
}


/*
 * \brief Main entry point of Arduino application
 */
int main( void )
{
    init();

    __libc_init_array();

    initVariant();

    osThreadDef(main_task, osPriorityRealtime, 1, 4096*4); 
    main_tid = osThreadCreate(osThread (main_task), NULL);

    osKernelStart();

    while(1);

    return 0;
}
