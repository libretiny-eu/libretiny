# WR2LE Wi-Fi Module

*by Tuya Inc.*

[Product page](https://developer.tuya.com/en/docs/iot/wr2le?id=K9eio9y9e8i8c)

- [General info](../../docs/platform/realtek-amb/README.md)
- [Flashing guide](../../docs/platform/realtek-ambz/flashing.md)
- [Debugging](../../docs/platform/realtek-ambz/debugging.md)

Parameter    | Value
-------------|-------------------------
Board code   | `wr2le`
MCU          | RTL8710BX
Manufacturer | Realtek
Series       | AmebaZ
Frequency    | 62.5 MHz
Flash size   | 2 MiB
RAM size     | 256 KiB
Voltage      | 3.0V - 3.6V
I/O          | 5x GPIO, 5x PWM, 1x UART
Wi-Fi        | 802.11 b/g/n

## Usage

**Board code:** `wr2le`

In `platformio.ini`:

```ini
[env:wr2le]
platform = libretuya
board = wr2le
framework = arduino
```

In ESPHome YAML:

```yaml
libretuya:
  board: wr2le
  framework:
    version: dev
```

## Pinout

![Pinout](pinout_wr2le.svg)

## Arduino Core pin mapping

No. | Pin  | UART      | I²C      | SPI                  | PWM  | Other
----|------|-----------|----------|----------------------|------|------
D0  | PA15 |           |          |                      | PWM1 | SWDIO
D1  | PA14 |           |          |                      | PWM0 | SWCLK
D2  | PA05 |           |          |                      | PWM4 |
D3  | PA22 | UART0_RTS | I2C0_SCL | SPI0_MISO, SPI1_MISO | PWM5 |
D4  | PA12 |           |          |                      | PWM3 |

## Flash memory map

Flash size: 2 MiB / 2,097,152 B / 0x200000

Hex values are in bytes.

Name            | Start    | Length            | End
----------------|----------|-------------------|---------
Boot XIP        | 0x000000 | 16 KiB / 0x4000   | 0x004000
Boot RAM        | 0x004000 | 16 KiB / 0x4000   | 0x008000
(reserved)      | 0x008000 | 4 KiB / 0x1000    | 0x009000
System Data     | 0x009000 | 4 KiB / 0x1000    | 0x00A000
Calibration     | 0x00A000 | 4 KiB / 0x1000    | 0x00B000
OTA1 Image      | 0x00B000 | 788 KiB / 0xC5000 | 0x0D0000
OTA2 Image      | 0x0D0000 | 788 KiB / 0xC5000 | 0x195000
Key-Value Store | 0x195000 | 24 KiB / 0x6000   | 0x19B000
User Data       | 0x19B000 | 400 KiB / 0x64000 | 0x1FF000
RDP             | 0x1FF000 | 4 KiB / 0x1000    | 0x200000

RDP is most likely not used in Tuya firmwares, as the System Data partition contains an incorrect offset 0xFF000 for RDP, which is in the middle of OTA2 image.

Additionally, Tuya firmware uses an encrypted KV or file storage, which resides at the end of flash memory. This seems to overlap system RDP area.
