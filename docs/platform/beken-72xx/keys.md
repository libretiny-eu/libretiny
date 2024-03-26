# Finding encryption keys

## Introduction

!!! tip inline end
	Before proceeding with this method, try using [ltchiptool](../../flashing/tools/ltchiptool.md)'s `Get chip info`
	function. It will read eFuse, which may reveal the raw encryption key. If you see all `00000000`s, then the eFuse
	is readout-protected and the key cannot be extracted in this simple way.

3-rd party firmware for Beken chips must be compiled with a flash encryption key matching the one programmed into
the chip. Incorrect keys will make the firmware unable to run.

The [`bk72xx-bootloader-dump`](https://github.com/libretiny-eu/bk72xx-bootloader-dump) firmware might make it easier
to find the encryption key of BK7231N/BK7231T chips.

The key is made of four 32-bit integers; the default key is usually `510fb093 a3cbeadc 5993a17e c7adeb03`
(used by Beken and Tuya on most devices), but devices with different keys have been recently discovered
(likely from other manufacturers).

If your device doesn't use the default keys (i.e. 3-rd party firmware doesn't boot up, or it hangs on bootloader logs),
you can try using this firmware file to extract the keys from the bootloader.

## Why this works (and when it doesn't)

The bootloader has its own copy of the keys. It uses that to encrypt firmware on-the-fly when applying OTA updates.

Files downloaded during an OTA update are *not* encrypted using the main encryption keys, so the bootloader
must encrypt them before flashing to the app partition. **This method works by flashing firmware directly to the**
**OTA partition**. It is then unpacked and encrypted properly by the bootloader.

However, OTA update packages *are* encrypted using AES - for this method to work, the AES key must be known in advance.

Most of the time, a simple `0123456789ABCDEF` key is used for OTA AES. We have seen manufacturers using different
keys - this method will not work in that case.

Additionally, OTA packages **don't have to** be encrypted - some bootloaders allow that, some don't. Using an
unencrypted package is worth trying if your device uses a non-standard OTA AES key.

## Prerequisites

1. A working computer with a working UART flashing setup. The preferred flashing tool is
[ltchiptool](../../flashing/tools/ltchiptool.md). You should have at least some prior experience with dumping
or flashing firmware.
2. **A full factory firmware dump** of the device you're working on. This is mostly in case something goes wrong,
but may also be necessary to read OTA partition offsets from.
3. A serial terminal (such as the [`ltchiptool-terminal`](https://github.com/libretiny-eu/ltchiptool-terminal) plugin).

## To be continued
