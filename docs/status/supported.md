# Supported boards, modules and CPUs

## Board list

{%
	include-markdown "supported_boards.md"
%}

\* I/O count includes GPIOs, ADCs, PWM outputs and UART, but doesn't count CEN/RST and power pins.

## CPU list

Chips currently supported by the project:

{%
	include-markdown "supported_chips.md"
%}

This list is not exhaustive, i.e. a similar chip (but different package) might work just fine, but there's no board definition for it yet.
If you have an unsupported chip, feel free to reach out using Issues or on the Discord server.

## Families

A list of chip families currently supported by this project.

!!! note
	The term *family* was chosen over *platform*, in order to reduce possible confusion between LibreTiny supported "platforms" and PlatformIO's "platform", as an entire package. *Family* is also more compatible with the UF2 term.

The following list corresponds to UF2 OTA format family names, and is also [available as JSON](../../families.json). The IDs are also present in [lt_Types.h](../../ltapi/lt__types_8h.md).

{%
	include-markdown "supported_families.md"
%}

## Unsupported boards

### Tuya Inc.

!!! note
	Only modules featuring at least Wi-Fi are included in the table. (TY)JW, (TY)WE and (TY)LC Series are omitted, as they contain Espressif chips.

{%
	include-markdown "unsupported_boards_tuya_all.md"
%}
