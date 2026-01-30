# Realtek AmebaZ2

## Introduction

Realtek AmebaZ2 is a family of Wi-Fi (or Wi-Fi + BLE) microcontrollers, primarily consisting of RTL8710CF, RTL8720CF and RTL8720CM.

The CF version has built-in 2 MiB flash, whereas the CM version has built-in 4 MiB PSRAM (but no internal flash).

Features:

- ARM Cortex-M33 (ARMv8-M) CPU (100 MHz)
- 384 KiB ROM
- 256 KiB SRAM
- 2 MiB SPI flash (CF only) with XiP
- 4 MiB SPI PSRAM (CM only)
- 802.11b/g/n Wi-Fi
- BLE 4.2 (8720 only)

Resources:

- [Realtek Ameba-ZII datasheet v0.8](https://web.archive.org/web/20230523175304if_/https://cetest02.cn-bj.ufileos.com/100001_2110255103/RTL872xZ2%20IC%20Datasheet.pdf)
- [Realtek Ameba-ZII application note v1.5](https://web.archive.org/web/20221109080820if_/https://cetest02.cn-bj.ufileos.com/100001_2110255103/Ameba%20Z2%20Application%20Note.pdf)

## Finding your board

{%
	include-markdown "../../inc/find-board.md"
%}

## Flashing

{%
	include-markdown "../../inc/flashing-note.md"
%}

Realtek RTL8720C has three UART ports - UART2 (sometimes called LOG_UART), UART1 and UART0. **The port used for flashing and viewing logs is UART2**.

You need to find which pins correspond to UART2 TX and RX. If your board is supported, you'll find the pinout on its documentation page. Otherwise (and for generic boards), you'll have to find the pinout online.

### Wiring

Connect UART2 of the Realtek chip to the USB-TTL adapter:

PC  | RTL8720C
----|--------------------
RX  | TX2 (Log_TX / PA16)
TX  | RX2 (Log_RX / PA15)
GND | GND

{%
	include-markdown "../../inc/uart-power.md"
%}

In order to flash the chip, you need to enable **download mode**. This is similar to ESP8266/ESP32, but the strapping pin (GPIO 0 / PA00) has to be pulled **to 3.3V**, *not GND*.

*Additionally, make sure that pin PA13 (RX0) is NOT pulled to GND.*

{%
	include-markdown "../../inc/uart-cen.md"
%}

Do this, in order:

- connect PA00 to 3.3V
- apply power to the device OR shortly connect CEN to GND
- start the flashing process

If you had a serial terminal (such as PuTTY) open at the time, you should see an `Open Download Mode on UART` message. This means that you've successfully entered download mode and are ready to start flashing.

### Partition layout

{%
	include-markdown "../../inc/partition-layout.md"
%}

### Flashing

{%
	include-markdown "../../inc/flashing.md"
%}

{%
	include-markdown "../../inc/uart-rom.md"
%}

### Auto-download-reboot

{%
	include-markdown "../../inc/uart-adr.md"
%}

## Firmware output files

These files are present in the build directory after successful compilation:

File                           | Description
-------------------------------|----------------------------------------------
**firmware.uf2**               | **UF2 package for UART and OTA upload**
image_flash_is.0x000000.bin    | Complete Flash Image, flashable to 0x0000
image_part_table.0x000000.bin  | Partition Table, flashable to 0x0000
image_bootloader.0x004000.bin  | Bootloader Image, flashable to 0x4000
image_firmware_is.0x00C000.bin | OTA 1/2, flashable to 0xC000 or OTA 2 address
