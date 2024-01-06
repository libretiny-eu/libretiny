# Realtek AmebaZ

## Introduction

Realtek AmebaZ is a family of Wi-Fi microcontrollers, primarily consisting of two chips - RTL8710BN and RTL8710BX.

RTL8710BX seems to be the same chip but clocked at 62.5 MHz (instead of 125 MHz for BN). However, it seems that firmware compiled for either of the chips can run on the other with no issues.

Features:

- ARM Cortex M4 CPU (up to 125 MHz)
- 512 KiB ROM
- 256 KiB SRAM
- SPI flash interface with XiP
- 802.11b/g/n Wi-Fi

Resources:

- [Realtek Ameba-Z datasheet v3.4](https://web.archive.org/web/20211203124711if_/https://adelectronicsru.files.wordpress.com/2018/10/um0114-realtek-ameba-z-data-sheet-v3-4.pdf)
- [Realtek product page](https://www.realtek.com/en/products/communications-network-ics/item/rtl8710bn)
- [Realtek PDFs for Ameba1/AmebaZ](https://github.com/ambiot/amb1_sdk/tree/0c8da639b097f01c60e419405aecfafab1d08e43/doc/)
- [Ameba1/AmebaZ SDK](https://github.com/ambiot/amb1_sdk)

## Finding your board

{%
	include-markdown "../../inc/find-board.md"
%}

---

## Flashing

Realtek RTL8710B has two UART ports - UART2 (sometimes called LOG_UART) and UART0. The port used for flashing and viewing logs is UART2.

You need to find which pins correspond to UART2 TX and RX. If your board is supported, you'll find the pinout on its documentation page. Otherwise (and for generic boards), you'll have to find the pinout online.

!!! tip
    You need a good USB<->UART adapter for the process. Some chips may not support 1.5M baud rate,
	required by the ROM for the initial handshake. Widespread PL2303 is currently known not to work,
	at least under Windows. FT232RL is verified to work reliably.

### Wiring

Connect UART2 of the Realtek chip to the USB-TTL adapter:

PC  | RTL8710B
----|--------------------
RX  | TX2 (Log_TX / PA30)
TX  | RX2 (Log_RX / PA29)
GND | GND

{%
	include-markdown "../../inc/uart-power.md"
%}

In order to flash the chip, you need to enable **download mode**. This is done by resetting the chip while pulling down the TX2 pin.

{%
	include-markdown "../../inc/uart-cen.md"
%}

Do this, in order:

- connect CEN to GND
- connect TX2 to GND
- release CEN from GND
- release TX2 from GND

To find out whether download mode is enabled, open a serial terminal (such as PuTTY) on your PC. You should see a few characters printed to the serial console every second (usually some kind of grey blocks, or other non-letter characters).

Note that you will not see any characters before you release TX2 from GND.

### Partition layout

When you compile firmware for Realtek with LibreTiny (either ESPHome or other PlatformIO projects), you need to choose a **board**. Different Realtek boards have different partition layouts - the main difference is the OTA2 firmware address. Choosing a board with wrong address will make it harder to flash OTA updates.

Flashing over UART will update (set) the on-chip OTA address to match the firmware being flashed. **OTA flashing will not update the address** - so make sure to choose the correct board, and **keep using the same board** for OTA flashing.

Using incorrect boards may result in OTA updates having no effect, or (worst case) bricking the device completely.

### Flashing

The recommended tool to flash (or dump firmware) is `ltchiptool`.

**Read [Using ltchiptool](../../flashing/tools/ltchiptool.md) to learn the flashing procedure**

!!! tip
	Because the UART uploading code is programmed in the ROM of the chip, it can't be software-bricked, even if you damage the bootloader.

---

## Auto-download-reboot

{%
	include-markdown "../../inc/uart-adr.md"
%}

## Firmware output files

These files are present in the build directory after successful compilation:

File                    | Description
------------------------|-------------------------------------------------------------------
**firmware.uf2**        | **UF2 package for UART and OTA upload**
image_ota1.0x00B000.bin | OTA 1 image, flashable to 0xB000
image_ota2.0x0D0000.bin | OTA 2 image, flashable to 0xD0000 (the address might be different)

## Other tools/guides

These tools are **not recommended** and are kept here for reference only. Don't use them, please.

- [Flashing (Tuya manual)](https://developer.tuya.com/en/docs/iot/burn-and-authorize-wr-series-modules?id=Ka789pjc581u8)
- [ImageTool (AmebaZ/AmebaD)](https://images.tuyacn.com/smart/Image_Tool/Image_Tool.zip)
- [rtltool.py](https://github.com/libretiny-eu/ltchiptool/blob/master/ltchiptool/soc/ambz/util/rtltool.py)

OTA1/2 files can be flashed using `ImageTool_v2.3.1_AmebaZ(8710b)`. Browse and select one of the files and enter an appropriate address. Select COM port, press `Open` and then `Download`.

This method is not recommended, as it requires you to know the currently enabled OTA index (1 or 2). Flashing the wrong file will either not make any changes, or upload firmware which won't run.
