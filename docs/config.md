# Configuration

## Project options

```ini
[env:my_board]
# custom firmware name, present in UF2 output files
# - default: project directory name
custom_fw_name = my_firmware
# custom firmware version
# - default: current date in yy.mm.dd format
custom_fw_version = 1.2.0
```

## LibreTuya API

!!! note
    See [LibreTuyaConfig.h](../ltapi/_libre_tuya_config_8h_source.md) for most options and their defaults.

All options are configurable via C++ defines in PlatformIO project file. For example:
```ini
[env:my_board]
build_flags =
  -D LT_LOGLEVEL=LT_LEVEL_DEBUG
```

### Logging

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

### Debug logging

The following options enable library-specific debugging messages. They are only effective if `LT_LOGLEVEL` is set below INFO. All of them are disabled by default.

Families should generally call i.e. WiFiClient debugging for client-related code, even if the `WiFiClient.cpp` file is physically absent.

- LT_DEBUG_WIFI - `WiFi.cpp`
- LT_DEBUG_WIFI_CLIENT - `WiFiClient.cpp`
- LT_DEBUG_WIFI_SERVER - `WiFiServer.cpp`
- LT_DEBUG_WIFI_STA - `WiFiSTA.cpp`
- LT_DEBUG_WIFI_AP - `WiFiAP.cpp`

### Family options

- LT_HAS_LWIP - whether family SDK has LwIP. This causes `LwIPRxBuffer.cpp` to be compiled for family libraries to use.
- LT_HAS_LWIP2 - whether family has LwIP v2.0.0 or newer. This causes `LwIPmDNS.cpp` to be compiled.
- LT_HAS_FREERTOS - whether family supports FreeRTOS
- LT_HAS_MBEDTLS - whether family has mbedTLS
