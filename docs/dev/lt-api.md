# LibreTiny API

The LibreTiny API is divided in two parts:

- the C API, available in all families and frameworks
- the C++ API, available in the Arduino framework only.

The C++ API is a thin wrapper around the C API (using classes with inline functions).
It's provided for less-experienced users, who are used to Arduino IDE's classes like `ESP` (and for backwards compatibility).
It's recommended to use the C API wherever possible.

## C API

This API is available using:

- `#include <libretiny.h>`
- `#include <Arduino.h>`

### CPU

{%
	include-markdown "../../ltapi/lt__cpu_8h.md"
   start="## Public Functions\n"
   end="## Public Functions Documentation"
%}

### Device

{%
	include-markdown "../../ltapi/lt__device_8h.md"
   start="## Public Functions\n"
   end="## Public Functions Documentation"
%}

### Flash

{%
	include-markdown "../../ltapi/lt__flash_8h.md"
   start="## Public Functions\n"
   end="## Public Functions Documentation"
%}

### Memory

{%
	include-markdown "../../ltapi/lt__mem_8h.md"
   start="## Public Functions\n"
   end="## Public Functions Documentation"
%}

### OTA

{%
	include-markdown "../../ltapi/lt__ota_8h.md"
   start="## Public Functions\n"
   end="## Public Functions Documentation"
%}

### Utilities

{%
	include-markdown "../../ltapi/lt__utils_8h.md"
   start="## Public Functions\n"
   end="## Macros"
%}

### Watchdog

{%
	include-markdown "../../ltapi/lt__wdt_8h.md"
   start="## Public Functions\n"
   end="## Public Functions Documentation"
%}

### Logger

{%
	include-markdown "../../ltapi/lt__logger_8h.md"
   start="## Public Functions\n"
   end="## Macros"
%}

### POSIX compatibility API

A small subset of POSIX functions, commonly present in other Arduino cores, is provided for compatibility.

{%
	include-markdown "../../ltapi/lt__posix__api_8h.md"
   start="## Public Functions\n"
   end="## Public Functions Documentation"
%}

## C++ API

This API is available using:

- `#include <Arduino.h>`

### LibreTiny

{%
	include-markdown "../../ltapi/class_libre_tiny.md"
   start="# Detailed Description\n"
   end="## Public Functions Documentation"
%}

{%
	include-markdown "../../ltapi/class_libre_tiny.md"
   start="## Public Functions\n"
   end="# Detailed Description"
%}

### LibreTinyOTA

{%
	include-markdown "../../ltapi/class_libre_tiny_o_t_a.md"
   start="# Detailed Description\n"
   end="## Public Functions Documentation"
%}

{%
	include-markdown "../../ltapi/class_libre_tiny_o_t_a.md"
   start="## Public Functions\n"
   end="# Detailed Description"
%}

### LibreTinyWDT

{%
	include-markdown "../../ltapi/class_libre_tiny_w_d_t.md"
   start="# Detailed Description\n"
   end="## Public Functions Documentation"
%}

{%
	include-markdown "../../ltapi/class_libre_tiny_w_d_t.md"
   start="## Public Functions\n"
   end="# Detailed Description"
%}

### ESP (compatibility class)

{%
	include-markdown "../../ltapi/class_esp_class.md"
   start="# Detailed Description\n"
   end="## Public Functions Documentation"
%}

{%
	include-markdown "../../ltapi/class_esp_class.md"
   start="## Public Functions\n"
   end="# Detailed Description"
%}

### Arduino custom API

These functions extend the standard Wiring (Arduino) library, to provide additional features.

{%
	include-markdown "../../ltapi/wiring__custom_8h.md"
   start="## Public Functions\n"
   end="## Macros"
%}

### Arduino compatibility API

These functions and macros provide compatibility between LT and other Arduino cores, such as ESP32.

{%
	include-markdown "../../ltapi/wiring__compat_8h.md"
   start="## Public Functions\n"
   end="## Macros"
%}

{%
	include-markdown "../../ltapi/wiring__compat_8h.md"
   start="## Macros\n"
   end="## Public Functions Documentation"
%}

## Chip & family types

{%
	include-markdown "../../ltapi/lt__types_8h.md"
   start="## Public Types Documentation\n"
   end="## Macro Definition Documentation"
%}
