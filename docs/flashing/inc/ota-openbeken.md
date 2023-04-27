## Migrating from OpenBeken (OTA)

[OpenBeken](https://github.com/openshwprojects/OpenBK7231T_App) is a custom, Tasmota-like firmware for non-ESP chips. Currently, this part of the guide applies to BK7231 only, as that's the only chip supported both by LT and OBK.

OBK is compatible with standard Beken OTA packages, but the web panel does a filename check to prevent chip type mismatch. Grab the `image_bk7231t_app.ota.rbl` file from build directory (note: without "UG" in the name!), rename it to something like `OpenBK7231T_esphome.rbl` (change T to N depending on the chip type), and drop it on the OTA panel.
