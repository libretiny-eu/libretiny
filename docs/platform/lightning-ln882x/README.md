# Lightning LN882x

## Introduction

Lightning LN882x is a family of Wi-Fi and BLE microcontrollers designed for indoor short-range IoT.

Features:

- 32-bit Cortex M4F CPU (160 MHz)
- 296 KiB SRAM
- built-in 512 KiB to 2 MiB flash with XiP
- 802.11b/g/n Wi-Fi
- Bluetooth 5.1 (LN882H only)

Resources:

- [LN882H](http://www.lightningsemi.com/ln882h)  : MCU Combo Wi-Fi/BLE IoT
- [LN882H Datasheet](https://gitee.com/lightningsemi/ln882h-document-collection/blob/master/1.%E8%8A%AF%E7%89%87%E8%A7%84%E6%A0%BC%E4%B9%A6/LN882H_Datasheet.pdf)  : LN882H Datasheet
- [LN882H DOC](https://gitee.com/lightningsemi/ln882h-document-collection)  : Official LN882H Document Collection
- [LN882H SDK](https://gitee.com/lightningsemi/ln882h)  : Official LN882H SDK
- [LN882X SDK](https://gitee.com/lightningsemi/ln882x-sdk)  : Official LN882X SDK
- [LN8825A/B](http://www.lightningsemi.com/LN8825AB) : Wi-Fi SOC
- [LN8825A/B SDK](https://gitee.com/lightningsemi/ln882x-mcu) : Official LN8825A/B SDK
- [Flashing tool (GUI)](https://gitee.com/lightningsemi/ln882h-document-collection/blob/master/4.%E7%83%A7%E5%BD%95%E5%B7%A5%E5%85%B7/%E4%B8%B2%E5%8F%A3%E7%83%A7%E5%BD%95/%E4%BA%AE%E7%89%9B%E4%B8%B2%E5%8F%A3%E7%83%A7%E5%BD%95%E5%B7%A5%E5%85%B7V3.0.5.zip) : Flashing tool with GUI (chinese)
- [Flashing tool (CMD)](https://gitee.com/lightningsemi/ln882h-document-collection/blob/master/4.%E7%83%A7%E5%BD%95%E5%B7%A5%E5%85%B7/%E4%B8%B2%E5%8F%A3%E7%83%A7%E5%BD%95/LN882H%E5%91%BD%E4%BB%A4%E8%A1%8C%E5%B7%A5%E5%85%B7_V1.0.16.zip) : Command-line Flashing tool

## Finding your board

{%
	include-markdown "../../inc/find-board.md"
%}

---

## Flashing

{%
	include-markdown "../../inc/flashing-note.md"
%}

LN882x has two UART ports - The UART0 port is used for flashing and text output.

You need to find which pins correspond to UART0 TX and RX. If your board is supported, you'll find the pinout on its documentation page. Otherwise (and for generic boards), you'll have to find the pinout online.

### Wiring

Connect UART0 of the LN882H to the USB-TTL adapter:

PC  | LN882H
----|-----------------------
RX  | TX0 (GPIOA2 / P2)
TX  | RX0 (GPIOA3 / P3)
GND | GND

{%
	include-markdown "../../inc/uart-power.md"
%}

{%
	include-markdown "../../inc/uart-cen.md"
%}

The download mode is entered when the chip communicates with the flasher program. Hence, the first step is running the flasher program (described below). While the program is trying to establish communication, **the chip has to be rebooted**. In order to do that, you need to bridge BOOT1 pin to GND with a wire **before** powering the chip.

After linking with the chip, the flasher program will begin writing (or reading) the firmware automatically. If that doesn't happen, try resetting the chip again until it works.

If you're getting a `No response received` (or similar) error, this means that:

- the power supply is too weak (read above)
- you're resetting the chip too quickly, i.e. you resetted it *after* the program started communicating with it

### Flashing

The recommended tool to flash (or dump firmware) is `ltchiptool`.

**Read [Using ltchiptool](../../flashing/tools/ltchiptool.md) to learn the flashing procedure**

!!! tip
	Because the UART uploading code is programmed in the ROM of the chip, it can't be software-bricked, even if you damage the bootloader.

## Firmware output files

These files are present in the build directory after successful compilation:

File                            | Description
--------------------------------|----------------------------------------
**firmware.uf2**                | **UF2 package for UART and OTA upload**
**image_firmware.0x000000.bin** | **Full flash binary image** - flashable at 0x0
image_boot.0x000000.bin         | Boot partition binary image - flashable at 0x0
image_part_tab.0x006000.bin     | Partition table binary image - flashable at 0x6000
image_app.0x007000.bin          | Application partition binary image - flashable at 0x7000
image_ota.0x133000.bin          | OTA partition binary image - flashable at 0x133000


## Other tools/guides

- Official command-line [flashing tool](https://gitee.com/lightningsemi/ln882h-document-collection/blob/master/4.%E7%83%A7%E5%BD%95%E5%B7%A5%E5%85%B7/%E4%B8%B2%E5%8F%A3%E7%83%A7%E5%BD%95/LN882H%E5%91%BD%E4%BB%A4%E8%A1%8C%E5%B7%A5%E5%85%B7_V1.0.16.zip)

	 Usage Examples:<br/>
	 `LN882H_CMD_Tool.exe COM1 download flash 921600 0x0 image_firmware.0x000000.bin`<br/>
	 `LN882H_CMD_Tool.exe COM1 download flash 921600 0x7000 image_app.0x007000.bin`
