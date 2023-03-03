# TODO list

## General

### Environment stability

Do not publish *any* SDK functions, macros, defines and includes. Define only what's needed in LT's public headers (like `Arduino.h`). Everything else is taken from `sdk_extern.h` or `WVariant.h` (TODO decide whether to keep WV public / make both private / get rid of WV and use sdk_extern only). Private headers are included by LT's .cpp units (maybe a dedicated private header that would include sdk_extern + Arduino.h).

Developers wanting to use SDK functions need to include them.

Explicit is better than implicit.

- consider moving to C++17 (GNU)? or any newer than C++11
- wrap all memory management functions (malloc, calloc, free, memset, etc.) and their vendor SDK counterparts to use FreeRTOS instead
	- pretty much done for ambz and ambz2, Beken is yet left to do (`os_malloc()` etc.)
- remove all network protocol client/server implementations from SDKs (mDNS, HTTP, DNS, etc.)

### New families

- BL602
- RTL8710A
- RTL8720C
- RTL8720D
- W600 and/or W800
- LN8825
- BK7231Q
- host-native family

### Tools

- write OpenOCD flashers, using uf2ota library + FAL for partitions (in ltchiptool repository)

### Serial

- configuration of RX/TX pins
- SoftwareSerial library - receiving + Beken family

### Other

- refactor `SerialClass` to have a shared header `Serial.h` in the common core (and `SerialData`, just like WiFi)
- implement Wire on BK, refactor the API and class
- watchdog API
- `Preferences` library
- test/fix IPv6 on different families
- what is `PowerManagement` at all? probably useless -> remove

## BK7231

- fix WiFi on BK7231N, test other functionality
- fix SSL (mbedTLS)
- I2C (Wire)
- SPI
- BLE

## RTL8710B

- take all stdio functions from stdio.h
- rewrite most of Wiring (it was copied from `ambd_arduino`, and is ugly)
