/* Copyright (c) Kuba Szczodrzyński 2022-07-31. */

#include "pinscan.h"

static unsigned long last = 0;

static void readAnalog(uint8_t number, pin_size_t pin) {
	stream->write("A");
	stream->write('0' + number);
	stream->print(": ");
	stream->print(analogReadVoltage(pin));
	stream->print(" mV\t");
}

void runAnalog() {
	if (mode[1] == '\0')
		return;
	int pin = 0;
	switch (mode[1]) {
		case '?':
			printHelp('a');
			break;

		case 'r':
			if (millis() - last < DELAY_READ)
				return;
			last = millis();
			printAnsi(ANSI_LINE_START);
#ifdef PIN_A0
			readAnalog(0, PIN_A0);
#endif
#ifdef PIN_A1
			readAnalog(1, PIN_A1);
#endif
#ifdef PIN_A2
			readAnalog(2, PIN_A2);
#endif
			if (!ansi)
				stream->println();
			return;

		case 'o':
			pin = inputPin();
			// clang-format off
#ifdef PIN_A0
			if (pin == 0) {
				readAnalog(0, PIN_A0);
			} else
#endif
#ifdef PIN_A1
			if (pin == 1) {
				readAnalog(0, PIN_A0);
			} else
#endif
#ifdef PIN_A2
			if (pin == 2) {
				readAnalog(2, PIN_A2);
			} else
#endif
			// clang-format on
			{
				stream->print("Pin unavailable");
			}
			stream->println();
			mode[1]		  = '?';
			ansiSkipErase = true;
			return;
	}
	mode[1] = '\0';
}
