/* Copyright (c) Kuba Szczodrzyński 2022-07-31. */

#include "pinscan.h"

#define PULL  0
#define WRITE 1

static bool pins[NUM_DIGITAL_PINS] = {};
static bool used[NUM_DIGITAL_PINS] = {};
static bool active				   = false;
static int pin					   = -1;

static unsigned long last = 0;
static bool outputMode	  = PULL;

static void printPin(bool state) {
	if (ansi) {
		stream->print(state ? (ANSI_RED "HI" ANSI_RESET) : (ANSI_BLUE "LO" ANSI_RESET));
	} else {
		stream->print(state ? "HI" : "LO");
	}
}

static void printState() {
	printAnsi(ANSI_TWO_LINES);
	printAnsi(ANSI_ERASE_LINE);
	for (pin_size_t i = 0; i < NUM_DIGITAL_PINS; i++) {
		if (i < 10)
			stream->write(' ');
		stream->write('D');
		stream->print(i);
		stream->write(' ');
	}
	stream->println();
	printAnsi(ANSI_ERASE_LINE);
	for (pin_size_t i = 0; i < NUM_DIGITAL_PINS; i++) {
		stream->write(' ');
		if (used[i]) {
			printPin(pins[i]);
		} else {
			stream->print("--");
		}
		stream->write(' ');
	}
	stream->println();
}

static void irqHandler(void *ptr) {
	bool *pin	 = (bool *)ptr;
	pin_size_t i = pin - pins;
	pins[i]		 = digitalRead(i);
	// change interrupt level according to current state
	PinStatus status = pins[i] ? FALLING : RISING;
	attachInterruptParam(i, irqHandler, status, ptr);
	printState();
}

static void digitalAllIrq() {
#if USE_WIFI
	LT_I("Disabling logger");
	lt_log_disable();
	Serial.end();
#endif
	for (pin_size_t i = 0; i < NUM_DIGITAL_PINS; i++) {
		if (pinSkip[0] == i || pinSkip[1] == i) {
			used[i] = false;
			continue;
		}
		pinMode(i, INPUT);
		pins[i] = digitalRead(i);
		used[i] = true;
		// choose interrupt level according to current state
		PinStatus status = pins[i] ? FALLING : RISING;
		attachInterruptParam(i, irqHandler, status, pins + i);
	}
	active = true;
}

static void digitalOut(pin_size_t pin, PinStatus state) {
	if (outputMode == PULL) {
		pinMode(pin, state ? INPUT_PULLUP : INPUT_PULLDOWN);
	} else {
		pinMode(pin, OUTPUT);
		digitalWrite(pin, state);
	}
	pins[pin] = state;
}

static int digitalAllLow() {
	int first = -1;
	for (pin_size_t i = 0; i < NUM_DIGITAL_PINS; i++) {
		if (pinSkip[0] == i || pinSkip[1] == i) {
			used[i] = false;
			continue;
		}
		if (first == -1)
			first = i;
		digitalOut(i, LOW);
		used[i] = true;
	}
	active = true;
	return first;
}

void digitalDetach() {
	outputMode = PULL;
	if (!active)
		return;
	for (pin_size_t i = 0; i < NUM_DIGITAL_PINS; i++) {
		if (pinSkip[0] == i || pinSkip[1] == i) {
			continue;
		}
		if (used[i])
			detachInterrupt(i);
		used[i] = false;
		pinMode(i, INPUT_PULLDOWN);
	}
	active = false;
	pin	   = -1;
#if USE_WIFI
	lt_log_set_port(LT_UART_DEFAULT_LOGGER);
	Serial.begin(115200);
	LT_I("Logger enabled");
#endif
}

void runDigital() {
	if (mode[1] == '\0')
		return;
	switch (mode[1]) {
		case '?':
			printHelp('d');
			pin = -1;
			break;

		case 'p':
			outputMode = PULL;
			stream->println("Will pull outputs UP/DOWN");
			mode[1]		  = '?';
			ansiSkipErase = true;
			return;
		case 'w':
			outputMode = WRITE;
			stream->println("Will write LOW/HIGH to outputs");
			mode[1]		  = '?';
			ansiSkipErase = true;
			return;

		case 'r':
			if (!active) {
				digitalAllIrq();
				if (ansi)
					printHelp('d');
				printAnsi("\n\n"); // reserve two lines for readouts
				printState();
			}
			return;

		case 'o':
			if (pin == -1) {
				pin = inputPin();
				pinMode(pin, INPUT);
			}
			printAnsi(ANSI_LINE_START);
			stream->print("Pin D");
			stream->print(pin);
			stream->print(" state: ");
			printPin(digitalRead(pin));
			if (!ansi)
				stream->println();
			return;

		case 's':
			if (pin == -1) {
				// choose the first pin
				pin = digitalAllLow();
				if (ansi)
					printHelp('d');
				printAnsi("\n\n"); // reserve two lines for readouts
				printState();
			}
			if (mode[2] == 'n') {
				// go to next pin; leave the current as LOW
				digitalOut(pin, LOW);
				do {
					if (++pin >= NUM_DIGITAL_PINS)
						pin = 0;
				} while (used[pin] == false);
				printState();
			}
			mode[2] = '\0';
			// toggle the pin every 500ms
			if (millis() - last < 500)
				return;
			last = millis();
			digitalOut(pin, (PinStatus)!pins[pin]);
			printState();
			return;

		case 'h':
		case 'l':
			pin = inputPin();
			digitalOut(pin, mode[1] == 'h' ? HIGH : LOW);
			stream->println("OK");
			mode[1]		  = '?';
			ansiSkipErase = true;
			return;
	}
	mode[1] = '\0';
}
