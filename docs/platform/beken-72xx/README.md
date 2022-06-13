# Beken 72xx - notes

There are many chip variations in this SoC family:

- BK7231
- BK7231T
- BK7231N
- BK7231S
- BK7231U

The "officially existing" ones are BK7231, BK7231N and BK7231U. These are supported by Beken SDKs, such as `bdk_freertos`, although `bk7231s_alios_sdk` also existed at some point.

- BK7231N is substantially different than the other chips, so running T code on N (and vice versa) is not directly possible.
- BK7231 does not have eFuse.
- there are some references to U meaning USB support
- T seems to be exclusive to Tuya boards (that would explain the name); in the T SDK from Tuya, `CFG_SOC_NAME` is set to `SOC_BK7231U`
- T's bootloader greets with `BK7231S_1.0.5` on UART

Regarding `bdk_freertos`:

- `make` allows selecting for which MCU the code should be compiled
- `make bk7231` doesn't compile at all
- `make bk7231u` doesn't run on T with Tuya's bootloader (1.0.5), though it works just fine after replacing the bootloader with one of these included with `bdk_freertos` (1.0.8) (yes, even `bk7231n` bootloader)
- after making a few changes to `driver/entry` and `driver/intc` (so it looks more like the code from T SDK) `bdk_freertos` runs just fine
