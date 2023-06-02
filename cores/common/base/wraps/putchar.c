// https://github.com/embeddedartistry/libc/blob/master/src/stdio/putchar.c

#include <printf/printf.h>
#include <stdio.h>

int __wrap_putchar(int c) {
	putchar_((char)c);
	return c;
}
