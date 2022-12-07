# LSC LMA35

*by Unknown*

[Product page](https://www.action.com/de-at/p/lsc-smart-connect-outdoor-led-streifen/)

- [General info](../../docs/platform/beken-72xx/README.md)
- [Flashing guide](../../docs/platform/beken-72xx/flashing.md)
- [BkWriter v1.6.0](https://images.tuyacn.com/smart/bk_writer1.60/bk_writer1.60.exe)

Parameter    | Value
-------------|----------------------------------
Board code   | `lsc-lma35`
MCU          | BK7231N
Manufacturer | Beken
Series       | BK72XX
Frequency    | 120 MHz
Flash size   | 2 MiB
RAM size     | 256 KiB
Voltage      | 3.0V - 3.6V
I/O          | 15x GPIO, 6x PWM, 2x UART, 1x ADC
Wi-Fi        | 802.11 b/g/n
Bluetooth    | BLE v5.1

## Usage

**Board code:** `lsc-lma35`

In `platformio.ini`:

```ini
[env:lsc-lma35]
platform = libretuya
board = lsc-lma35
framework = arduino
```

In ESPHome YAML:

```yaml
libretuya:
  board: lsc-lma35
  framework:
    version: dev
```

## Pinout

![Pinout](pinout_lsc-lma35.svg)

## Arduino Core pin mapping

No. | Pin       | UART     | I²C      | SPI  | PWM  | Other
----|-----------|----------|----------|------|------|------
D0  | P26       |          |          |      | PWM5 |
D1  | P14       |          |          | SCK  |      |
D2  | P16       |          |          | MOSI |      |
D3  | P24       |          |          |      | PWM4 |
D4  | P22       |          |          |      |      | TDI
D5  | P0        | UART2_TX | I2C2_SCL |      |      |
D6  | P23       |          |          |      |      | TDO
D7  | P8        |          |          |      | PWM2 |
D8  | P9        |          |          |      | PWM3 |
D9  | P21       |          | I2C1_SDA |      |      | TMS
D10 | P6        |          |          |      | PWM0 |
D11 | P7        |          |          |      | PWM1 |
D12 | P10       | UART1_RX |          |      |      |
D13 | P11       | UART1_TX |          |      |      |
D14 | P1        | UART2_RX | I2C2_SDA |      |      |
A0  | P23, ADC3 |          |          |      |      |

## Flash memory map

Flash size: 2 MiB / 2,097,152 B / 0x200000

Hex values are in bytes.

Name            | Start    | Length             | End
----------------|----------|--------------------|---------
Bootloader      | 0x000000 | 68 KiB / 0x11000   | 0x011000
App Image       | 0x011000 | 1.1 MiB / 0x119000 | 0x12A000
OTA Image       | 0x12A000 | 664 KiB / 0xA6000  | 0x1D0000
Calibration     | 0x1D0000 | 4 KiB / 0x1000     | 0x1D1000
Network Data    | 0x1D1000 | 4 KiB / 0x1000     | 0x1D2000
TLV Store       | 0x1D2000 | 4 KiB / 0x1000     | 0x1D3000
Key-Value Store | 0x1D3000 | 32 KiB / 0x8000    | 0x1DB000
User Data       | 0x1DB000 | 148 KiB / 0x25000  | 0x200000

Bootloader and app partitions contain CRC16 sums every 32 bytes. That results in the actual flash offsets/sizes not aligned to sector boundaries. To simplify calculations, the values shown in the table (extracted from bootloader's partition table) were aligned to 4096 bytes.

## Information

This board has no marking on the front side, only something that looks like PCB manufacturing info on the back; thus it was named based on these symbols.

It can be found in 'LSC Smart Connect Outdoor LED Strip', and is likely custom-made for this product.

The pinout was established by writing to and probing consecutive GPIOs, using the generic board definition.

Pins marked with '?' are currently unknown, with a possibility of being CEN. Pin 22 (P1/D14) is also not confirmed.
