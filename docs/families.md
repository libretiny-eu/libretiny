# Families

A list of families currently available in this project.

**Note:** the term *family* was chosen over *platform*, in order to reduce possible confusion between LibreTuya supported "platforms" and PlatformIO's "platform", as an entire package. *Family* is also more compatible with the UF2 term.

The following list corresponds to UF2 OTA format family names, and is also [available as JSON](../families.json). The IDs are also present in [ChipType.h](../arduino/libretuya/core/ChipType.h).

Title                                                              | Name (parent)                | Code                 | Short name & ID         | Supported MCU(s)          | Arduino Core | Source SDK
-------------------------------------------------------------------|------------------------------|----------------------|-------------------------|---------------------------|--------------|----------------------------------------------------------------------------------
Realtek Ameba1                                                     | `-`                          | `-`                  | `RTL8710A` (0x9FFFD543) | -                         | ❌            | -
[Realtek AmebaZ](https://www.amebaiot.com/en/amebaz/)              | `realtek-ambz`               | `ambz`               | `RTL8710B` (0x22E0D6FC) | RTL8710BN                 | ✔️           | `framework-realtek-amb1` ([amb1_sdk](https://github.com/ambiot/amb1_sdk))
Realtek AmebaZ2                                                    | `-`                          | `-`                  | `RTL8720C` (0xE08F7564) | -                         | ❌            | -
Realtek AmebaD                                                     | `-`                          | `-`                  | `RTL8720D` (0x3379CFE2) | -                         | ❌            | -
[Beken 7231T](http://www.bekencorp.com/en/goods/detail/cid/7.html) | `beken-7231t` (`beken-72xx`) | `bk7231t` (`bk72xx`) | `BK7231T` (0x675A40B0)  | BK7231T, BK7231S, BK7231U | ❌            | `framework-beken-bdk` ([bdk_freertos](https://github.com/bekencorp/bdk_freertos))
Beken 7231N                                                        | `-`                          | `-`                  | `BK7231N` (0x7B3EF230)  | -                         | ❌            | -
Boufallo 602                                                       | `-`                          | `-`                  | `BL602` (0xDE1270B7)    | -                         | ❌            | -
Xradiotech 809                                                     | `-`                          | `-`                  | `XR809` (0x51E903A8)    | -                         | ❌            | -
