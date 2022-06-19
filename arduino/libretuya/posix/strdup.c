/* Copyright (c) Kuba Szczodrzyński 2022-05-16. */

#include <stddef.h>
#include <sdk_mem.h>

__attribute__((weak)) char *strdup(const char *s) {
	size_t len = strlen(s) + 1;
	void *newp = malloc(len);
	if (newp == NULL)
		return NULL;
	return (char *)memcpy(newp, s, len);
}
