# WR3 Wi-Fi Module

[Product page](https://developer.tuya.com/en/docs/iot/wifiwr1module?id=K9605tc0k90t3)

Parameter    | Value
-------------|-------------------------
MCU          | RTL8710BN
Manufacturer | Realtek
Series       | AmebaZ
Max. clock   | 125 MHz
Flash size   | 2 MiB
RAM size     | 256 KiB
Voltage      | 3.0V - 3.6V
I/O          | 7x GPIO, 2x UART, 1x ADC
Wi-Fi        | 802.11 b/g/n

## Pinout

![image](pinout.png)

## Arduino core pin map

No. | Pin   | Alt. function | PWM
----|-------|---------------|-----
D0  | PA_22 |               | PWM5
D1  | PA_19 |               |
D2  | PA_14 |               | PWM0
D3  | PA_15 |               | PWM1
D4  | PA_0  |               | PWM2
D5  | PA_20 |               |
D6  | PA_29 | UART_Log_RXD  | PWM4
D7  | PA_30 | UART_Log_TXD  | PWM3
D8  | PA_5  |               | PWM4
D9  | PA_12 |               | PWM3
D10 | PA_18 | UART0_RXD     |
D11 | PA_23 | UART0_TXD     | PWM0
A0  | PA_19 |               |
A1  | -     |               |
A2  | PA_20 |               |
