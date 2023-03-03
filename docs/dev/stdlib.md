# C stdlib

Usually, functions available in C standard library should not be defined by the SDK. Instead, they should be included using GCC's headers, and implemented by the libc or wrapped and implemented in the SDK.

The following functions must not be defined by the SDK. Their presence makes conflicts due to incompatibility with C library, so they should be removed or disabled, and replaced with wrappers.

Memory management functions should be wrapped and redirected to FreeRTOS (if possible).

Additionally, if the `printf` library is used in the chip family code, all other vendor-defined printf-like functions should be replaced with it.

Function prototypes taken from [devdocs.io](https://devdocs.io/c/).

```cpp
// stdlib.h / Dynamic memory management
void *calloc(size_t num, size_t size);
void *malloc(size_t size);
void *realloc(void *ptr, size_t new_size);
void free(void *ptr);
// stdlib.h / Conversions to numeric formats
double atof(const char *str);
int atoi(const char *str);
long atol(const char *str);
long long atoll(const char *str);
double strtod(const char *str, char **str_end);
float strtof(const char *str, char **str_end);
long strtol(const char *str, char **str_end, int base);
long double strtold(const char *str, char **str_end);
long long strtoll(const char *str, char **str_end, int base);
unsigned long strtoul(const char *str, char **str_end, int base);
unsigned long long strtoull(const char *str, char **str_end, int base);

// string.h / Character classification
int isalnum(int ch);
int isalpha(int ch);
int islower(int ch);
int isupper(int ch);
int isdigit(int ch);
int isxdigit(int ch);
int iscntrl(int ch);
int isgraph(int ch);
int isspace(int ch);
int isblank(int ch);
int isprint(int ch);
int ispunct(int ch);
int tolower(int ch);
int toupper(int ch);
// string.h / String manipulation
char *strcat(char *dest, const char *src);
char *strcpy(char *dest, const char *src);
char *strncat(char *dest, const char *src, size_t count);
char *strncpy(char *dest, const char *src, size_t count);
size_t strxfrm(char *dest, const char *src, size_t count);
// string.h / String examination
char *strchr(const char *str, int ch);
int strcmp(const char *lhs, const char *rhs);
size_t strcspn(const char *dest, const char *src);
size_t strlen(const char *str);
int strncmp(const char *lhs, const char *rhs, size_t count);
char *strpbrk(const char *dest, const char *breakset);
char *strrchr(const char *str, int ch);
size_t strspn(const char *dest, const char *src);
char *strstr(const char *str, const char *substr);
char *strtok(char *str, const char *delim);
// string.h / Character array manipulation
void *memchr(const void *ptr, int ch, size_t count);
int memcmp(const void *lhs, const void *rhs, size_t count);
void *memcpy(void *dest, const void *src, size_t count);
void *memmove(void *dest, const void *src, size_t count);
void *memset(void *dest, int ch, size_t count);

// stdio.h / Unformatted input/output
int putc(int ch, FILE *stream);
int putchar(int ch);
int puts(const char *str);

// stdio.h / Formatted input/output
int printf(const char *format, ...);
int sprintf(char *buffer, const char *format, ...);
int snprintf(char *buffer, size_t bufsz, const char *format, ...);
int vprintf(const char *format, va_list vlist);
int vsprintf(char *buffer, const char *format, va_list vlist);
int vsnprintf(char *buffer, size_t bufsz, const char *format, va_list vlist);

// Non-stdlib
_calloc_r
_free_r
_malloc_r
_realloc_r
atoui
atoul
atoull
strlcpy
strnicmp
strnlen
strsep
zalloc

// Additional forbidden macros
#define max(a, b)
#define min(a, b)
typedef bool boolean;
```
