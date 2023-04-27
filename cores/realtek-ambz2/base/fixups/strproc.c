/* Copyright (c) Kuba Szczodrzyński 2023-03-03. */

#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef struct strproc_func_stubs_s {
	char *(*strcat)(char *dest, const char *src);
	char *(*strchr)(const char *s, int c);
	int (*strcmp)(const char *cs, const char *ct);
	int (*strncmp)(const char *cs, const char *ct, size_t count);
	int (*strnicmp)(const char *s1, const char *s2, size_t len);
	char *(*strcpy)(char *dest, const char *src);
	char *(*strncpy)(char *dest, const char *src, size_t count);
	size_t (*strlcpy)(char *dst, const char *src, size_t s);
	size_t (*strlen)(const char *s);
	size_t (*strnlen)(const char *s, size_t count);
	char *(*strncat)(char *dest, const char *src, size_t count);
	char *(*strpbrk)(const char *cs, const char *ct);
	size_t (*strspn)(const char *s, const char *accept);
	char *(*strstr)(const char *s1, const char *s2);
	char *(*strtok)(char *s, const char *ct);
	size_t (*strxfrm)(char *dest, const char *src, size_t n);
	char *(*strsep)(char **s, const char *ct);
	double (*strtod)(const char *str, char **endptr);
	float (*strtof)(const char *str, char **endptr);
	long double (*strtold)(const char *str, char **endptr);
	long (*strtol)(const char *nptr, char **endptr, int base);
	long long (*strtoll)(const char *nptr, char **endptr, int base);
	unsigned long (*strtoul)(const char *nptr, char **endptr, int base);
	unsigned long long (*strtoull)(const char *nptr, char **endptr, int base);
	int (*atoi)(const char *num);
	unsigned int (*atoui)(const char *num);
	long (*atol)(const char *num);
	unsigned long (*atoul)(const char *num);
	unsigned long long (*atoull)(const char *num);
	double (*atof)(const char *str);

	uint32_t reserved[16]; // reserved space for next ROM code version function table extending.
} strproc_func_stubs_t;

extern const strproc_func_stubs_t strproc_stubs;

// Wrappers

char *__wrap_strcat(char *dest, const char *src) {
	return strproc_stubs.strcat(dest, src);
}

char *__wrap_strchr(const char *s, int c) {
	return strproc_stubs.strchr(s, c);
}

int __wrap_strcmp(const char *cs, const char *ct) {
	return strproc_stubs.strcmp(cs, ct);
}

int __wrap_strncmp(const char *cs, const char *ct, size_t count) {
	return strproc_stubs.strncmp(cs, ct, count);
}

char *__wrap_strcpy(char *dest, const char *src) {
	return strproc_stubs.strcpy(dest, src);
}

char *__wrap_strncpy(char *dest, const char *src, size_t count) {
	return strproc_stubs.strncpy(dest, src, count);
}

size_t __wrap_strlen(const char *s) {
	return strproc_stubs.strlen(s);
}

char *__wrap_strncat(char *dest, const char *src, size_t count) {
	return strproc_stubs.strncat(dest, src, count);
}

char *__wrap_strpbrk(const char *cs, const char *ct) {
	return strproc_stubs.strpbrk(cs, ct);
}

size_t __wrap_strspn(const char *s, const char *accept) {
	return strproc_stubs.strspn(s, accept);
}

char *__wrap_strstr(const char *s1, const char *s2) {
	return strproc_stubs.strstr(s1, s2);
}

char *__wrap_strtok(char *s, const char *ct) {
	return strproc_stubs.strtok(s, ct);
}

size_t __wrap_strxfrm(char *dest, const char *src, size_t n) {
	return strproc_stubs.strxfrm(dest, src, n);
}

double __wrap_strtod(const char *str, char **endptr) {
	return strproc_stubs.strtod(str, endptr);
}

float __wrap_strtof(const char *str, char **endptr) {
	return strproc_stubs.strtof(str, endptr);
}

long double __wrap_strtold(const char *str, char **endptr) {
	return strproc_stubs.strtold(str, endptr);
}

double __wrap_atof(const char *str) {
	return strproc_stubs.atof(str);
}

long __wrap_strtol(const char *nptr, char **endptr, int base) {
	return strproc_stubs.strtol(nptr, endptr, base);
}

long long __wrap_strtoll(const char *nptr, char **endptr, int base) {
	return strproc_stubs.strtoll(nptr, endptr, base);
}

unsigned long __wrap_strtoul(const char *nptr, char **endptr, int base) {
	return strproc_stubs.strtoul(nptr, endptr, base);
}

unsigned long long __wrap_strtoull(const char *nptr, char **endptr, int base) {
	return strproc_stubs.strtoull(nptr, endptr, base);
}

int __wrap_atoi(const char *num) {
	return strproc_stubs.atoi(num);
}

long __wrap_atol(const char *num) {
	return strproc_stubs.atol(num);
}

// Standalone functions

unsigned long atoul(const char *num) {
	return strproc_stubs.atoul(num);
}

unsigned long long atoull(const char *num) {
	return strproc_stubs.atoull(num);
}

unsigned int atoui(const char *num) {
	return strproc_stubs.atoui(num);
}

size_t strnlen(const char *s, size_t count) {
	return strproc_stubs.strnlen(s, count);
}

size_t strlcpy(char *dst, const char *src, size_t s) {
	return strproc_stubs.strlcpy(dst, src, s);
}

int strnicmp(const char *s1, const char *s2, size_t len) {
	return strproc_stubs.strnicmp(s1, s2, len);
}

char *strsep(char **s, const char *ct) {
	return strproc_stubs.strsep(s, ct);
}
