# LibreTuya API Configuration

Note: see [LibreTuyaConfig.h](../../arduino/libretuya/api/LibreTuyaConfig.h) for most options and their defaults.

All options are configurable via C++ defines in PlatformIO project file. For example:
```ini
[env:my_board]
build_flags =
  -D LT_LOGLEVEL=LT_LEVEL_DEBUG
```

## Logging

- LT_LOGGER - enable/disable LibreTuya logger globally. Enabled by default.
- LT_LOGLEVEL - global LT loglevel:
  - LT_LEVEL_TRACE (same as LT_LEVEL_VERBOSE)
  - LT_LEVEL_DEBUG
  - LT_LEVEL_INFO - default
  - LT_LEVEL_WARN
  - LT_LEVEL_ERROR
  - LT_LEVEL_FATAL
- LT_LOGGER_TIMESTAMP - print program runtime in printk-like format
- LT_LOGGER_FILE - print calling source filename
- LT_LOGGER_TASK - print calling FreeRTOS task (if available)
- LT_LOGGER_COLOR - output ANSI terminal colors
- LT_PRINTF_BROKEN - whether printf outputs "0." for floats with value 0

## Debug logging

The following options enable library-specific debugging messages. They are only effective if `LT_LOGLEVEL` is set below INFO. All of them are disabled by default.

Platforms should generally call i.e. WiFiClient debugging for client-related code, even if the `WiFiClient.cpp` file is physically absent.

- LT_DEBUG_WIFI - `WiFi.cpp`
- LT_DEBUG_WIFI_CLIENT - `WiFiClient.cpp`
- LT_DEBUG_WIFI_SERVER - `WiFiServer.cpp`
- LT_DEBUG_WIFI_STA - `WiFiSTA.cpp`
- LT_DEBUG_WIFI_AP - `WiFiAP.cpp`

## Platform options

- LT_HAS_LWIP - whether platform SDK has LwIP. This causes `LwIPRxBuffer.cpp` to be compiled for platform libraries to use.
