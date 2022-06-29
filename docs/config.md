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

- `LT_LOGGER` - enable/disable LibreTuya logger globally. Enabled by default.
- `LT_LOGLEVEL` - global LT loglevel:
  - `LT_LEVEL_TRACE` (same as LT_LEVEL_VERBOSE)
  - `LT_LEVEL_DEBUG`
  - `LT_LEVEL_INFO` - default
  - `LT_LEVEL_WARN`
  - `LT_LEVEL_ERROR`
  - `LT_LEVEL_FATAL`
- `LT_LOGGER_TIMESTAMP` - print program runtime in printk-like format
- `LT_LOGGER_CALLER` - print calling method name
- `LT_LOGGER_TASK` - print calling FreeRTOS task (if available)
- `LT_LOGGER_COLOR` - output ANSI terminal colors
- `LT_PRINTF_BROKEN` - whether printf outputs "0." for floats with value 0
- `LT_LOG_HEAP` - print free heap size using `LT_HEAP_I()`
- `LT_LOG_ERRNO` - print and clear errno value (if set) using `LT_ERRNO()`

### Debug logging

The following options enable library-specific debugging messages. They are only effective if `LT_LOGLEVEL` is set below INFO. All of them are disabled by default.

Families should generally call i.e. WiFiClient debugging for client-related code, even if the `WiFiClient.cpp` file is physically absent.

- `LT_DEBUG_WIFI` - `WiFi.cpp`
- `LT_DEBUG_WIFI_CLIENT` - `WiFiClient.cpp`
- `LT_DEBUG_WIFI_SERVER` - `WiFiServer.cpp`
- `LT_DEBUG_WIFI_STA` - `WiFiSTA.cpp`
- `LT_DEBUG_WIFI_AP` - `WiFiAP.cpp`
- `LT_DEBUG_SSL` - `WiFiClientSecure`

### Family options

These options are selectively set by all families, as part of the build process. They are used for enabling LT core API parts, if the family has support for it.

The `LT_ARD_*` options are only used with Arduino frameworks.

The meaning of most flags is as follows:

- `LT_HAS_LWIP` - LwIP in SDK (any version)
- `LT_HAS_LWIP2` - LwIP v2.0.0 or newer
- `LT_HAS_FREERTOS` - FreeRTOS supported and used
- `LT_HAS_MBEDTLS` - mbedTLS in SDK
- `LT_ARD_HAS_MD5` - MD5 library implemented, `MD5Impl.h` available
- `LT_ARD_HAS_WIFI` - WiFi library implemented, `WiFiData.h` available
- `LT_HEAP_FUNC` - function name used to get available heap size (for `LT_HEAP_I()`)
