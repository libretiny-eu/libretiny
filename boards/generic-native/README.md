# Generic - Host-native

*by N/A*

[Product page](https://kuba2k2.github.io/libretuya/)

Parameter    | Value
-------------|-------
MCU          | NATIVE
Manufacturer | N/A
Series       | N/A
Frequency    | 1 GHz
Flash size   | 4 MiB
RAM size     | 4 MiB
Voltage      | 5V

## Flash memory map

Flash size: 4 MiB / 4,194,304 B / 0x400000

Hex values are in bytes.

Name            | Start    | Length            | End
----------------|----------|-------------------|---------
Bootloader      | 0x000000 | 64 KiB / 0x10000  | 0x010000
System Data     | 0x010000 | 64 KiB / 0x10000  | 0x020000
OTA1 Image      | 0x020000 | 1 MiB / 0x100000  | 0x120000
OTA2 Image      | 0x120000 | 1 MiB / 0x100000  | 0x220000
OTA Image       | 0x220000 | 1 MiB / 0x100000  | 0x320000
Key-Value Store | 0x320000 | 256 KiB / 0x40000 | 0x360000
User Data       | 0x360000 | 640 KiB / 0xA0000 | 0x400000

## Description

`generic-native` is a dummy board using the `host-native` family, which also serves as a template for building new families, as well as for testing flash-related modules (i.e. Fat FS, OTA, etc).
