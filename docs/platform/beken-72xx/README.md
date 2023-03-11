# Beken 72xx

## Resources

Name                                                                                                                                                                    | Notes
------------------------------------------------------------------------------------------------------------------------------------------------------------------------|---------------------------------------------------
[BK7231U Datasheet v0.71](https://cdn.discordapp.com/attachments/983843871320580096/1050461302537064508/BK7231Uv0.71.zh-CN.en.pdf)                                      | Machine-translated to English
[BL2028N Datasheet v1.0](https://cdn.discordapp.com/attachments/983843871320580096/1050461346111697028/BL2028N_Datasheet_v1.0.pdf)                                      | BL2028N is a "clone" of BK7231N
[BK72XX SDK User Manual 3.0.3](https://cdn.discordapp.com/attachments/983843871320580096/1003661237730672730/BK72XX_SDK_User_Manual-3.0.3.pdf)                          | Machine-translated to English
[BEKEN WiFi SDK API Reference 3.0.27](https://cdn.discordapp.com/attachments/983843871320580096/1003661237349003355/BEKEN_WiFi_SDK_API_Reference-3.0.27_compressed.pdf) | Machine-translated to English
[Beken SDK documentation (online)](http://docs.bekencorp.com/backup/v3.0/)                                                                                              | Not much here
[`encrypt v0.3` source code](https://github.com/ghsecuritylab/tysdk_for_bk7231/tree/master/toolchain/encrypt_crc)                                                       |
[`ota_tools` source code](https://github.com/tiancj/rtt_ota_tools)                                                                                                      | Encryption routines and FPGA code used on the chip
[BK7231 OpenOCD debugging](https://www.elektroda.com/rtvforum/viewtopic.php?p=20028605#20028605)                                                                        | From Elektroda.pl user `@xabean`

## General info

There are many chip variations in this SoC family:

- BK7231 - marked BK7231QN40, so we're calling it "BK7231Q" to reduce confusion
- BK7231T
- BK7231N
- BK7231S
- BK7231U

The "officially existing" ones are BK7231Q, BK7231N and BK7231U. These are supported by Beken SDKs, such as `bdk_freertos`, although `bk7231s_alios_sdk` also existed at some point.

- BK7231N is substantially different than the other chips, so running T code on N (and vice versa) is not directly possible.
- BK7231Q does not have eFuse.
- there are some references to U meaning USB support
- T seems to be exclusive to Tuya boards (that would explain the name); in the T SDK from Tuya, `CFG_SOC_NAME` is set to `SOC_BK7231U`
- T's bootloader greets with `BK7231S_1.0.5` on UART

Regarding `bdk_freertos`:

- `make` allows selecting for which MCU the code should be compiled
- `make bk7231` doesn't compile at all
- `make bk7231u` doesn't run on T with Tuya's bootloader (1.0.5), though it works just fine after replacing the bootloader with one of these included with `bdk_freertos` (1.0.8) (yes, even `bk7231n` bootloader)
- after making a few changes to `driver/entry` and `driver/intc` (so it looks more like the code from T SDK) `bdk_freertos` runs just fine
