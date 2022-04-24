# LibreTuya

PlatformIO development platform for IoT modules manufactured by Tuya Inc.

The main goal of this project is to provide a usable build environment for IoT developers. While also providing vendor SDKs as PlatformIO cores,
the project focuses on developing working Arduino-compatible cores for supported platforms. The cores are inspired by Espressif's official core for ESP32,
which should make it easier to port/run existing ESP apps on Tuya IoT (and 3-rd party) platforms.

LibreTuya also provides a common interface for all platform implementations. The interface is based on ESP32 official libraries.

**Note:** this project is work-in-progress.

## Board List

A (mostly) complete* list of Tuya wireless module boards.

&nbsp;                        | Module Name                                                                                    | MCU                     | Flash | RAM      | Pins**      | Wi-Fi | BLE | Platform name
------------------------------|------------------------------------------------------------------------------------------------|-------------------------|-------|----------|-------------|-------|-----|---------------
❌                             | [WB1S](https://developer.tuya.com/en/docs/iot/wb1s?id=K9duevbj3ol4x)                           | BK7231T @ 120 MHz       | 2 MiB | 256 KiB  | 18 (11 I/O) | ✔️    | ✔️  | -
❌                             | [WB2L](https://developer.tuya.com/en/docs/iot/wb2l-datasheet?id=K9duegc9bualu)                 | BK7231T @ 120 MHz       | 2 MiB | 256 KiB  | 7 (5 I/O)   | ✔️    | ✔️  | -
❌                             | [WB2S](https://developer.tuya.com/en/docs/iot/wb2s-module-datasheet?id=K9ghecl7kc479)          | BK7231T @ 120 MHz       | 2 MiB | 256 KiB  | 11 (8 I/O)  | ✔️    | ✔️  | -
❌                             | [WB3L](https://developer.tuya.com/en/docs/iot/wb3l-module-datasheet?id=K9duiggw2v8sp)          | BK7231T @ 120 MHz       | 2 MiB | 256 KiB  | 16 (12 I/O) | ✔️    | ✔️  | -
❌                             | [WB3S](https://developer.tuya.com/en/docs/iot/wb3s-module-datasheet?id=K9dx20n6hz5n4)          | BK7231T @ 120 MHz       | 2 MiB | 256 KiB  | 16 (11 I/O) | ✔️    | ✔️  | -
❌                             | [WB3S-IPEX](https://developer.tuya.com/en/docs/iot/wb3sipex-module-datasheet?id=K9irq0laun21z) | BK7231T @ 120 MHz       | 2 MiB | 256 KiB  | 16 (11 I/O) | ✔️    | ✔️  | -
❌                             | [WB8P](https://developer.tuya.com/en/docs/iot/wb8p-module-datasheet?id=K9fwx4f89tvzd)          | BK7231T @ 120 MHz       | 2 MiB | 256 KiB  | 10 (8 I/O)  | ✔️    | ✔️  | -
❌                             | [WBLC5](https://developer.tuya.com/en/docs/iot/wblc5-module-datasheet?id=K9duilns1f3gi)        | BK7231T @ 120 MHz       | 2 MiB | 256 KiB  | 6 (3 I/O)   | ✔️    | ✔️  | -
❌                             | [WBLC9](https://developer.tuya.com/en/docs/iot/wblc9-module-datasheet?id=K9hgglry2jp5h)        | BK7231T @ 120 MHz       | 2 MiB | 256 KiB  | 8 (6 I/O)   | ✔️    | ✔️  | -
❌                             | [CB1S](https://developer.tuya.com/en/docs/iot/cb1s-module-datasheet?id=Kaij1abmwyjq2)          | BK7231N @ 120 MHz       | 2 MiB | 256 KiB  | 18 (11 I/O) | ✔️    | ✔️  | -
❌                             | [CB2L](https://developer.tuya.com/en/docs/iot/cb2l-module-datasheet?id=Kai2eku1m3pyl)          | BK7231N @ 120 MHz       | 2 MiB | 256 KiB  | 7 (5 I/O)   | ✔️    | ✔️  | -
❌                             | [CB2S](https://developer.tuya.com/en/docs/iot/cb2s-module-datasheet?id=Kafgfsa2aaypq)          | BK7231N @ 120 MHz       | 2 MiB | 256 KiB  | 11 (8 I/O)  | ✔️    | ✔️  | -
❌                             | [CB3L](https://developer.tuya.com/en/docs/iot/cb3l-module-datasheet?id=Kai51ngmrh3qm)          | BK7231N @ 120 MHz       | 2 MiB | 256 KiB  | 16 (11 I/O) | ✔️    | ✔️  | -
❌                             | [CB3S](https://developer.tuya.com/en/docs/iot/cb3s?id=Kai94mec0s076)                           | BK7231N @ 120 MHz       | 2 MiB | 256 KiB  | 22 (14 I/O) | ✔️    | ✔️  | -
❌                             | [CB3S-NL](https://developer.tuya.com/en/docs/iot/CB3S-NL-module-datasheet?id=Kbaesan0vyoms)    | BK7231NL (?) @ 120 MHz  | 2 MiB | 256 KiB  | 22 (14 I/O) | ✔️    | ✔️  | -
❌                             | [CB3SE](https://developer.tuya.com/en/docs/iot/CB3SE-Module-Datasheet?id=Kanoiluul7nl2)        | BK7231N @ 120 MHz       | 2 MiB | 256 KiB  | 22 (17 I/O) | ✔️    | ✔️  | -
❌                             | [CB8P](https://developer.tuya.com/en/docs/iot/cb8p-module-datasheet?id=Kahvig14r1yk9)          | BK7231N @ 120 MHz       | 2 MiB | 256 KiB  | 10 (8 I/O)  | ✔️    | ✔️  | -
❌                             | [CBLC5](https://developer.tuya.com/en/docs/iot/cblc5-module-datasheet?id=Ka07iqyusq1wm)        | BK7231N @ 120 MHz       | 2 MiB | 256 KiB  | 6 (3 I/O)   | ✔️    | ✔️  | -
❌                             | [CBLC9](https://developer.tuya.com/en/docs/iot/cblc9-module-datasheet?id=Ka42cqnj9r0i5)        | BK7231N @ 120 MHz       | 2 MiB | 256 KiB  | 8 (6 I/O)   | ✔️    | ✔️  | -
❌                             | [CBU](https://developer.tuya.com/en/docs/iot/cbu-module-datasheet?id=Ka07pykl5dk4u)            | BK7231N @ 120 MHz       | 2 MiB | 256 KiB  | 21 (18 I/O) | ✔️    | ✔️  | -
❌                             | [CBU-IPEX](https://developer.tuya.com/en/docs/iot/cbuipex-module-datasheet?id=Kaedsyurckrhu)   | BK7231N @ 120 MHz       | 2 MiB | 256 KiB  | 21 (18 I/O) | ✔️    | ✔️  | -
❌                             | [CBU-NL](https://developer.tuya.com/en/docs/iot/CBU-NL-module-datasheet?id=Kbaeq6j53y0yg)      | BK7231N (?) @ 120 MHz   | 2 MiB | 256 KiB  | 21 (18 I/O) | ✔️    | ✔️  | -
❌                             | [WR1](https://developer.tuya.com/en/docs/iot/wifiwr1module?id=K9605tc0k90t3)                   | RTL8710BN @ 125 MHz     | 1 MiB | 256 KiB  | 18 (11 I/O) | ✔️    | ❌   | -
❌                             | [WR1-IPEX](https://developer.tuya.com/en/docs/iot/wifiwr1ipexmodule?id=K9605t977tx5u)          | RTL8710BN @ 125 MHz     | 1 MiB | 256 KiB  | 18 (11 I/O) | ✔️    | ❌   | -
❌                             | [WR1E](https://developer.tuya.com/en/docs/iot/wr1e?id=K96smbbeycxtf)                           | RTL8710BN @ 125 MHz     | 2 MiB | 256 KiB  | 18 (11 I/O) | ✔️    | ❌   | -
❌                             | [WR2](https://developer.tuya.com/en/docs/iot/wifiwr2module?id=K9605tko0juc3)                   | RTL8710BN @ 125 MHz     | 2 MiB | 256 KiB  | 11 (8 I/O)  | ✔️    | ❌   | -
❌                             | [WR2E](https://developer.tuya.com/en/docs/iot/wr2e?id=K97scnsjhue4h)                           | RTL8710BN @ 125 MHz     | 2 MiB | 256 KiB  | 11 (8 I/O)  | ✔️    | ❌   | -
❌                             | [WR2L](https://developer.tuya.com/en/docs/iot/wifiwr2lmodule?id=K9605tnbj7gva)                 | RTL8710BX @ 62.5 MHz    | 2 MiB | 256 KiB  | 7 (5 I/O)   | ✔️    | ❌   | -
❌                             | [WR2LE](https://developer.tuya.com/en/docs/iot/wr2le?id=K9eio9y9e8i8c)                         | RTL8710BX @ 62.5 MHz    | 2 MiB | 256 KiB  | 7 (5 I/O)   | ✔️    | ❌   | -
[`wr3`](boards/wr3/README.md) | [WR3](https://developer.tuya.com/en/docs/iot/wr3-module-datasheet?id=K9g3ainzbj9z1)            | RTL8710BN @ 125 MHz     | 2 MiB | 256 KiB  | 16 (12 I/O) | ✔️    | ❌   | `realtek-ambz`
❌                             | [WR3E](https://developer.tuya.com/en/docs/iot/wr3e-module-datasheet?id=K9elwlqbfosbc)          | RTL8710BN @ 125 MHz     | 2 MiB | 256 KiB  | 16 (12 I/O) | ✔️    | ❌   | -
❌                             | [WR3L](https://developer.tuya.com/en/docs/iot/wifiwr3lmodule?id=K9605tt0kveqm)                 | RTL8710BX @ 125 MHz (?) | 2 MiB | 256 KiB  | 16 (12 I/O) | ✔️    | ❌   | -
❌                             | [WR3LE](https://developer.tuya.com/en/docs/iot/wr3le?id=K986l7a1ha8tm)                         | RTL8710BX @ 62.5 MHz    | 2 MiB | 256 KiB  | 16 (12 I/O) | ✔️    | ❌   | -
❌                             | [WR3N](https://developer.tuya.com/en/docs/iot/wr3n-datasheet?id=K98zdx31ztdge)                 | RTL8710BN @ 125 MHz     | 2 MiB | 256 KiB  | 16 (10 I/O) | ✔️    | ❌   | -
❌                             | [WR4](https://developer.tuya.com/en/docs/iot/wifiwr4module?id=K9605tvu78p3e)                   | RTL8710BN @ 125 MHz     | 1 MiB | 256 KiB  | 16 (12 I/O) | ✔️    | ❌   | -
❌                             | [WR5E](https://developer.tuya.com/en/docs/iot/wr5e?id=K986r9pxqxa8i)                           | RTL8710BX @ 62.5 MHz    | 2 MiB | 256 KiB  | 15 (12 I/O) | ✔️    | ❌   | -
❌                             | [WR6-H](https://developer.tuya.com/en/docs/iot/wr6h-module-datasheet?id=K9pork8eeowgl)         | RTL8710BN @ 125 MHz     | 2 MiB | 256 KiB  | 18 (12 I/O) | ✔️    | ❌   | -
❌                             | [WR6](https://developer.tuya.com/en/docs/iot/wr6-datasheet?id=K97rp45u6gff9)                   | RTL8710BN @ 125 MHz     | 2 MiB | 256 KiB  | 18 (12 I/O) | ✔️    | ❌   | -
❌                             | [WRG1](https://developer.tuya.com/en/docs/iot/wr6-datasheet?id=K97rp45u6gff9)                  | RTL8711AM @ 166 MHz     | 4 MiB | 2048 KiB | 25 (20 I/O) | ✔️    | ❌   | -
❌                             | [XR1](https://developer.tuya.com/en/docs/iot/xr1?id=K9lq3y9xo0zkx)                             | XR809 @ 160 MHz         | 2 MiB | 384 KiB  | 18 (11 I/O) | ✔️    | ❌   | -
❌                             | [XR1-IPEX](https://developer.tuya.com/en/docs/iot/xr1ipex-module-datasheet?id=K9razqu9gqele)   | XR809 @ 160 MHz         | 2 MiB | 384 KiB  | 18 (11 I/O) | ✔️    | ❌   | -
❌                             | [XR3](https://developer.tuya.com/en/docs/iot/xr3-datasheet?id=K98s9168qi49g)                   | XR809 @ 160 MHz         | 2 MiB | 384 KiB  | 22 (17 I/O) | ✔️    | ❌   | -
❌                             | [WE1S](https://developer.tuya.com/en/docs/iot/wifie1smodule?id=K9605thnvg3e7)                  | ESP8266EX @ 80/160 MHz  | 2 MiB | 36 KiB   | 18 (11 I/O) | ✔️    | ❌   | -
❌                             | [WE2L](https://developer.tuya.com/en/docs/iot/wifie2lmodule?id=K9605ud0gkjmh)                  | ESP8285 @ 80/160 MHz    | 1 MiB | 50 KiB   | 7 (5 I/O)   | ✔️    | ❌   | -
❌                             | [WE2S](https://developer.tuya.com/en/docs/iot/wifie2smodule?id=K9605u79tgxug)                  | ESP8285 @ 80/160 MHz    | 1 MiB | 50 KiB   | 11 (8 I/O)  | ✔️    | ❌   | -
❌                             | [WE3L](https://developer.tuya.com/en/docs/iot/wifie3lpinmodule?id=K9605uj1ar87n)               | ESP8266 @ 80/160 MHz    | 2 MiB | 50 KiB   | 16 (12 I/O) | ✔️    | ❌   | -
❌                             | [WE3S](https://developer.tuya.com/en/docs/iot/wifie3smodule?id=K9605ua1cx9tv)                  | ESP8266 @ 80/160 MHz    | 2 MiB | 50 KiB   | 16 (12 I/O) | ✔️    | ❌   | -
❌                             | [WE3SE](https://developer.tuya.com/en/docs/iot/we3se?id=K97qv6ab7oh8d)                         | ESP32 @ 240 MHz         | 4 MiB | 520 KiB  | 22 (17 I/O) | ✔️    | ✔️  | -
❌                             | [WE5P](https://developer.tuya.com/en/docs/iot/wifie5pmodule?id=K9605um3dtjbx)                  | ESP8266 @ 80/160 MHz    | 1 MiB | 50 KiB   | 15 (11 I/O) | ✔️    | ❌   | -

\* Only modules featuring at least Wi-Fi. WBR, JWBR, CR, (TY)JW and (TY)LC Series are not included here

** I/O count includes GPIOs, ADCs, PWM outputs and UART, but doesn't count CEN/RST and power pins.

## Project structure

```
arduino/
├─ <platform name>/             Arduino Core for specific platform
│  ├─ cores/                        Core files
│  ├─ libraries/                    Supported built-in libraries
├─ libretuya/
│  ├─ api/                      Library interfaces (.h) for LibreTuya Arduino cores
│  ├─ <library name>.cpp        Built-in platform-independent libraries
boards/
├─ <board name>/                Board-specific code
│  ├─ variant.cpp                   Arduino variant initialization
│  ├─ variant.h                     Arduino variant pin configs
├─ <board name>.json            PlatformIO board description
builder/
├─ frameworks/                  Framework builders for PlatformIO
│  ├─ <platform name>-sdk.py        Vanilla SDK build system
│  ├─ <platform name>-arduino.py    Arduino Core build system
├─ main.py                      Main PlatformIO builder
fixups/
├─ <platform name>/             Code fix-ups to replace SDK parts
platform/
├─ <platform name>/             Other platform-specific files
│  ├─ bin/                          Binary blobs (bootloaders, etc.)
│  ├─ ld/                           Linker scripts
tools/
├─ <tool name>/                 Tools used during the build
platform.json                   PlatformIO manifest
platform.py                     Custom PlatformIO script
```

## Platforms

A list of platforms currently available in this project.

Platform name  | Supported MCU(s)                                                       | Arduino Core | Source SDK (PIO framework)
---------------|------------------------------------------------------------------------|--------------|--------------------------------------------------------------------------
`realtek-ambz` | Realtek [AmebaZ](https://www.amebaiot.com/en/amebaz/) SoC (`RTL87xxB`) | ✔️           | `framework-realtek-amb1` ([amb1_sdk](https://github.com/ambiot/amb1_sdk))

### Realtek Ameba

The logic behind naming of Realtek chips and their series took me some time to figure out:
- RTL8xxxA - Ameba1/Ameba Series
- RTL8xxxB - AmebaZ Series
- RTL8xxxC - AmebaZ2/ZII Series
- RTL8xxxD - AmebaD Series

As such, there are numerous CPUs with the same numbers but different series, which makes them require different code and SDKs.
- [RTL8195AM](https://www.realtek.com/en/products/communications-network-ics/item/rtl8195am)
- RTL8710AF (found in amb1_arduino)
- [RTL8711AM](https://www.realtek.com/en/products/communications-network-ics/item/rtl8711am)
- [RTL8710BN](https://www.realtek.com/en/products/communications-network-ics/item/rtl8710bn)
- RTL8710BX (found in Tuya product pages)
- RTL8710B? (found in amb1_sdk)
- RTL8711B? (found in amb1_sdk)
- [RTL8710CM](https://www.realtek.com/en/products/communications-network-ics/item/rtl8710cm)
- RTL8722CSM (found in ambd_arduino)
- RTL8720DN (found in ambd_arduino)
- [RTL8721DM](https://www.realtek.com/en/products/communications-network-ics/item/rtl8721dm)
- RTL8722DM (found in ambd_arduino)
- and probably many more

Different Ameba series are not compatible with each other. Apparently, there isn't a public SDK for AmebaZ that can support C++ properly (yet).

## Arduino Core support status

Note: this list will probably change with each functionality update.

&nbsp;              | `realtek-ambz`
--------------------|---------------
Core functions      | ✔️
GPIO/PWM/IRQ        | ✔️/❓/✔️
Analog input        | ❓
UART I/O            | ✔️
Flash I/O           | ❓
**CORE LIBRARIES**  |
SoftwareSerial      | ❌
SPI                 | ❌
Wire                | ❌
**OTHER LIBRARIES** |
SPIFFS              | ❌
BLE                 | -
Wi-Fi (SSL)         | ✔️ (❌)
HTTP                | ❌
NTP                 | ❌
OTA                 | ❌
MDNS                | ❌
MQTT                | ❌
SD                  | ❌

Legend:
- ✔️ working
- ❗ broken
- ❓ untested
- ❌ not implemented (yet?)
- \- not applicable
