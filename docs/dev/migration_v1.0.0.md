# Migration to v1.0.0

1.0.0 is the first major release of LT. Compared to previous versions, few things have changed which can impact developers using LT in PlatformIO (but shouldn't affect ESPHome users at all).

## GPIO numbering

Pin numbers passed to `pinMode()`/`digitalWrite()`/etc. are now the raw GPIO numbers of the chip. Previously, one had to use `D#` numbering in these functions, so you have to migrate your code to use GPIO numbers instead.

To make the migration easier, you can simply change:

```cpp
digitalWrite(1, LOW);
```

to:

```cpp
digitalWrite(PIN_D1, LOW);
// or
digitalWrite(D1, LOW);
```

For more information, refer to [GPIO instructions](../getting-started/gpio.md).

## Environment stability

All public headers exported by LT are now stable between all chip families - this means that they're not including any code from the vendor SDK.

This change was made to prevent the SDK from introducing its own functions and macros, which often replace stdlib functions, or cause other compilation issues.

If your code is using vendor SDK functions, you'll have to import the appropriate headers yourself.

## OTA (.uf2 packages)

The format of OTA packages has changed slightly, to reflect the OTA scheme naming change (described below). There's also a distinction between the `flasher` (PC flasher) and `device` (on-device OTA code) in the package.

New OTA packages are backwards-compatible (i.e. can be installed on devices running LT v0.x.x), but **v1.0.0 will not accept older packages** - it's **not** possible to rollback a device from v1.0.0 to v0.x.x with an old .uf2 file.

## OTA scheme naming change

Previously, each chip family had a "has dual OTA" property, which was very confusing (even to me, the author of the code). A new scheme has been introduced:

- single OTA - devices with a separate "download" partition, which is used by the bootloader to flash the main application
- dual OTA - devices with two separate application partitions, which can be updated directly by the application
