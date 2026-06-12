# silabs-efm32gg11

LibreTiny chip family for Silicon Labs **EFM32GG11** (Giant Gecko 11), specifically the EFM32GG11B820F2048GM64 inside the **WGM160P** Wi-Fi module SiP. Phase 1 ships the chip family proper; Phase 2 will add WF200 Wi-Fi support; Phase 3 ports `openevse_esp32_firmware` to it for an Enel X JuiceBox 40.

## Supported boards

| Variant | Notes |
|---|---|
| `wgm160p-slwstk6121a` | Silicon Labs WGM160P Starter Kit (SLWRB4321A radio board + WSTK mainboard). Uses USART0 LOC1 (PE7/PE6) for the WSTK VCOM. |

## Phase 1 Arduino surface

| Works | Does not work (link-time fail by design) |
|---|---|
| `pinMode`, `digitalRead`, `digitalWrite` | `analogRead`, `analogWrite` |
| `delay`, `delayMicroseconds`, `millis`, `micros` | `Wire` (I2C), `SPI` |
| `Serial` (USART0 LOC1 @ 115200 8N1) | `WiFi.h` (Phase 2) |
| `attachInterrupt`, `detachInterrupt` (GPIO EXTI lines 0–15) | OTA / `Update` (Phase 2/3) |
| FreeRTOS C API (`xTaskCreate`, `vTaskDelay`, `xQueueSend*`, …) | |

## Phase 1 limitations to know

### GPIO EXTI lines are indexed by pin number, not (port, pin)

EFM32 hardware: there are exactly 16 EXTI lines, indexed 0–15 by the GPIO pin number. Two pins with the same number on different ports (e.g., `PA5` and `PB5`) share EXTI line 5 and cannot both have interrupts simultaneously — the second `attachInterrupt` call clobbers the first. Pick a pin number that no other peripheral in your sketch needs.

### The watchdog is wired but not armed by default

`lt_wdt_*` functions work, but the WDOG peripheral is left disabled at boot. Call `lt_wdt_enable(timeout_ms)` from `setup()` if you want WDOG protection. Reason: an armed watchdog bites during early bring-up the moment you sit at a breakpoint.

### WGM160P internal SPI pin blacklist

The following EFM32 GPIOs are physically committed inside the WGM160P SiP to the internal EFM32↔WF200 SPI link and **must not** be used by user sketches:

- `PA0` (WF200 SPI MOSI, USART3 LOC0)
- `PA1` (WF200 SPI MISO)
- `PA2` (WF200 SPI SCK)
- `PA3` (WF200 SPI CS)
- `PA8` (WF200 host IRQ)
- `PA12` (WF200 LP_CLK)
- `PE4` (WF200 wake)
- `PF12` (WF200 reset, active-low)

The `boards/wgm160p-slwstk6121a.json` board file documents these in its `pcb.blacklist` block.

## Flashing

Default flash tool is **Simplicity Commander** (`commander`). For the SLWSTK6121A, plug a USB cable into the WSTK "J-Link" port and run:

```sh
pio run -t upload -e wgm160p-slwstk6121a
```

PlatformIO routes through `tools/silabs-efm32gg11/flash.py`, which invokes:

```sh
commander flash --device EFM32GG11B820F2048GM64 <bin>
commander device reset
```

### If you brick the device

If user code wedges the chip badly enough that Commander can't enumerate normally (e.g., trashing CMU before debug pins are configured), use halt-at-reset mass-erase:

```sh
commander device masserase --serialno <wstk_serial>
commander adapter dbgmode --mode MCU --serialno <wstk_serial>
```

Then re-flash. The WSTK J-Link can hold the EFM32 in reset while erasing flash.

### Alternative: pyOCD

If Simplicity Commander isn't available, `pyOCD` works as a fallback:

```sh
pip install pyocd
pyocd flash -t efm32gg11b820 <bin>
```

## Open follow-ups (post Phase 1, in suggested order)

1. ADC (`analogRead`) — wraps emlib `ADC_*` against ADC0.
2. PWM (`analogWrite`) — TIMER CC channels.
3. I2C / `Wire` — emlib `I2C_*` against I2C0 or I2C1.
4. SPI — USART-as-SPI master mode.
5. Phase 2: WiFi via WF200 (see project `SCOPING.md`).

## References

- [Silabs WGM160P datasheet](https://www.silabs.com/documents/public/data-sheets/wgm160p-datasheet.pdf)
- [Silabs EFM32GG11 family datasheet](https://www.silabs.com/documents/public/data-sheets/efm32gg11-datasheet.pdf)
- [Silabs UG384 — WGM160P Hardware Design User's Guide](https://www.silabs.com/documents/public/user-guides/ug384-wgm160p-hardware-design.pdf)
- [SiliconLabs/gecko_sdk](https://github.com/SiliconLabs/gecko_sdk) — upstream vendor SDK (mirrored in [`RAR/framework-silabs-gecko-sdk`](https://github.com/RAR/framework-silabs-gecko-sdk))
