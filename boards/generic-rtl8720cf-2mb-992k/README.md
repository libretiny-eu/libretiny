# Generic - RTL8720CF (2M/992k)

*by Generic*

[Product page](https://kuba2k2.github.io/libretuya/boards/generic-rtl8720cf-2mb-992k/)

- [General info](../../docs/platform/realtek-amb/README.md)

Parameter    | Value
-------------|-----------------------------
Board code   | `generic-rtl8720cf-2mb-992k`
MCU          | RTL8720CF
Manufacturer | Realtek
Series       | AmebaZ2
Frequency    | 100 MHz
Flash size   | 2 MiB
RAM size     | 256 KiB
Voltage      | 3.0V - 3.6V
I/O          | 20x GPIO, 8x PWM, 3x UART
Wi-Fi        | 802.11 b/g/n
BLE          | v4.2

## Usage

**Board code:** `generic-rtl8720cf-2mb-992k`

In `platformio.ini`:

```ini
[env:generic-rtl8720cf-2mb-992k]
platform = libretuya
board = generic-rtl8720cf-2mb-992k
framework = arduino
```

In ESPHome YAML:

```yaml
libretuya:
  board: generic-rtl8720cf-2mb-992k
  framework:
    version: dev
```

## Flash memory map

Flash size: 2 MiB / 2,097,152 B / 0x200000

Hex values are in bytes.

Name            | Start    | Length            | End
----------------|----------|-------------------|---------
Partition Table | 0x000000 | 4 KiB / 0x1000    | 0x001000
System Data     | 0x001000 | 4 KiB / 0x1000    | 0x002000
Calibration     | 0x002000 | 4 KiB / 0x1000    | 0x003000
(reserved)      | 0x003000 | 4 KiB / 0x1000    | 0x004000
Boot Image      | 0x004000 | 32 KiB / 0x8000   | 0x00C000
OTA1 Image      | 0x00C000 | 992 KiB / 0xF8000 | 0x104000
OTA2 Image      | 0x104000 | 992 KiB / 0xF8000 | 0x1FC000
Key-Value Store | 0x1FC000 | 8 KiB / 0x2000    | 0x1FE000
User Data       | 0x1FE000 | 8 KiB / 0x2000    | 0x200000
