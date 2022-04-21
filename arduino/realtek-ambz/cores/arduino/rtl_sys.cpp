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
#include <flash_api.h>

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

unsigned int GetFlashSize(void)
{
	unsigned int FlashSize;
	if(!fspic_isinit) flash_get_status(&flashobj);
	if(flashobj.SpicInitPara.id[3] >= 0x14 && flashobj.SpicInitPara.id[0] <= 0x19) {
		FlashSize = 1<<(flashobj.SpicInitPara.id[2]); // Flash size in bytes
	}
	else FlashSize = 1024*1024;  // 1 mbytes
	return FlashSize;
}

unsigned int GetFlashId(void)
{
	if(!fspic_isinit) flash_get_status(&flashobj);
	return (flashobj.SpicInitPara.id[0]<<16) | (flashobj.SpicInitPara.id[1]<<8) | flashobj.SpicInitPara.id[2];
}


void * malloc(size_t size)
{
	void * ret;
	if((ret = pvPortMalloc(size)) == NULL)
		ret = tcm_heap_malloc(size);
	return ret;
}

void * zalloc(size_t size) 
{
	void * ret;
	if((ret = pvPortMalloc(size)) == NULL)
		ret = tcm_heap_calloc(size);
	else memset(ret, 0, size);
	return ret;
}

void *calloc(size_t count, size_t size)
{
  return zalloc(count * size);
}

void free(void *pv)
{
	vPortFree(pv);
}

void * realloc(void *pv, size_t size)
{
	return pvPortReAlloc(pv, size);
}

void *operator new(size_t size)
{
	void * ret;
	if((ret = zalloc(size)) == NULL) {
		DiagPrintf("\r\nMEM error!\r\n");
		while(1);
	}
    return ret;
}

void *operator new[](size_t size)
{
	void * ret;
	if((ret = zalloc(size)) == NULL) {
		DiagPrintf("\r\nMEM error!\r\n");
		while(1);
	}
    return ret;
}

void operator delete(void * ptr)
{
    free(ptr);
}

void operator delete[](void * ptr)
{
    free(ptr);
}

/*
extern "C" void __cxa_pure_virtual(void) __attribute__ ((__noreturn__));
extern "C" void __cxa_deleted_virtual(void) __attribute__ ((__noreturn__));

void __cxa_pure_virtual(void)
{
    panic();
}

void __cxa_deleted_virtual(void)
{
    panic();
}

typedef struct {
    uint8_t guard;
    uint8_t ps;
} guard_t;

extern "C" int __cxa_guard_acquire(__guard* pg)
{
    uint8_t ps = xt_rsil(15);
    if (reinterpret_cast<guard_t*>(pg)->guard) {
        xt_wsr_ps(ps);
        return 0;
    }
    reinterpret_cast<guard_t*>(pg)->ps = ps;
    return 1;
}

extern "C" void __cxa_guard_release(__guard* pg)
{
    reinterpret_cast<guard_t*>(pg)->guard = 1;
    xt_wsr_ps(reinterpret_cast<guard_t*>(pg)->ps);
}

extern "C" void __cxa_guard_abort(__guard* pg)
{
    xt_wsr_ps(reinterpret_cast<guard_t*>(pg)->ps);
}


namespace std
{
void __throw_bad_function_call()
{
    panic();
}

void __throw_length_error(char const*)
{
    panic();
}

void __throw_bad_alloc()
{
    panic();
}

void __throw_logic_error(const char* str)
{
    panic();
}

void __throw_out_of_range(const char* str)
{
    panic();
}
}

*/    

