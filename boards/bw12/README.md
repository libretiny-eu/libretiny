# BW12

*by Ai-Thinker Co., Ltd.*

[Product page](http://www.ai-thinker.com/pro_view-13.html)

- [General info](../../docs/platform/realtek-amb/README.md)
- [Flashing guide](../../docs/platform/realtek-ambz/flashing.md)
- [Debugging](../../docs/platform/realtek-ambz/debugging.md)
- [Vendor datasheet](https://docs.ai-thinker.com/_media/rtl8710/hardware/bw12_datasheet_en.pdf)

Parameter    | Value
-------------|------------------------------------------
Board code   | `bw12`
MCU          | RTL8710BX
Manufacturer | Realtek
Series       | AmebaZ
Frequency    | 62.5 MHz
Flash size   | 2 MiB
RAM size     | 256 KiB
Voltage      | 3.0V - 3.6V
I/O          | 11x GPIO, 6x PWM, 2x UART, 1x ADC
Wi-Fi        | 802.11 b/g/n
FCC ID       | [2ARI3-BW1X](https://fccid.io/2ARI3-BW1X)

## Usage

**Board code:** `bw12`

In `platformio.ini`:

```ini
[env:bw12]
platform = libretuya
board = bw12
framework = arduino
```

In ESPHome YAML:

```yaml
libretuya:
  board: bw12
  framework:
    version: dev
```

## Pinout

![Pinout](pinout_bw12.svg)

## Arduino Core pin mapping

No. | Pin        | UART      | I²C      | SPI                  | PWM  | Other
----|------------|-----------|----------|----------------------|------|------
D0  | PA05       |           |          |                      | PWM4 |
D1  | PA29       | UART2_RX  | I2C0_SCL |                      | PWM4 |
D2  | PA00       |           |          |                      | PWM2 |
D3  | PA19       | UART0_CTS | I2C0_SDA | SPI0_CS, SPI1_CS     |      |
D4  | PA22       | UART0_RTS | I2C0_SCL | SPI0_MISO, SPI1_MISO | PWM5 |
D5  | PA30       | UART2_TX  | I2C0_SDA |                      | PWM4 |
D6  | PA14       |           |          |                      | PWM0 | SWCLK
D7  | PA12       |           |          |                      | PWM3 |
D8  | PA15       |           |          |                      | PWM1 | SWDIO
D9  | PA18       | UART0_RX  | I2C1_SCL | SPI0_SCK, SPI1_SCK   |      |
D10 | PA23       | UART0_TX  | I2C1_SDA | SPI0_MOSI, SPI1_MOSI | PWM0 |
A0  | PA19, ADC1 |           |          |                      |      |

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
OTA1 Image      | 0x00B000 | 468 KiB / 0x75000 | 0x080000
OTA2 Image      | 0x080000 | 468 KiB / 0x75000 | 0x0F5000
Key-Value Store | 0x0F5000 | 24 KiB / 0x6000   | 0x0FB000
User Data       | 0x0FB000 | 1 MiB / 0x104000  | 0x1FF000
RDP             | 0x1FF000 | 4 KiB / 0x1000    | 0x200000

RDP is most likely not used in Tuya firmwares, as the System Data partition contains an incorrect offset 0xFF000 for RDP, which is in the middle of OTA2 image.

Additionally, Tuya firmware uses an encrypted KV or file storage, which resides at the end of flash memory. This seems to overlap system RDP area.
