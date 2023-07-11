# Download mode - Realtek AmebaZ

<div align="center" markdown>

[Read chip docs](README.md){ .md-button }
</div>

Downloading is done using UART2 (sometimes called Log_UART). Refer to your board documentation to find the correct pins.

!!! tip
    You need a good USB<->UART adapter for the process. Some chips may not support 1.5M baud rate,
	required by the ROM for the initial handshake. Widespread PL2303 is currently known not to work,
	at least under Windows. FT232RL is verified to work reliably.

**Read [Using ltchiptool](../../flashing/tools/ltchiptool.md) to learn the flashing procedure**

!!! success "Wiring"
	Connect UART2 of the Realtek chip to the USB-TTL adapter:

	PC  | RTL8710B
	----|------------------------------
	RX  | TX2 (Log_TX / PA30)
	TX  | RX2 (Log_RX / PA29)
	GND | GND

	Make sure to use a good 3.3V power supply, otherwise the adapter might
	lose power during chip reset. Usually, the adapter's power regulator
	is not enough and an external power supply is needed (like AMS1117).

	You need to put the chip in download mode manually:

	- connect CEN to GND
	- connect TX2 to GND
	- release CEN from GND
	- release TX2 from GND

	If the download mode is enabled, you'll see a few garbage characters
	printed to the serial console every second.


!!! tip
	Because the UART uploading code is programmed in the ROM of the chip, it can't be software-bricked, even if you damage the bootloader.

## Firmware output files

These files are present in the build directory after successful compilation:

File                    | Description
------------------------|-------------------------------------------------------------------
**firmware.uf2**        | **UF2 package for UART and OTA upload**
image_ota1.0x00B000.bin | OTA 1 image, flashable to 0xB000
image_ota2.0x0D0000.bin | OTA 2 image, flashable to 0xD0000 (the address might be different)

## Other tools/guides

These tools are **not recommended** and are kept here for reference only. Don't use them, please.

- [Flashing (Tuya manual)](https://developer.tuya.com/en/docs/iot/burn-and-authorize-wr-series-modules?id=Ka789pjc581u8)
- [ImageTool (AmebaZ/AmebaD)](https://images.tuyacn.com/smart/Image_Tool/Image_Tool.zip)
- [rtltool.py](https://github.com/libretiny-eu/ltchiptool/blob/master/ltchiptool/soc/ambz/util/rtltool.py)

OTA1/2 files can be flashed using `ImageTool_v2.3.1_AmebaZ(8710b)`. Browse and select one of the files and enter an appropriate address. Select COM port, press `Open` and then `Download`.

This method is not recommended, as it requires you to know the currently enabled OTA index (1 or 2). Flashing the wrong file will either not make any changes, or upload firmware which won't run.
