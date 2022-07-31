# Generic - BK7231T (Tuya QFN32)

*by Generic*

[Product page](https://kuba2k2.github.io/libretuya/boards/generic-bk7231t-qfn32-tuya/)

- [General info](../../docs/platform/beken-72xx/README.md)
- [Flashing guide](../../docs/platform/beken-72xx/flashing.md)
- [BkWriter v1.6.0](https://images.tuyacn.com/smart/bk_writer1.60/bk_writer1.60.exe)

Parameter    | Value
-------------|----------------------------------
Board code   | `generic-bk7231t-qfn32-tuya`
MCU          | BK7231T
Manufacturer | Beken
Series       | BK72XX
Frequency    | 120 MHz
Flash size   | 2 MiB
RAM size     | 256 KiB
Voltage      | 3.0V - 3.6V
I/O          | 19x GPIO, 6x PWM, 2x UART, 1x ADC
Wi-Fi        | 802.11 b/g/n
Bluetooth    | BLE v4.2

## Usage

**Board code:** `generic-bk7231t-qfn32-tuya`

In `platformio.ini`:

```ini
[env:generic-bk7231t-qfn32-tuya]
platform = libretuya
board = generic-bk7231t-qfn32-tuya
framework = arduino
```

## Arduino Core pin mapping

No. | Pin       | UART     | I²C      | SPI  | PWM  | Other
----|-----------|----------|----------|------|------|------
D0  | P0        | UART2_TX | I2C2_SCL |      |      |
D1  | P1        | UART2_RX | I2C2_SDA |      |      |
D2  | P6        |          |          |      | PWM0 |
D3  | P7        |          |          |      | PWM1 |
D4  | P8        |          |          |      | PWM2 |
D5  | P9        |          |          |      | PWM3 |
D6  | P10       | UART1_RX |          |      |      |
D7  | P11       | UART1_TX |          |      |      |
D8  | P14       |          |          | SCK  |      |
D9  | P15       |          |          | CS   |      |
D10 | P16       |          |          | MOSI |      |
D11 | P17       |          |          | MISO |      |
D12 | P20       |          | I2C1_SCL |      |      | TCK
D13 | P21       |          | I2C1_SDA |      |      | TMS
D14 | P22       |          |          |      |      | TDI
D15 | P23       |          |          |      |      | TDO
D16 | P24       |          |          |      | PWM4 |
D17 | P26       |          |          |      | PWM5 |
D18 | P28       |          |          |      |      |
A0  | P23, ADC3 |          |          |      |      |

## Flash memory map

Flash size: 2 MiB / 2,097,152 B / 0x200000

Hex values are in bytes.

Name            | Start    | Length             | End
----------------|----------|--------------------|---------
Bootloader      | 0x000000 | 68 KiB / 0x11000   | 0x011000
App Image       | 0x011000 | 1.1 MiB / 0x121000 | 0x132000
OTA Image       | 0x132000 | 664 KiB / 0xA6000  | 0x1D8000
Key-Value Store | 0x1D8000 | 32 KiB / 0x8000    | 0x1E0000
TLV Store       | 0x1E0000 | 4 KiB / 0x1000     | 0x1E1000
Network Data    | 0x1E1000 | 8 KiB / 0x2000     | 0x1E3000
User Data       | 0x1E3000 | 116 KiB / 0x1D000  | 0x200000

Bootloader and app partitions contain CRC16 sums every 32 bytes. That results in the actual flash offsets/sizes not aligned to sector boundaries. To simplify calculations, the values shown in the table (extracted from bootloader's partition table) were aligned to 4096 bytes.
