/* Copyright (c) Bl00d-B0b 2026-08-06. */

// Newlib syscall stubs the SDK does not provide on AmebaD.

#include <sys/types.h>

// Normally from crtbegin, which -nostartfiles drops; referenced by
// __cxa_atexit() for C++ static objects. Nothing runs destructors on a
// firmware image, so the address only has to be unique.
void *__dso_handle = 0;

void _exit(int status) {
	(void)status;
	while (1) {}
}

int _kill(pid_t pid, int sig) {
	(void)pid;
	(void)sig;
	return -1;
}

pid_t _getpid(void) {
	return 1;
}

// Minimal newlib file-IO stubs: the SDK has no filesystem behind stdio, but
// printf's fallback path and abort() reference these.
int _close(int fd) {
	(void)fd;
	return -1;
}

int _fstat(int fd, void *st) {
	(void)fd;
	(void)st;
	return -1;
}

int _isatty(int fd) {
	(void)fd;
	return 1;
}

int _lseek(int fd, int off, int whence) {
	(void)fd;
	(void)off;
	(void)whence;
	return -1;
}

int _read(int fd, char *buf, int len) {
	(void)fd;
	(void)buf;
	(void)len;
	return -1;
}

int _write(int fd, const char *buf, int len) {
	extern void putchar_p(char c, unsigned long port);
	extern unsigned char lt_uart_port;
	(void)fd;
	for (int i = 0; i < len; i++)
		putchar_p(buf[i], lt_uart_port);
	return len;
}
