#include <stdint.h>

// for some reason, cmsis_os.c does not link properly when this method is inlined in core_cmFunc.h
// (or I am too stupid to understand this)
__attribute__((weak)) uint32_t __get_IPSR()
{
	uint32_t result;
    asm volatile ("MRS %0, ipsr" : "=r" (result) );
}
