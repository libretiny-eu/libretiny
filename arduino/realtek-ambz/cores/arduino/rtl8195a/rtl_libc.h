/*
*	 ROM calls
*/

#ifndef _INC_RTL_LIBC_
#define _INC_RTL_LIBC_
/*
#undef malloc
#define malloc(size)            pvPortMalloc(size)
#undef free
#define free(pbuf)              vPortFree(pbuf)
*/

#define atoi(str)               prvAtoi(str)

#define calloc(nelements, elementSize) calloc_freertos(nelements, elementSize)

#define snprintf	rtl_snprintf	
#define sprintf		rtl_sprintf		
#define printf		rtl_printf		
//#define vprintf		rtl_vprintf		
//#define vsnprintf	rtl_vsnprintf	
//#define vfprintf	rtl_vfprintf	
#define memchr		rtl_memchr		
#define memcmp		rtl_memcmp		
#define memcpy		rtl_memcpy		
#define memmove		rtl_memmove		
#define memset		rtl_memset		
#define strcat		rtl_strcat		
#define strchr		rtl_strchr		
#define strcmp		rtl_strcmp		
#define strcpy		rtl_strcpy		
#define strlen		rtl_strlen		
#define strncat		rtl_strncat		
#define strncmp		rtl_strncmp		
#define strncpy		rtl_strncpy		
#define strstr		rtl_strstr		
#define strsep		rtl_strsep		
#define strtok		rtl_strtok		

#define dtoi	rtl_dtoi	
#define dtoui	rtl_dtoui	
#define i2f		rtl_i2f		
#define i2d		rtl_i2d		
#define ui2f	rtl_ui2f	
#define ui2d	rtl_ui2d	
#define itoa	rtl_itoa	
#define ltoa	rtl_ltoa	
#define utoa	rtl_utoa	
#define ultoa	rtl_ultoa	
#define ftol	rtl_ftol	
#define ftod	rtl_ftod	
#define dtof	rtl_dtof	
#define fadd	rtl_fadd	
#define fsub	rtl_fsub	
#define fmul	rtl_fmul	
#define fdiv	rtl_fdiv	
#define dadd	rtl_dadd	
#define dsub	rtl_dsub	
#define dmul	rtl_dmul	
#define ddiv	rtl_ddiv	
#define dcmpeq	rtl_dcmpeq	
#define dcmplt	rtl_dcmplt	
#define dcmple	rtl_dcmple	
#define dcmpgt	rtl_dcmpgt	
#define fcmplt  rtl_fcmplt
#define fcmpgt	rtl_fcmpgt		

#define fabsf	rtl_fabsf
#define fabs	rtl_fabs
#define cos_f32 rtl_cos_f32
#define sin_f32 rtl_sin_f32


#if 0
extern void *calloc_freertos(size_t nelements, size_t elementSize);
// ram_libc.c
extern int rtl_snprintf(char *str, size_t size, const char *fmt, ...);             
extern int rtl_sprintf(char *str, const char *fmt, ...);                           
extern int rtl_printf(const char *fmt, ...);                                       
extern int rtl_vprintf(const char *fmt, void *param);                              
extern int rtl_vsnprintf(char *str, size_t size, const char *fmt, void *param);    
extern int rtl_vfprintf(FILE *fp, const char *fmt0, va_list ap);                   
extern int rtl_memchr(const void *src_void, int c, size_t length);                 
extern int rtl_memcmp(const void *m1, const void *m2, size_t n);                   
extern int rtl_memcpy(void *dst0, const void *src0, size_t len0);                  
extern int rtl_memmove(void *dst_void, const void *src_void, size_t length);       
extern int rtl_memset(void *m, int c, size_t n);                                   
extern char * rtl_strcat(char *s1, const char *s2);                                
extern char * rtl_strchr(const char *s1, int i);                                   
extern int rtl_strcmp(const char *s1, const char *s2);                             
extern char * rtl_strcpy(char *dst0, const char *src0);                            
extern int rtl_strlen(const char *str);                                            
extern char * rtl_strncat(char *s1, const char *s2, size_t n);                     
extern int rtl_strncmp(const char *s1, const char *s2, size_t n);                  
extern char * rtl_strncpy(char *dst0, const char *src0, size_t count);             
extern char * rtl_strstr(const char *searchee, const char *lookfor);               
extern char * rtl_strsep(char **source_ptr, const char *delim);                    
extern char * rtl_strtok(char *s, const char *delim);

//rtl_eabi_cast_ram.c
extern int rtl_dtoi(double d);
extern int rtl_dtoui(double d);
extern float rtl_i2f(int val);
extern int rtl_i2d(int val);
extern float rtl_ui2f(unsigned int val);
extern int rtl_ui2d(unsigned int val);
extern char *rtl_itoa(int value, char *string, int radix);
extern char *rtl_ltoa(int value, char *string, int radix);
extern char *rtl_utoa(unsigned int value, char *string, int radix);
extern char *rtl_ultoa(unsigned int value, char *string, int radix);
extern int rtl_ftol(float f);
extern int rtl_ftod(float f);
extern float rtl_dtof(double d);
extern float rtl_fadd(float a, float b);
extern float rtl_fsub(float a, float b);
extern float rtl_fmul(float a, float b);
extern float rtl_fdiv(float a, float b);
extern int rtl_dadd(double a, double b);
extern int rtl_dsub(double a, double b);
extern int rtl_dmul(double a, double b);
extern int rtl_ddiv(double a, double b);
extern int rtl_dcmpeq(double a, double b);
extern int rtl_dcmplt(double a, double b);
extern int rtl_dcmple(double a, double b);
extern int rtl_dcmpgt(double a, double b);
extern int rtl_fcmplt(float a, float b);
extern int rtl_fcmpgt(float a, float b);

// rtl_math_ram.c
extern float rtl_fabsf(float a);
extern int rtl_fabs(double a);
extern float rtl_cos_f32(float a);
extern float rtl_sin_f32(float a);

// ram_pvvx_libc.c
extern int snprintf(char *str, size_t size, const char *fmt, ...);
extern int sprintf(char *str, const char *fmt, ...);
extern int printf(const char *fmt, ...);
extern int vprintf(const char *fmt, void *param);
extern int vsnprintf(char *str, size_t size, const char *fmt, void *param);
extern int vfprintf(FILE *fp, const char *fmt0, va_list ap);
extern int memchr(const void *src_void, int c, size_t length);
extern int memcmp(const void *m1, const void *m2, size_t n);
extern int memcpy(void *dst0, const void *src0, size_t len0);
extern int memmove(void *dst_void, const void *src_void, size_t length);
extern int memset(void *m, int c, size_t n);
extern char * strcat(char *s1, const char *s2);
extern char * strchr(const char *s1, int i);
extern int strcmp(const char *s1, const char *s2);
extern char * strcpy(char *dst0, const char *src0);
extern int strlen(const char *str);
extern char * strncat(char *s1, const char *s2, size_t n);
extern int strncmp(const char *s1, const char *s2, size_t n);
extern char * strncpy(char *dst0, const char *src0, size_t count);
extern char * strstr(const char *searchee, const char *lookfor);
extern char * strsep(char **source_ptr, const char *delim);
extern char * strtok(char *s, const char *delim);
extern int sscanf(const char *buf, const char *fmt, ...);
extern char toupper(char ch);
extern int _stricmp (const char *s1, const char *s2);
extern unsigned long long __aeabi_llsr(unsigned long long val, unsigned int shift);
#endif

#endif // _INC_RTL_LIBC_
