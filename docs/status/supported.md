# Supported boards, modules and CPUs

## Board list

{%
	include-markdown "./supported_boards.md"
%}

\* I/O count includes GPIOs, ADCs, PWM outputs and UART, but doesn't count CEN/RST and power pins.

## CPU list

Chips currently supported by the project:

{%
	include-markdown "./supported_chips.md"
%}

This list is not exhaustive, i.e. a similar chip (but different package) might work just fine, but there's no board definition for it yet.
If you have an unsupported chip, feel free to reach out using Issues or on the Discord server.

## Families

A list of chip families currently supported by this project.

!!! note
	The term *family* was chosen over *platform*, in order to reduce possible confusion between LibreTiny supported "platforms" and PlatformIO's "platform", as an entire package. *Family* is also more compatible with the UF2 term.

The following list corresponds to UF2 OTA format family names, and is also [available as JSON](../../families.json). The IDs are also present in [lt_types.h](../../ltapi/lt__types_8h.md). You can view the family list by using `ltchiptool list families`.

{%
	include-markdown "./supported_families.md"
%}

## Feature support

If you notice a feature that you've tested, which works (or not) and doesn't match this table, feel free to submit an issue on GitHub.

&nbsp;                   | `BK7231T` | `BK7231N` | `RTL8710B` | `RTL8720C` | `BK7231Q`  |  `LN882H` 
-------------------------|-----------|-----------|------------|------------|------------|------------
Stability                | 5/5       | 5/5       | 4/5        | 2/5        | 1/5        | 3/5
LibreTiny Core           | ✔️        | ✔️        | ✔️         | ✔️         | ✔️         | ✔️
Wiring Core              | ✔️        | ✔️        | ✔️         | ✔️         | ✔️         | ✔️
**PERIPHERALS** (Core)   |           |           |            |            |            |
UART I/O                 | ✔️        | ✔️        | ✔️         | ✔️         | ✔️         | ✔️
Flash I/O                | ✔️        | ✔️        | ✔️         | ❓          | ❓         | ✔️
Deep sleep               | ❓         | ✔️        | ❌          | ❌          | ❓         | ❌
Watchdog timer           | ✔️        | ✔️        | ✔️         | ❓          | ❓         | ✔️
**PERIPHERALS** (Wiring) |           |           |            |            |            |
Digital I/O              | ✔️        | ✔️        | ✔️         | ❓          | ❓         | ✔️
PWM                      | ✔️        | ✔️        | ✔️         | ❓          | ❓         | ❌
Interrupts               | ✔️        | ✔️        | ✔️         | ❓          | ❓         | ✔️
Analog input (ADC)       | ✔️        | ✔️        | ✔️         | ❓          | ❓         | ✔️
`Wire` (I²C)             | ❌         | ❌         | ❗          | ❌          | ❌         | ❓
`SPI`                    | ❌         | ❌         | ❌          | ❌          | ❌         | ❌
`Serial`                 | ✔️        | ✔️        | ✔️         | ✔️         | ❓         | ✔️
`SoftwareSerial`         | ❌         | ❌         | ✔️         | ❌          | ❌         | ❌
**NETWORKING**           |           |           |            |            |            |
Wi-Fi STA/AP/Mixed       | ✔️        | ✔️        | ✔️         | ❓          | ❌         | ✔️
Wi-Fi Events             | ✔️        | ✔️        | ✔️         | ❓          | ❌         | ✔️
OTA updates              | ✔️        | ✔️        | ✔️         | ❌          | ❌         | ✔️
MDNS                     | ✔️        | ✔️        | ✔️         | ❓          | ❓         | ✔️

Symbols:

- ✔️ working
- ❓ untested
- ❗ broken
- ❌ not implemented (yet?)
- \- not applicable

## Unsupported boards

### Tuya Inc.

!!! note
	Only modules featuring at least Wi-Fi are included in the table. (TY)JW, (TY)WE and (TY)LC Series are omitted, as they contain Espressif chips.

{%
	include-markdown "./unsupported_boards_tuya_all.md"
%}
