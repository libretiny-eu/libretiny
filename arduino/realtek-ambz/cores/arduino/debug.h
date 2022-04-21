#ifndef ARD_DEBUG_H
#define ARD_DEBUG_H

#include <stddef.h>
#include <stdint.h>

#ifdef DEBUG_RTL_CORE
#define DEBUGV(...) rtl_printf(__VA_ARGS__)
#endif

#ifndef DEBUGV
#define DEBUGV(...)
#endif

#ifdef __cplusplus
void hexdump(void * addr, int len = 16);
#else
void hexdump(void * addr, int len);
#endif

#ifdef __cplusplus
extern "C" {
#endif
void print_udp_pcb(void);
void print_tcp_pcb(void);
void DumpForOneBytes(void *addr, int cnt); // cnt max 0x70 !

void __panic_func(const char* file, int line, const char* func) __attribute__((noreturn));
#define panic() __panic_func(__FILE__, __LINE__, __func__)

#ifdef __cplusplus
}
#endif


#endif//ARD_DEBUG_H
