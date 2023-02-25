/* Copyright (c) Kuba Szczodrzyński 2022-11-26. */

// make <strproc.h> not #define isprint, isdigit, isxdigit, islower and isspace
// this conflicts with stdlib <ctype.h>, if <strproc.h> is included before it

// include <ctype.h> before to get all its macros
#include <ctype.h>

// make 'static inline int _tolower' unused
#undef _tolower
#define _tolower _tolower_dummy

#include_next <strproc.h>

// restore _tolower to ctype's macro
#undef _tolower
#define _tolower(__c) ((unsigned char)(__c) - 'A' + 'a')

// dirty fix for compiling mbedTLS which uses _B as variable name
#ifdef CONFIG_SSL_RSA
#undef _B
#endif