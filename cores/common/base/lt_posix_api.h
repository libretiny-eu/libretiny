/* Copyright (c) Kuba Szczodrzyński 2022-05-16. */

#include <sys/time.h>
#include <time.h>

// These implementations live in base/posix/*.c (C linkage). Without the
// guard, a C++ TU whose libc headers don't already declare them (e.g.
// newlib's strptime is hidden unless __XSI_VISIBLE) emits mangled
// references that can never resolve against the C definitions.
#ifdef __cplusplus
extern "C" {
#endif
extern char *strdup(const char *);
extern int strcasecmp(const char *s1, const char *s2);
extern int strncasecmp(const char *s1, const char *s2, size_t n);
extern char *strptime(const char *buf, const char *fmt, struct tm *tm);
#ifdef __cplusplus
} // extern "C"
#endif
