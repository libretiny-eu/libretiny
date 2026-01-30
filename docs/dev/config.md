# Configuration

## Project options

```ini title="platformio.ini"
[env:my_board]
# custom firmware name, present in UF2 output files
# - default: project directory name
custom_fw_name = my_firmware
# custom firmware version
# - default: current date in yy.mm.dd format
custom_fw_version = 1.2.0

# custom build options (#defines, NOT compiler flags)
custom_options.lwip =
	# make sure to enable LT_DEBUG_LWIP as well
	NETIF_DEBUG = 0x80
    IP_DEBUG = 0x80
    TCP_DEBUG = 0x80
custom_options.freertos =
	configUSE_TICK_HOOK = 1

# partition layout modification (not recommended, unless you know what you're doing)
custom_flash.app = 0x12000

# custom board JSON (overrides)
# - path relative to the project directory; only values specified
#     in the JSON will override the defaults
#     (it's like using board_build.xxx but for more keys)
custom_board = myboard.json

# custom library versions (not recommended)
custom_versions.lwip = 2.1.3
custom_versions.beken_bdk = 2021.06.07
```

## LibreTiny options

!!! note
	See [lt_config.h](../../ltapi/lt__config_8h.md) for most options and their defaults.

All options are configurable via C++ defines in PlatformIO project file. For example:
```ini title="platformio.ini"
[env:my_board]
build_flags =
  -D LT_LOGLEVEL=LT_LEVEL_DEBUG
```

!!! tip
	Values in parentheses represent the defaults for the config options.

### Logger

- `LT_LOGGER` (1) - enable/disable LibreTiny logger globally; disabling this sets the loglevel to `LT_LEVEL_NONE` - the logger can't be enabled even by using `lt_log_set_port()`
- `LT_LOGLEVEL` - global LT loglevel:

	- `LT_LEVEL_VERBOSE`
	- `LT_LEVEL_TRACE` - same as `LT_LEVEL_VERBOSE`
	- `LT_LEVEL_DEBUG`
	- `LT_LEVEL_INFO` - default
	- `LT_LEVEL_WARN`
	- `LT_LEVEL_ERROR`
	- `LT_LEVEL_FATAL`
	- `LT_LEVEL_NONE` - disables everything

- `LT_LOGGER_TIMESTAMP` (1) - print program runtime in printk-like format
- `LT_LOGGER_CALLER` (1) - print calling method name
- `LT_LOGGER_TASK` (1) - print calling FreeRTOS task (if available)
- `LT_LOGGER_COLOR` (0) - output ANSI terminal colors
- `LT_PRINTF_BROKEN` (0) - whether printf outputs "0." for floats with value 0
- `LT_LOG_HEAP` (0) - print free heap size using `LT_HEAP_I()`, and periodically every 1000 ms
- `LT_LOG_ERRNO` (0) - print and clear errno value (if set) using `LT_ERRNO()`

#### Per-module logging & debugging

The following options enable library-specific logging output. They are effective **for all loglevels** - i.e. disabling `LT_DEBUG_WIFI` will disable WiFi debug messages, warnings, as well as errors.

To see debug messages from i.e. OTA, loglevel must also be changed.

