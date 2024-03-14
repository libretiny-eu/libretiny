# Beken 72xx

## Introduction

!!! note inline end "Also read"
	- [Finding encryption keys](keys.md) - what to do if LibreTiny doesn't boot because of incorrect flash encryption keys

Beken BK7231 is a family of Wi-Fi and BLE microcontrollers, of which most popular are BK7231N and BK7231T.

Features:

- ARM968E-S (ARMv5TE) CPU (120 MHz)
- 256 KiB SRAM
- built-in 2 MiB SPI flash with XiP
- 802.11b/g/n Wi-Fi

Resources:

- [BK7231U Datasheet v0.71](https://cdn.discordapp.com/attachments/983843871320580096/1050461302537064508/BK7231Uv0.71.zh-CN.en.pdf) (Machine-translated to English)
- [BL2028N Datasheet v1.0](https://cdn.discordapp.com/attachments/983843871320580096/1050461346111697028/BL2028N_Datasheet_v1.0.pdf) (BL2028N is a "clone" of BK7231N)
- [BK72XX SDK User Manual 3.0.3](https://cdn.discordapp.com/attachments/983843871320580096/1003661237730672730/BK72XX_SDK_User_Manual-3.0.3.pdf) (Machine-translated to English)
- [BEKEN WiFi SDK API Reference 3.0.27](https://cdn.discordapp.com/attachments/983843871320580096/1003661237349003355/BEKEN_WiFi_SDK_API_Reference-3.0.27_compressed.pdf) (Machine-translated to English)
- [Beken SDK documentation (online)](http://docs.bekencorp.com/backup/v3.0/)
- [`encrypt v0.3` source code](https://github.com/ghsecuritylab/tysdk_for_bk7231/tree/master/toolchain/encrypt_crc) (Encryption routines and FPGA code used on the chip)
- [`ota_tools` source code](https://github.com/tiancj/rtt_ota_tools)
- [BK7231 OpenOCD debugging](https://www.elektroda.com/rtvforum/viewtopic.php?p=20028605#20028605) (From Elektroda.pl user `@xabean`)

## Finding your board

{%
	include-markdown "../../inc/find-board.md"
%}

---

## Flashing

{%
	include-markdown "../../inc/flashing-note.md"
%}

BK7231 has two UART ports - UART2 (sometimes called LOG_UART) and UART1. The UART1 port is used for flashing (and external components, such as TuyaMCU) and has no text output. The UART2 port is used for log viewing only.

You need to find which pins correspond to UART1 TX and RX. If your board is supported, you'll find the pinout on its documentation page. Otherwise (and for generic boards), you'll have to find the pinout online.

For best experience, you should have two USB<->UART adapters plugged in:

- One for flashing, preferably a real FT232RL or a good alternative. This connects to UART1 of the chip.
- One for log output - BK72xx outputs messages on a separate port. You can have a terminal session continuously open on this adapter. This connects to UART2 of the chip - but it's not necessary for flashing.

### Wiring

Connect UART1 of the BK7231 to the USB-TTL adapter:

PC  | BK7231
----|-----------------------
RX  | TX1 (GPIO11 / P11)
TX  | RX1 (GPIO10 / P10)
GND | GND

{%
	include-markdown "../../inc/uart-power.md"
%}

{%
	include-markdown "../../inc/uart-cen.md"
%}

The download mode is entered when the chip communicates with the flasher program. Hence, the first step is running the flasher program (described below). While the program is trying to establish communication, **the chip has to be rebooted**. In order to do that, you need to bridge CEN pin to GND with a wire.

Keep in mind that BK7231T (not N) will exit the download mode when it can't communicate with the flasher (or when the flasher finishes its work). It's not possible to forcefully enter download mode without it.

After linking with the chip, the flasher program will begin writing (or reading) the firmware automatically. If that doesn't happen, try resetting the chip again until it works.

If you're getting a `No response received` (or similar) error, this means that:

- the power supply is too weak (read above)
- you're resetting the chip too quickly, i.e. you resetted it *after* the program started communicating with it

### Flashing

The recommended tool to flash (or dump firmware) is `ltchiptool`.

**Read [Using ltchiptool](../../flashing/tools/ltchiptool.md) to learn the flashing procedure**

!!! tip
	BK7231N can't be software-bricked, because it has a ROM that contains the download mode. **BK7231T doesn't contain it, so be careful with this one**.

	`ltchiptool`'s Beken flashing program is based on [bk7231tools](https://github.com/tuya-cloudcutter/bk7231tools). Refer to the guide for information how to use it, but keep in mind that using the ltchiptool GUI is probably just easier.

---

## Auto-download-reboot

{%
	include-markdown "../../inc/uart-adr.md"
%}

## Single UART adapter usage

If you only have a single adapter, or just want to use the UART1 (upload) port only, you can change the logging port of LibreTiny firmware.

Refer to [Options & config](../../dev/config.md) (`Serial output` section). Set `LT_UART_DEFAULT_PORT` to `1`, which will use UART1 for all output.

## Firmware output files

These files are present in the build directory after successful compilation:

File                            | Description
--------------------------------|----------------------------------------
**firmware.uf2**                | **UF2 package for UART and OTA upload**
image_bk7231t_app.ota.rbl       | Beken OTA package (e.g. OpenBeken)
image_bk7231t_app.ota.ug.bin    | Tuya OTA package (incl. Cloudcutter)
image_bk7231t_app.0x011000.rbl  | App partition - flashable at 0x11000
image_bk7231t_app.0x011000.crc  | Encrypted app image - not for flashing
image_bk7231t_app.0x129F0A.rblh | RBL header - not for flashing

## SPI flashing (unbricking BK7231T)

The [bk7231_spi_flasher.py](https://github.com/openshwprojects/BK7231_SPI_Flasher/blob/main/bk7231_spi_flasher.py) script can be used to put BK7231 in SPI flashing mode. Then, one can use [flashrom](https://www.flashrom.org/Flashrom) to read/write the raw flash chip.

## Other tools/guides

These tools are **not recommended** and are kept here for reference only. Don't use them, please.

- [Flashing (Tuya manual)](https://developer.tuya.com/en/docs/iot/burn-and-authorize-wb-series-modules?id=Ka78f4pttsytd)
- [BkWriter v1.6.0](https://images.tuyacn.com/smart/bk_writer1.60/bk_writer1.60.exe)
- [hid_download_py](https://github.com/tiancj/hid_download_py)


## Other info

There are many chip variations in this SoC family:

- BK7231 - marked BK7231QN40, so we're calling it "BK7231Q" to reduce confusion
- BK7231T
- BK7231N
- BK7231S
- BK7231U

The "officially existing" ones are BK7231Q, BK7231N and BK7231U. These are supported by Beken SDKs, such as `bdk_freertos`, although `bk7231s_alios_sdk` also existed at some point.

- BK7231N is substantially different than the other chips, so running T code on N (and vice versa) is not directly possible.
- BK7231Q does not have eFuse **and BLE**
- there are some references to U meaning USB support
- T seems to be exclusive to Tuya boards (that would explain the name); in the T SDK from Tuya, `CFG_SOC_NAME` is set to `SOC_BK7231U`
- T's bootloader greets with `BK7231S_1.0.5` on UART

Regarding `bdk_freertos`:

- `make` allows selecting for which MCU the code should be compiled
- `make bk7231` doesn't compile at all
- `make bk7231u` doesn't run on T with Tuya's bootloader (1.0.5), though it works just fine after replacing the bootloader with one of these included with `bdk_freertos` (1.0.8) (yes, even `bk7231n` bootloader)
- after making a few changes to `driver/entry` and `driver/intc` (so it looks more like the code from T SDK) `bdk_freertos` runs just fine
