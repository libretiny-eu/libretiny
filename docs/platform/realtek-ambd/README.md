# Realtek AmebaD

## Introduction

Realtek AmebaD is a family of dual-band Wi-Fi + BLE 5.0 microcontrollers — RTL8720DN, RTL8720DM, RTL8721DM and newer DX/CS revisions. The most common module is the Ai-Thinker BW16 (RTL8720DN).

The chip contains two cores; LibreTiny applications run on the high-power core, while the low-power core image ships prebuilt in the framework package.

Features:

- KM4: ARM Cortex-M33 (ARMv8-M) CPU @ 200 MHz (application core)
- KM0: ARM Cortex-M23 CPU @ 20 MHz (low-power core, prebuilt image)
- 512 KiB SRAM (KM4)
- SPI flash interface with XiP
- 802.11a/b/g/n dual-band (2.4 GHz + 5 GHz) Wi-Fi
- Bluetooth 5.0 LE (not yet supported in LibreTiny)

Resources:

- [Realtek product page](https://www.realtek.com/en/products/communications-network-ics/item/rtl8720dn)
- [AmebaD SDK (ambd_sdk)](https://github.com/ambiot/ambd_sdk)
- [BW16 Arduino getting started (official pin map)](https://www.amebaiot.com.cn/en/amebad-bw16-arduino-getting-started/)
- [BW16 module docs (Ai-Thinker)](https://docs.ai-thinker.com/en/wifi/bw16)

## Finding your board

{%
	include-markdown "../../inc/find-board.md"
%}

---

## Flashing

{%
	include-markdown "../../inc/flashing-note.md"
%}

The port used for flashing and viewing logs is LOG_UART (PA7/PA8), at 115200 baud for logs. On the BW16 KIT, the USB-C socket connects to the AT-command UART (PB1/PB2) and can **not** flash the chip — use the LOG_UART pins with an external USB-TTL adapter.

### Wiring

PC  | RTL8720D
----|--------------------
RX  | LOG_TX (PA7)
TX  | LOG_RX (PA8)
GND | GND

{%
	include-markdown "../../inc/uart-power.md"
%}

### Download mode

To enter UART download mode:

- connect LOG_TX (PA7) to GND (on boards with a `BURN`/`Burn` button, hold it instead)
- press and release the reset (CHIP_EN) button
- release LOG_TX / the button

The ROM then accepts the flashloader handshake. Flashing writes the full image at offset 0x0.

### Tools

`ltchiptool` support is in progress (libretiny-eu/ltchiptool#98). Until it lands, these tools flash AmebaD correctly:

- [BK7231Flasher / ltchiptool GUI fork with Ameba support](https://github.com/openshwprojects/BK7231GUIFlashTool) — select RTL8720D, 460800 baud
- [amebad_image_tool](https://github.com/Seeed-Studio/ambd_flash_tool) — command line, 115200/1.5M baud
