# silabs-efm32gg11

LibreTiny chip family for Silicon Labs **EFM32GG11** (Giant Gecko 11), specifically the EFM32GG11B820F2048GM64 inside the **WGM160P** Wi-Fi module SiP. This family ships the chip family proper plus WF200 Wi-Fi support (STA + softAP); a later phase ports `openevse_esp32_firmware` to it for an Enel X JuiceBox 40.

## Supported boards

| Variant | Notes |
|---|---|
| `wgm160p-juicebox-40` | Enel X JuiceBox 40 (WGM160P host). The bench-proven board: GPIO, Serial, and Wi-Fi (STA + softAP) are all hardware-confirmed on this unit. |

## Arduino surface

| Works | Does not work (link-time fail by design) |
|---|---|
| `pinMode`, `digitalRead`, `digitalWrite` | `analogRead`, `analogWrite` |
| `delay`, `delayMicroseconds`, `millis`, `micros` | `Wire` (I2C), `SPI` |
| `Serial` (USART0 LOC1 @ 115200 8N1) | OTA / `Update` (stubbed) |
| `attachInterrupt`, `detachInterrupt` (GPIO EXTI lines 0–15) | |
| `WiFi.h` — STA (`WiFi.begin`, scan, DHCP client, `HTTPClient`) and softAP | |
| FreeRTOS C API (`xTaskCreate`, `vTaskDelay`, `xQueueSend*`, …) | |

### Wi-Fi (WF200)

Wi-Fi runs on the on-module **WF200** transceiver via the vendored `wfx_fmac_driver`. Both modes are bench-confirmed on the JuiceBox 40:

- **Station**: `WiFi.begin()`, scan, DHCP client, and `HTTPClient` GET. The host↔WF200 link runs with **Secure Link enforced** (encrypted/authenticated control path).
- **softAP**: concurrent AP+STA on a shared channel, with an on-device DHCP server; associated AP clients can reach an on-device lwIP `httpd`.

The WF200 host bus is selected by the module's **HIF_SEL** strap. The supported configuration is **SDIO** (the JuiceBox strap; PE8–PE13), which is what this backend drives.

## Phase 1 limitations to know

### GPIO EXTI lines are indexed by pin number, not (port, pin)

EFM32 hardware: there are exactly 16 EXTI lines, indexed 0–15 by the GPIO pin number. Two pins with the same number on different ports (e.g., `PA5` and `PB5`) share EXTI line 5 and cannot both have interrupts simultaneously — the second `attachInterrupt` call clobbers the first. Pick a pin number that no other peripheral in your sketch needs.

### The watchdog is wired but not armed by default

`lt_wdt_*` functions work, but the WDOG peripheral is left disabled at boot. Call `lt_wdt_enable(timeout_ms)` from `setup()` if you want WDOG protection. Reason: an armed watchdog bites during early bring-up the moment you sit at a breakpoint.

### WGM160P internal WF200 pin blacklist

The following EFM32 GPIOs are physically committed inside the WGM160P SiP to the internal EFM32↔WF200 link and **must not** be used by user sketches. On the JuiceBox 40 (SDIO strap) the data path is the four SDIO lines plus clock and command; the rest are control/power signals:

- `PE8`–`PE11` (WF200 SDIO DAT3/DAT2/DAT1/DAT0; `PE9` doubles as the HIF_SEL strap)
- `PE12` (WF200 SDIO CMD)
- `PE13` (WF200 SDIO CLK) — routed at LOC0 (`SDIO->ROUTELOC0/1` use `*LOC_LOC0`)
- `PA8` (WF200 host IRQ)
- `PA12` (WF200 LP_CLK)
- `PE4` (WF200 wake)
- `PF12` (WF200 reset, active-low)

The `boards/wgm160p-juicebox-40.json` board file documents these in its `pcb.blacklist` block.

## Flashing

Default flash tool is **Simplicity Commander** (`commander`). Connect an SWD debug probe (e.g. a J-Link) to the WGM160P's debug pins and run:

```sh
pio run -t upload -e wgm160p-juicebox-40
```

PlatformIO routes through `tools/silabs-efm32gg11/flash.py`, which invokes:

```sh
commander flash --device EFM32GG11B820F2048GM64 <bin>
commander device reset
```

### If you brick the device

If user code wedges the chip badly enough that Commander can't enumerate normally (e.g., trashing CMU before debug pins are configured), use halt-at-reset mass-erase:

```sh
commander device masserase --serialno <probe_serial>
commander adapter dbgmode --mode MCU --serialno <probe_serial>
```

Then re-flash. The probe holds the EFM32 in reset while erasing flash.

### Alternative: pyOCD

If Simplicity Commander isn't available, `pyOCD` works as a fallback:

```sh
pip install pyocd
pyocd flash -t efm32gg11b820 <bin>
```

## Open follow-ups (in suggested order)

1. ADC (`analogRead`) — wraps emlib `ADC_*` against ADC0.
2. PWM (`analogWrite`) — TIMER CC channels.
3. I2C / `Wire` — emlib `I2C_*` against I2C0 or I2C1.
4. SPI — USART-as-SPI master mode.
5. OTA / `Update` — currently stubbed (link-time fail).

## References

- [Silabs WGM160P datasheet](https://www.silabs.com/documents/public/data-sheets/wgm160p-datasheet.pdf)
- [Silabs EFM32GG11 family datasheet](https://www.silabs.com/documents/public/data-sheets/efm32gg11-datasheet.pdf)
- [Silabs UG384 — WGM160P Hardware Design User's Guide](https://www.silabs.com/documents/public/user-guides/ug384-wgm160p-hardware-design.pdf)
- [SiliconLabs/gecko_sdk](https://github.com/SiliconLabs/gecko_sdk) — upstream vendor SDK (mirrored in [`RAR/framework-silabs-gecko-sdk`](https://github.com/RAR/framework-silabs-gecko-sdk))
