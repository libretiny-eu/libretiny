# Libraries

A page outlining 3-rd party libraries compatible with LibreTuya.

To use some (most? (all?)) of these, a flag in `platformio.ini` is required to disable compatibility checks (because most libs are meant for ESP32/Arduino official framework):
```ini
[env:my_board]
lib_compat_mode = off
```

## MQTT
Tested with `realtek-ambz`.
```ini
lib_deps = 256dpi/MQTT@^2.5.0
```
