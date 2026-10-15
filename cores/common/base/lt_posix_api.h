/* Copyright (c) Kuba Szczodrzyński 2022-05-16. */

#include <sys/time.h>
#include <time.h>

#ifdef __cplusplus
extern "C" {
#endif // __cplusplus

extern char *strdup(const char *);
extern int strcasecmp(const char *s1, const char *s2);
extern int strncasecmp(const char *s1, const char *s2, size_t n);
extern char *strptime(const char *buf, const char *fmt, struct tm *tm);

#ifdef __cplusplus
} // extern "C"
#endif // __cplusplus
