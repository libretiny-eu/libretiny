# Flashing - Beken 72xx

- [Flashing (Tuya manual)](https://developer.tuya.com/en/docs/iot/burn-and-authorize-wb-series-modules?id=Ka78f4pttsytd)
- [BkWriter v1.6.0](https://images.tuyacn.com/smart/bk_writer1.60/bk_writer1.60.exe)

Downloading is done using UART. For best experience, you should have two USB<->UART adapters plugged in (for UART1 and UART2).

!!! hint
	The adapter connected to UART1 (for uploading) can have RTS connected to chip's RESET or CEN - to provide auto-reset during uploading.

	If you're not using auto-reset, you'll have to reset the chip manually when upload starts (you have 10 seconds to do that).

- UART1 is used for uploading the code. This adapter will be used by PlatformIO.
- UART2 allows for log output. You can have a terminal session continuously open on this adapter.

!!! hint
	If you only have a single adapter, or just want to use the UART1 (upload) port only, you can change the logging port.

	Refer to [Options & config](../../reference/config.md) (`Serial output` section). Set `LT_UART_DEFAULT_PORT` to `1`, which will use UART1 for all output.

## Automatically - using PlatformIO (recommended)

LibreTuya has built-in firmware uploaders. Pressing `Upload` in PlatformIO IDE does all the work for you.

Although the default options work in most cases, it is recommended to configure your PIO project first:

```ini
[env:my_board]
# default baudrate for both UARTs
monitor_speed = 115200
# port connected to UART2
monitor_port = COM60

# port connected to UART1
upload_port = COM96
```

## Manually - using `uf2upload`

{%
   include-markdown "../../ota/flashing.md"
%}

## Manually - using raw binaries/BkWriter

A binary file named `bk7231t_app_0x011000.rbl` is generated in `.pio/build/my_board/`. This file can be flashed to offset 0x11000, using BkWriter or other flashing tool of choice (including SPI download or flashrom).

Keep in mind that BkWriter is an extremely buggy and broken tool, and simply does not work in most cases.
