/* Copyright (c) Kuba Szczodrzyński 2022-07-31. */

#include "pinscan.h"

bool ansi		   = true;
bool ansiSkipErase = false;

void printAnsi(const char *str) {
	if (ansi)
		stream->print(str);
}

void printAnsiErase() {
	if (!ansiSkipErase)
		printAnsi(ANSI_ERASE ANSI_HOME);
	ansiSkipErase = false;
}

void line() {
	stream->print("--------");
	if (output == -1) {
		stream->print("TELNET");
	} else {
		stream->print(" UART");
		stream->print(output);
		stream->write(' ');
	}
	stream->print("--------");
	stream->println();
}

int inputPin() {
	stream->print("Enter pin number: ");
	int pin = stream->parseInt();
	stream->println();
	return pin;
}
