/* Copyright (c) Kuba Szczodrzyński 2023-03-03. */

#pragma once

// Remove SDK definitions of string processing methods.
// Include stdlib versions instead.
// Wrapper functions for the removed prototypes are provided in strproc.c.

#include <stdarg.h>
#include <stddef.h>
#include <stdlib.h>
#include <string.h>

// These functions aren't part of stdlib.

unsigned long atoul(const char *num);
unsigned long long atoull(const char *num);
unsigned int atoui(const char *num);
size_t strnlen(const char *s, size_t count);
size_t strlcpy(char *dst, const char *src, size_t s);
int strnicmp(const char *s1, const char *s2, size_t len);
char *strsep(char **s, const char *ct);
