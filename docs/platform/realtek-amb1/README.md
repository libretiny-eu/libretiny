# Realtek Ameba1

## Introduction

Realtek Ameba1 is the first generation of the Ameba Wi-Fi family. The
`realtek-amb1` port targets RTL8710A-series parts such as the RTL8195A and
RTL8711AM.

Features:

- ARM Cortex-M3 CPU (166 MHz on the tested RTL8711AM)
- 256 KiB SRAM plus external SDRAM on some parts
- SPI flash interface
- 802.11b/g/n Wi-Fi

Resources:

- [Realtek product page](https://www.realtek.com/en/products/communications-network-ics/item/rtl8711am)
- [Ameba1/AmebaZ SDK](https://github.com/ambiot/amb1_sdk)
- [Realtek PDFs for Ameba1/AmebaZ](https://github.com/ambiot/amb1_sdk/tree/0c8da639b097f01c60e419405aecfafab1d08e43/doc/)

## Finding your board

{%
	include-markdown "../../inc/find-board.md"
%}

---

## Flashing

{%
	include-markdown "../../inc/flashing-note.md"
%}

The RTL8711AM boot ROM does not provide the standard UART download protocol used
by AmebaZ. Boards in this family are supported with an external SPI programmer
and a native loader image; use the board's documentation before attempting to
write flash.

The runtime status, memory layout, and known limitations of this port are
documented in [Port status](port-status.md).
