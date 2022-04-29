#include "LibreTuyaAPI.h"

__weak char *strdup(const char *s) {
	size_t len = strlen(s) + 1;
	void *newp = malloc(len);
	if (newp == NULL)
		return NULL;
	return (char *)memcpy(newp, s, len);
}
