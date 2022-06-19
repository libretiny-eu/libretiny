// https://github.com/embeddedartistry/libc/blob/master/src/stdio/puts.c

#include <printf/printf.h>
#include <stdio.h>

int __wrap_puts(const char *str) {
	int r = 0;

	for (const char *c = str; *c != 0; c++) {
		putchar_((int)*c);
		r++;
	}

	// puts adds a newline
	if (r) {
		putchar_('\n');
		r++;
	}

	return r ? r : EOF;
}
