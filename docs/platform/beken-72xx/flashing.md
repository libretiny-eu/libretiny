# Download mode - Beken 72xx

Downloading is done using UART. For best experience, you should have two USB<->UART adapters plugged in:

- One for flashing, preferably a real FT232RL or a good alternative. This connects to UART1 of the chip.
- One for log output - BK72xx outputs messages on a separate port. You can have a terminal session continuously open on this adapter. This connects to UART2 of the chip - but it's not necessary for flashing.

**Read [Using ltchiptool](../../flashing/tools/ltchiptool.md) to learn the flashing procedure**

!!! success "Wiring"
	Connect UART1 of the BK7231 to the USB-TTL adapter:

	PC  | BK7231
	----|-----------------------
	RX  | TX1 (GPIO11 / P11)
	TX  | RX1 (GPIO10 / P10)
	RTS | CEN (or RST, optional)
	GND | GND

	Make sure to use a good 3.3V power supply, otherwise the adapter might
	lose power during chip reset. Usually, the adapter's power regulator
	is not enough and an external power supply is needed (like AMS1117).

	If you didn't connect RTS to CEN, after starting the flasher you have
	around 20 seconds to reset the chip manually. In order to do that,
	you need to bridge CEN to GND with a wire.

Note that the download mode can only be activated when the flasher is running (there's no GPIO-strapping like on ESP8266). Additionally, BK7231T (not N) will exit the download mode when the flasher finishes its work.

!!! tip
	BK7231N can't be software-bricked, because it has a ROM that contains the download mode. **BK7231T doesn't contain the ROM, so be careful with this one**.

## bk7231tools

`ltchiptool`'s Beken flashing program is based on [bk7231tools](https://github.com/tuya-cloudcutter/bk7231tools). Refer to the guide for information how to use it, but keep in mind that using the ltchiptool GUI is probably just easier.

## Auto-download-reboot

If you have a recent version of LibreTuya installed on the chip, you can use [Auto-download-reboot](../../flashing/tools/adr.md) to reboot the chip automatically. This is enabled by default, so you don't have to change anything.

## Single-adapter usage

If you only have a single adapter, or just want to use the UART1 (upload) port only, you can change the logging port.

Refer to [Options & config](../../reference/config.md) (`Serial output` section). Set `LT_UART_DEFAULT_PORT` to `1`, which will use UART1 for all output.

## Firmware output files

These files are present in the build directory after successful compilation:

File                      | Description
--------------------------|----------------------------------------
**firmware.uf2**          | **UF2 package for UART and OTA upload**
bk7231t_app.ota.rbl       | Beken OTA package (e.g. OpenBeken)
bk7231t_app.ota.ug.bin    | Tuya OTA package (incl. Cloudcutter)
bk7231t_app_0x011000.rbl  | App partition - flashable at 0x11000
bk7231t_app_0x011000.crc  | Encrypted app image - not for flashing
bk7231t_app_0x129F0A.rblh | RBL header - not for flashing

## SPI flashing (unbricking BK7231T)

The [bk7231_spi_flasher.py](https://github.com/openshwprojects/BK7231_SPI_Flasher/blob/main/bk7231_spi_flasher.py) script can be used to put BK7231 in SPI flashing mode. Then, one can use [flashrom](https://www.flashrom.org/Flashrom) to read/write the raw flash chip.

## Other tools/guides

These tools are **not recommended** and are kept here for reference only. Don't use them, please.

- [Flashing (Tuya manual)](https://developer.tuya.com/en/docs/iot/burn-and-authorize-wb-series-modules?id=Ka78f4pttsytd)
- [BkWriter v1.6.0](https://images.tuyacn.com/smart/bk_writer1.60/bk_writer1.60.exe)
- [hid_download_py](https://github.com/tiancj/hid_download_py)
