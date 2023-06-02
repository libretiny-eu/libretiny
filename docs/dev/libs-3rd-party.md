# Libraries

A page outlining 3-rd some party libraries compatible with LibreTiny.

!!! note
	To use some (most? (all?)) of these, a flag in `platformio.ini` is required to disable compatibility checks (because most libs are meant for ESP32/Arduino official framework):
	```ini
	[env:my_board]
	lib_compat_mode = off
	```

## [256dpi/MQTT](https://registry.platformio.org/libraries/256dpi/MQTT)
Tested with `realtek-ambz`.
```ini
lib_deps = 256dpi/MQTT@^2.5.0
```

## [bbx10/DNSServer](https://registry.platformio.org/libraries/bbx10/DNSServer)
Tested with `beken-72xx`.
```ini
lib_deps = bbx10/DNSServer@^1.1.0
```
This is the same library as in ESP32 Arduino Core.

## [esphome/AsyncTCP-esphome](https://registry.platformio.org/libraries/esphome/AsyncTCP-esphome)
Tested with `beken-72xx` and `realtek-ambz`.
```ini
lib_deps = esphome/AsyncTCP-esphome@^2.0.0
```
This is ESPHome's fork of the original library.

## [esphome/ESPAsyncWebServer-esphome](https://registry.platformio.org/libraries/esphome/ESPAsyncWebServer-esphome)
Tested with `beken-72xx` and `realtek-ambz`.
```ini
lib_deps = esphome/ESPAsyncWebServer-esphome@^3.0.0
```
This is ESPHome's fork of the original library.
