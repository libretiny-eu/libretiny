# Download mode - Realtek AmebaZ

<div align="center" markdown>

[Read chip docs](README.md){ .md-button }
</div>

Downloading is done using UART2 (sometimes called Log_UART). Refer to your board documentation to find the correct pins. You need a good USB<->UART adapter (preferably a real FT232RL) for the process.

**Read [Using ltchiptool](../../flashing/tools/ltchiptool.md) to learn the flashing procedure**

!!! success "Wiring"
	Connect UART2 of the Realtek chip to the USB-TTL adapter:

	PC  | RTL8710B
	----|------------------------------
	RX  | TX2 (Log_TX / PA30)
	TX  | RX2 (Log_RX / PA29)
	RTS | CEN (or RST, optional)
	DTR | TX2 (Log_TX / PA30, optional)
	GND | GND

	Make sure to use a good 3.3V power supply, otherwise the adapter might
	lose power during chip reset. Usually, the adapter's power regulator
	is not enough and an external power supply is needed (like AMS1117).

	If you didn't connect RTS and DTR, you need to put the chip in download
	mode manually. This is done by connecting CEN to GND, while holding TX2 (Log_TX)
	to GND as well. After doing that, you need to disconnect TX2 from GND.

	If the download mode is enabled, you'll see a few garbage characters
	printed to the serial console every second.

!!! tip
	Because the UART uploading code is programmed in the ROM of the chip, it can't be software-bricked, even if you damage the bootloader.

## Firmware output files

These files are present in the build directory after successful compilation:

File                    | Description
------------------------|-------------------------------------------------------------------
**firmware.uf2**        | **UF2 package for UART and OTA upload**
image_0x00B000.ota1.bin | OTA 1 image, flashable to 0xB000
image_0x0D0000.ota2.bin | OTA 2 image, flashable to 0xD0000 (the address might be different)

## Other tools/guides

These tools are **not recommended** and are kept here for reference only. Don't use them, please.

- [Flashing (Tuya manual)](https://developer.tuya.com/en/docs/iot/burn-and-authorize-wr-series-modules?id=Ka789pjc581u8)
- [ImageTool (AmebaZ/AmebaD)](https://images.tuyacn.com/smart/Image_Tool/Image_Tool.zip)
- [rtltool.py](https://github.com/libretuya/ltchiptool/blob/master/ltchiptool/soc/ambz/util/rtltool.py)

OTA1/2 files can be flashed using `ImageTool_v2.3.1_AmebaZ(8710b)`. Browse and select one of the files and enter an appropriate address. Select COM port, press `Open` and then `Download`.

This method is not recommended, as it requires you to know the currently enabled OTA index (1 or 2). Flashing the wrong file will either not make any changes, or upload firmware which won't run.