- `LT_DEBUG_ALL` (0) - enable all following options by default (except for FDB and LWIP)
- `LT_DEBUG_WIFI` (1) - WiFi (generic, STA, AP, scan, events, etc.)
- `LT_DEBUG_CLIENT` (0) - TCP client
- `LT_DEBUG_SERVER` (0) - TCP server
- `LT_DEBUG_SSL` (0) - SSL clients
- `LT_DEBUG_OTA` (1) - OTA updates (`Update` library)
- `LT_DEBUG_FDB` (0) - FlashDB debugging (macros within the library)
- `LT_DEBUG_MDNS` (0) - mDNS client library
- `LT_DEBUG_LWIP` (0) - enables `LWIP_DEBUG`, provides `LWIP_PLATFORM_DIAG`; per-module options (i.e. `TCP_DEBUG`) are off by default and need to be enabled separately - see example in `Project options` above
- `LT_DEBUG_LWIP_ASSERT` (0) - enables assertions within lwIP (doesn't need `LT_DEBUG_LWIP`)

!!! tip
	Enabling `LT_DEBUG_ALL` doesn't mean that *every* debugging message will be printed. If loglevel is i.e. `WARN`, debug messages won't be visible anyway.

	This can be used, for example, to enable only "important" messages:
	```ini title="platformio.ini"
	[env:my_board]
	build_flags =
	  -D LT_LOGLEVEL=LT_LEVEL_WARN
	  -D LT_DEBUG_ALL=1 # will print only warnings and errors from all modules
	```

### Serial output

Options for controlling default UART log output.

- `LT_UART_DEFAULT_PORT` (unset) - default output port for all messages (SDK, LT logger, Serial class); can be 0, 1 or 2
- `LT_UART_DEFAULT_LOGGER` (unset) - override default output port for LT logger only
- `LT_UART_DEFAULT_SERIAL` (unset) - override default output port for `Serial` class (without a number)
- `LT_UART_SILENT_ENABLED` (1) - enable auto-silencing of SDK "loggers"; this makes the serial output much more readable, but can hide some error messages
- `LT_UART_SILENT_ALL` (0) - disable all SDK output (LT output and logger still work); since v1.0.0 this has no effect if `LT_UART_SILENT_ENABLED` is 0

!!! info
	Values 0, 1 and 2 correspond to physical UART port numbers (refer to board pinout for the available ports).

	Serial class instances (`Serial0`, `Serial1`, `Serial2`) use the respective port numbers for printing.

	If `LT_UART_DEFAULT_LOGGER` is not set, it is chosen by the family code - whichever port is most appropriate (i.e. LOG_UART (2) on Realtek, RX2/TX2 on Beken).

### Misc options

- `LT_USE_TIME` (0) - enables implementation of `gettimeofday()` and `settimeofday()`; checks for `millis()` overflows periodically
- `LT_MICROS_HIGH_RES` (1) - count runtime microseconds using a high-resolution timer (if possible); disable if your application doesn't need `micros()`
- `LT_AUTO_DOWNLOAD_REBOOT` (1) - automatically reboot into "download mode" after detecting a flashing protocol command; [read more](../flashing/tools/adr.md)

### Family configuration

!!! bug "Warning"
	These options are not meant for end-users. They're provided here as a reference for developers. **Do not set these options manually**.

These options are selectively set by all families, as part of the build process. They are used for enabling LT core API parts, if the family has support for it. Files named `lt_defs.h`, containing these options, are read by the PlatformIO builders (note: they're never included by C code).

Checking for option value should be done with `#if` (not with `#ifdef`!) - if it's not defined, it will evaluate to 0. Otherwise, it will use the defined value, either 0 or 1.

- family-/chip-specific hardware peripherals
	- `LT_HW_WIFI` - WiFi supported on the chip
	- `LT_HW_BT` - Bluetooth Classic supported on the chip
	- `LT_HW_BLE` - Bluetooth Low Energy supported on the chip
	- `LT_HW_WATCHDOG` - watchdog available
	- `LT_HW_DEEP_SLEEP` - deep sleep possible
- board-specific peripherals (note: defined in `lt_pins.h`, depending on available pin numbers)
	- `LT_HW_UART#` - UART number # available on the board
	- `LT_HW_I2C#` - I²C number # available on the board
	- `LT_HW_SPI#` - SPI number # available on the board
- family software options (SDK features, LT implementation status)
	- `LT_HAS_FREERTOS` - FreeRTOS supported and used
	- `LT_HAS_LWIP` - LwIP in SDK (any version)
	- `LT_HAS_LWIP2` - LwIP v2.0.0 or newer
	- `LT_HAS_MBEDTLS` - mbedTLS in SDK
	- `LT_HAS_PRINTF` - printf library implemented
	- `LT_HAS_FLASH` - FAL flash port implemented
	- `LT_HAS_OTA` - OTA implemented in base framework
- Arduino Core implementation status (only available and used along with Arduino framework)
	- `LT_ARD_HAS_SERIAL` - Serial class implemented
	- `LT_ARD_HAS_SOFTSERIAL` - SoftwareSerial library implemented
	- `LT_ARD_HAS_WIFI` - WiFi library implemented
	- `LT_ARD_HAS_WIRE` - Wire (I²C) library implemented
	- `LT_ARD_HAS_SPI` - SPI library implemented
	- `LT_ARD_MD5_MBEDTLS` - use mbedTLS for MD5 library
	- `LT_ARD_MD5_HOSTAPD` - use hostapd for MD5 library
- misc options
	- `LT_HEAP_FUNC` - function name used to get available heap size (for `LT_HEAP_I()`)
	- `LT_REALLOC_FUNC` - function name used for `realloc()` call
	- `LT_REMALLOC` - use `malloc()` and `memcpy()` in `realloc()` call
