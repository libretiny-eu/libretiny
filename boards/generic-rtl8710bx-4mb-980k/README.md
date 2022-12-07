# Generic - RTL8710BX (4M/980k)

*by Generic*

[Product page](https://kuba2k2.github.io/libretuya/boards/generic-rtl8710bx-4mb-980k/)

- [General info](../../docs/platform/realtek/README.md)
- [Debugging](../../docs/platform/realtek/debugging.md)
- [Flashing guide](../../docs/platform/realtek-ambz/flashing.md)
- [ImageTool (AmebaZ/AmebaD)](https://images.tuyacn.com/smart/Image_Tool/Image_Tool.zip)

Parameter    | Value
-------------|----------------------------------
Board code   | `generic-rtl8710bx-4mb-980k`
MCU          | RTL8710BX
Manufacturer | Realtek
Series       | AmebaZ
Frequency    | 62.5 MHz
Flash size   | 4 MiB
RAM size     | 256 KiB
Voltage      | 3.0V - 3.6V
I/O          | 17x GPIO, 6x PWM, 2x UART, 1x ADC
Wi-Fi        | 802.11 b/g/n

## Usage

**Board code:** `generic-rtl8710bx-4mb-980k`

In `platformio.ini`:

```ini
[env:generic-rtl8710bx-4mb-980k]
platform = libretuya
board = generic-rtl8710bx-4mb-980k
framework = arduino
```

In ESPHome YAML:

```yaml
libretuya:
  board: generic-rtl8710bx-4mb-980k
  framework:
    version: dev
```

## Arduino Core pin mapping

No. | Pin        | UART      | I²C      | SPI                  | PWM  | Other
----|------------|-----------|----------|----------------------|------|------
D0  | PA00       |           |          |                      | PWM2 |
D1  | PA05       |           |          |                      | PWM4 |
D2  | PA06       |           |          | FCS                  |      |
D3  | PA07       |           |          | FD1                  |      |
D4  | PA08       |           |          | FD2                  |      |
D5  | PA09       |           |          | FD0                  |      |
D6  | PA10       |           |          | FSCK                 |      |
D7  | PA11       |           |          | FD3                  |      |
D8  | PA12       |           |          |                      | PWM3 |
D9  | PA14       |           |          |                      | PWM0 | SWCLK
D10 | PA15       |           |          |                      | PWM1 | SWDIO
D11 | PA18       | UART0_RX  | I2C1_SCL | SPI0_SCK, SPI1_SCK   |      |
D12 | PA19       | UART0_CTS | I2C0_SDA | SPI0_CS, SPI1_CS     |      |
D13 | PA22       | UART0_RTS | I2C0_SCL | SPI0_MISO, SPI1_MISO | PWM5 |
D14 | PA23       | UART0_TX  | I2C1_SDA | SPI0_MOSI, SPI1_MOSI | PWM0 |
D15 | PA29       | UART2_RX  | I2C0_SCL |                      | PWM4 |
D16 | PA30       | UART2_TX  | I2C0_SDA |                      | PWM4 |
A0  | PA19, ADC1 |           |          |                      |      |

## Flash memory map

Flash size: 4 MiB / 4,194,304 B / 0x400000

Hex values are in bytes.

Name            | Start    | Length            | End
----------------|----------|-------------------|---------
Boot XIP        | 0x000000 | 16 KiB / 0x4000   | 0x004000
Boot RAM        | 0x004000 | 16 KiB / 0x4000   | 0x008000
(reserved)      | 0x008000 | 4 KiB / 0x1000    | 0x009000
System Data     | 0x009000 | 4 KiB / 0x1000    | 0x00A000
Calibration     | 0x00A000 | 4 KiB / 0x1000    | 0x00B000
OTA1 Image      | 0x00B000 | 980 KiB / 0xF5000 | 0x100000
OTA2 Image      | 0x100000 | 980 KiB / 0xF5000 | 0x1F5000
Key-Value Store | 0x1F5000 | 32 KiB / 0x8000   | 0x1FD000
User Data       | 0x1FD000 | 2 MiB / 0x202000  | 0x3FF000
RDP             | 0x3FF000 | 4 KiB / 0x1000    | 0x400000

RDP is most likely not used in Tuya firmwares, as the System Data partition contains an incorrect offset 0xFF000 for RDP, which is in the middle of OTA2 image.

Additionally, Tuya firmware uses an encrypted KV or file storage, which resides at the end of flash memory. This seems to overlap system RDP area.

## Information

This is a generic board definition for RTL8710BX with 4 MiB of flash. It has a bigger application partition size (980 KiB). The used bootloader is also different from the standard Tuya one.

It can be found in [Ezviz T31 smart plug](https://www.ezviz.com/product/T31/2021) - bare chip soldered onto the manufacturer-made PCB. The plug is not Tuya/SmartLife-compatible and has a 25Q32CSIG flash chip. Refer to [libretuya#23](https://github.com/kuba2k2/libretuya/issues/23) for photos and more information.

Note that stock firmware seems to use smaller app images (0x80000 / 512 KiB). After 0x180000 some product-test data and device logs can be found. Because the OTA2 offset is 0x100000, the board definition was configured to use all available space.
