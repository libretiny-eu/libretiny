# Generic - BK7252

*by Generic*

[Product page](https://kuba2k2.github.io/libretuya/boards/generic-bk7252/)

- [General info](../../docs/platform/beken-72xx/README.md)
- [Flashing guide](../../docs/platform/beken-72xx/flashing.md)
- [BkWriter v1.6.0](https://images.tuyacn.com/smart/bk_writer1.60/bk_writer1.60.exe)

Parameter    | Value
-------------|----------------------------------
Board code   | `generic-bk7252`
MCU          | BK7252
Manufacturer | Beken
Series       | BK72XX
Frequency    | 180 MHz
Flash size   | 4 MiB
RAM size     | 512 KiB
Voltage      | 3.0V - 3.6V
I/O          | 38x GPIO, 4x PWM, 2x UART, 7x ADC
Wi-Fi        | 802.11 b/g/n
Bluetooth    | BLE v5.0

## Usage

**Board code:** `generic-bk7252`

In `platformio.ini`:

```ini
[env:generic-bk7252]
platform = libretuya
board = generic-bk7252
framework = arduino
```

In ESPHome YAML:

```yaml
libretuya:
  board: generic-bk7252
  framework:
    version: dev
```

## Arduino Core pin mapping

No. | Pin       | UART      | I²C      | SPI  | PWM  | Other
----|-----------|-----------|----------|------|------|----------
D0  | P0        | UART2_TX  | I2C2_SCL |      |      |
D1  | P1        | UART2_RX  | I2C2_SDA |      |      |
D2  | P2        |           |          |      |      |
D3  | P3        |           |          |      |      |
D4  | P4        |           |          |      |      |
D5  | P5        |           |          |      |      |
D6  | P6        |           |          |      | PWM0 |
D7  | P7        |           |          |      | PWM1 |
D8  | P10       | UART1_RX  |          |      |      |
D9  | P11       | UART1_TX  |          |      |      |
D10 | P12       | UART1_CTS |          |      |      |
D11 | P13       | UART1_RTS |          |      |      |
D12 | P14       |           |          | SCK  |      |
D13 | P15       |           |          | CS   |      |
D14 | P16       |           |          | MOSI |      |
D15 | P17       |           |          | MISO |      |
D16 | P18       |           |          |      |      |
D17 | P19       |           |          |      |      |
D18 | P20       |           | I2C1_SCL |      |      | TCK
D19 | P21       |           | I2C1_SDA |      |      | TMS
D20 | P22       |           |          |      |      | TDI
D21 | P23       |           |          |      |      | TDO
D22 | P24       |           |          |      | PWM4 |
D23 | P25       |           |          |      |      |
D24 | P26       |           |          |      | PWM5 |
D25 | P27       |           |          |      |      | DVP_MCLK
D26 | P28       |           |          |      |      |
D27 | P29       |           |          |      |      | DVP_PCLK
D28 | P30       |           |          |      |      | DVP_HSYNC
D29 | P31       |           |          |      |      | DVP_VSYNC
D30 | P32       |           |          |      |      | DVP_PD0
D31 | P33       |           |          |      |      | DVP_PD1
D32 | P34       |           |          |      |      | DVP_PD2
D33 | P35       |           |          |      |      | DVP_PD3
D34 | P36       |           |          |      |      | DVP_PD4
D35 | P37       |           |          |      |      | DVP_PD5
D36 | P38       |           |          |      |      | DVP_PD6
D37 | P39       |           |          |      |      | DVP_PD7
A1  | P4, ADC1  |           |          |      |      |
A2  | P5, ADC2  |           |          |      |      |
A3  | P23, ADC3 |           |          |      |      |
A4  | P3, ADC5  |           |          |      |      |
A5  | P2, ADC4  |           |          |      |      |
A6  | P12, ADC6 |           |          |      |      |
A7  | P13, ADC7 |           |          |      |      |

## Flash memory map

Flash size: 4 MiB / 4,194,304 B / 0x400000

Hex values are in bytes.

Name            | Start    | Length             | End
----------------|----------|--------------------|---------
Bootloader      | 0x000000 | 68 KiB / 0x11000   | 0x011000
App Image       | 0x011000 | 1.7 MiB / 0x1BA000 | 0x1CB000
File System     | 0x1CB000 | 1.1 MiB / 0x119000 | 0x2E4000
Key-Value Store | 0x2E4000 | 32 KiB / 0x8000    | 0x2EC000
OTA Image       | 0x2EC000 | 1.1 MiB / 0x112000 | 0x3FE000
Calibration     | 0x3FE000 | 4 KiB / 0x1000     | 0x3FF000
TLV Store       | 0x3FF000 | 4 KiB / 0x1000     | 0x400000

Bootloader and app partitions contain CRC16 sums every 32 bytes. That results in the actual flash offsets/sizes not aligned to sector boundaries. To simplify calculations, the values shown in the table (extracted from bootloader's partition table) were aligned to 4096 bytes.
