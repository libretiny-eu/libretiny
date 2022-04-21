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
#endif
extern void HalCpuClkConfig(u8  CpuType);
extern void SystemCoreClockUpdate(void);
extern void En32KCalibration(void);

#include "FreeRTOS.h"
#include "task.h"
#include "diag.h"
#include "rtl8195a.h"
extern int tcm_heap_freeSpace(void);
extern void console_init(void);

osThreadId main_tid = 0;

// Weak empty variant initialization function.
// May be redefined by variant files.
void initVariant() __attribute__((weak));
void initVariant() { }

/*
 * \brief User PreInit of Arduino application
 */
void UserPreInit(void) __attribute__((weak));
void UserPreInit(void) { }


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
 * \brief Set CPU CLK 166MHz
 * clk : 0 - 166666666 Hz, 1 - 83333333 Hz, 2 - 41666666 Hz, 3 - 20833333 Hz, 4 - 10416666 Hz, 5 - 4000000 Hz
 *       6 - 200000000 Hz, 7 - 10000000 Hz, 8 - 50000000 Hz, 9 - 25000000 Hz, 10 - 12500000 Hz, 11 - 4000000 Hz
 * baud: 38400,...
 */
void Init_CPU_CLK_UART(int clkn, int baud)
{
//	if(HalGetCpuClk() < 166000000)
		if(clkn > 5) {
	 		HalCpuClkConfig(clkn - 6);
	 		*((int *)0x40000074) |= (1<<17);
		}
		else
		{
	 		HalCpuClkConfig(clkn);
	 		*((int *)0x40000074) &= (~(1<<17));
		}
		HAL_LOG_UART_ADAPTER pUartAdapter;
		pUartAdapter.BaudRate = baud;
		HalLogUartSetBaudRate(&pUartAdapter);
		SystemCoreClockUpdate();
		En32KCalibration();
}
/*
 * \brief handle sketch
 */
void main_task( void const *arg )
{

    setup();

/*    
	 ConfigDebugErr  = -1;
	 ConfigDebugInfo = ~_DBG_SPI_FLASH_;
	 ConfigDebugWarn = -1;
	 CfgSysDebugErr = -1;
	 CfgSysDebugInfo = -1;
	 CfgSysDebugWarn = -1;
*/
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
	UserPreInit();
	
    init();

    initVariant();

#if 0 // pvvx: add start info
    vPortFree(pvPortMalloc(4)); // Init RAM heap
	sys_info();
#endif 

    osThreadDef(main_task, osPriorityRealtime, 1, 4096*4); 
    main_tid = osThreadCreate(osThread (main_task), NULL);

    osKernelStart();

    while(1);

    return 0;
}

#ifdef __cplusplus
}
#endif

