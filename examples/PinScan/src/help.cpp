/* Copyright (c) Kuba Szczodrzyński 2022-07-31. */

#include "pinscan.h"

void printHelp(uint8_t mode) {
	printAnsiErase();
	switch (mode) {
		case '\0':
			stream->setTimeout(10000);
			stream->println("LibreTuya v" LT_VERSION_STR ", PinScan v" EXAMPLE_VER);
			stream->println("Board: " LT_BOARD_STR);
			stream->print("I/O count: ");
			stream->println(PINS_COUNT);
			stream->print("Digital I/O count: ");
			stream->println(NUM_DIGITAL_PINS);
			stream->print("Analog input count: ");
			stream->println(NUM_ANALOG_INPUTS);
			break;
		case 'd':
			stream->println("Digital I/O");
			break;
		case 'a':
			stream->println("Analog inputs");
			break;
	}
	line();
	stream->println("Commands:");
	switch (mode) {
		case '\0':
			// clang-format off
			stream->println(
				TAB "d - Check digital pins" EOL
				TAB "a - Check analog pins" EOL
				// TAB "p - Check PWM outputs" EOL
#if !USE_WIFI
				TAB "s - Select UART port" EOL
#endif
				TAB "t - Toggle ANSI control codes" EOL
				TAB "r - Reboot (for uploading)" EOL
				TAB "q - Go back to menu, at any time" EOL
				TAB "? - Print help text, also for subcommands" EOL
			);
			// clang-format on
			break;
		case 'd':
			// clang-format off
			stream->println(
				TAB "r - Realtime readout of all pins" EOL
				TAB "o - Read one pin continuously" EOL
				TAB "s - Manual Scan - toggle each pin" EOL
				TAB "h - Write HIGH to a pin" EOL
				TAB "l - Write LOW to a pin" EOL
				TAB "p - Output using pull up/down (default)" EOL
				TAB "w - Output using write low/high (less safe)" EOL
			);
			// clang-format on
			break;
		case 'a':
			// clang-format off
			stream->println(
				TAB "r - Realtime readout of all pins" EOL
				TAB "o - Read one pin once" EOL
			);
			// clang-format on
			break;
	}
}
