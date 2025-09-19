/* Copyright (c) Kuba Szczodrzyński 2022-07-31. */

#include <Arduino.h>

#define USE_WIFI   0   // set up a WiFi telnet server
#define DELAY_READ 100 // interval for realtime readouts

#define WIFI_SSID "MySSID"
#define WIFI_PASS "Secr3tPa$$w0rd"

#define EXAMPLE_VER "1.0"

#define EOL "\r\n"
#define TAB "\t"

#define ANSI_ERASE		"\x1B[2J"
#define ANSI_HOME		"\x1B[H"
#define ANSI_LINE_START "\x1B[0G"
#define ANSI_TWO_LINES	"\x1B[2F"
#define ANSI_ERASE_LINE "\x1B[0K"
#define ANSI_BLUE		"\x1B[0;34m"
#define ANSI_RED		"\x1B[0;31m"
#define ANSI_RESET		"\x1B[0m"

extern Stream *stream;
extern uint8_t mode[];
extern pin_size_t pinSkip[2];
extern int output;
extern bool ansi;
extern bool ansiSkipErase;

extern void printHelp(uint8_t mode = '\0');
extern void printAnsi(const char *str);
extern void printAnsiErase();
extern void line();
extern int inputPin();
extern void runAnalog();
extern void runDigital();
extern void digitalDetach();
