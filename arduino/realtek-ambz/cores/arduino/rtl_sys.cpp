/*
 Copyright (c) 2014 Arduino.  All right reserved.

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

#include <stdlib.h>
#include <assert.h>
#include <debug.h>
#include <Arduino.h>


#ifdef __cplusplus
extern "C" {
#endif // __cplusplus

#include <diag.h>

extern void *pvPortMalloc( size_t xWantedSize );
extern size_t xPortGetFreeHeapSize( void );
extern size_t xPortGetMinimumEverFreeHeapSize( void );

extern void *tcm_heap_malloc(int size);
extern void *tcm_heap_calloc(int size);
extern void tcm_heap_free(void * mem);
extern void tcm_heap_dump(void);
extern int tcm_heap_freeSpace(void);


__attribute__((noreturn)) void __panic_func(const char* file, int line, const char* func)
{
	DiagPrintf("\r\nPanic: %s, line: %d, %s\r\n");
	while(1);
}

#ifdef __cplusplus
} // extern "C"
#endif

void hexdump(void * ptr, int cnt)
{
	char * p = (char *) ptr;
	int c = cnt;
	while(c > 64) {
		DumpForOneBytes((void *)p, 64);
		p += 64;
		c -= 64;
	}
	if(c != 0) DumpForOneBytes((void *)p, c);
}

void debug_on(void)
{
	 ConfigDebugErr  = -1;
	 ConfigDebugInfo = -1;
	 ConfigDebugWarn = -1;
	 CfgSysDebugErr = -1;
	 CfgSysDebugInfo = -1;
	 CfgSysDebugWarn = -1;
}

void sys_info(void) {
	rtl_printf("\r\nCLK CPU\t\t%d Hz\r\nRAM heap\t%d bytes\r\nTCM heap\t%d bytes\r\n",
				HalGetCpuClk(), xPortGetFreeHeapSize(), tcm_heap_freeSpace());
}

void *pvPortZalloc(size_t size) {
    void *pvReturn = pvPortMalloc(size);
	if (pvReturn)
		memset(pvReturn, 0, size);
	return pvReturn;
}

void *pvPortCalloc(size_t nmemb, size_t size) {
    return pvPortZalloc(nmemb * size);
}
