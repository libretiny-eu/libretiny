# TODO list

## General

### New families

- BL602
- RTL8710A
- RTL8720C
- RTL8720D
- W600 and/or W800
- LN8825
- host-native family

### Tools

- move all UF2 assembling/uploading/processing tools (as well as `uf2ota` C library) to a separate repository, possibly rewriting parts of it again. Make these tools CLI-usable
- write OpenOCD flashers, using uf2ota library + FAL for partitions (same repo as above)

### Serial

- configuration of RX/TX pins
- SoftwareSerial library - receiving + Beken family

### Other

- watchdog API
- `Preferences` library
- test/fix IPv6 on different families
- what is `PowerManagement` at all? probably useless -> remove

## BK7231

- WiFi events
- implement OTA
- fix WiFi on BK7231N, test other functionality
- add generic board definition
- fix SSL (mbedTLS)
- I2C (Wire)
- SPI
- BLE

## RTL8710B

- add generic board definition
- rewrite most of Wiring (it was copied from `ambd_arduino`, and is ugly)
