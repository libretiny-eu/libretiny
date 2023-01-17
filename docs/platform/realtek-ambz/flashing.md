# Flashing - Realtek AmebaZ

- [Flashing (Tuya manual)](https://developer.tuya.com/en/docs/iot/burn-and-authorize-wr-series-modules?id=Ka789pjc581u8)
- [ImageTool (AmebaZ/AmebaD)](https://images.tuyacn.com/smart/Image_Tool/Image_Tool.zip)

Downloading is done using UART2 (sometimes called Log_UART). Refer to your board documentation to find the correct pins.

!!! important
	It is required to put the chip into download mode, prior to flashing. This can be done by resetting the chip, while pulling UART2_TX to GND. If successful, the chip should print few garbage characters every second.

## Automatically - using PlatformIO (recommended)

LibreTuya has built-in firmware uploaders. Pressing `Upload` in PlatformIO IDE does all the work for you.

If you have more than one COM port,configure your PIO project first:

```ini
[env:my_board]
monitor_port = COM60
upload_port = COM60
```

## Manually - using `uf2upload`

{%
	include-markdown "../../ota/flashing.md"
%}

## Manually - using raw binaries/BkWriter

There are two binary files in `.pio/build/my_board/`:

- `image_0x00B000.ota1.bin`
- `image_0x0D0000.ota2.bin`

They can be flashed using `ImageTool_v2.3.1_AmebaZ(8710b)`. Browse and select one of the files and enter an appropriate address. Select COM port, press `Open` and then `Download`.

This method is not recommended, as it requires you to know the currently enabled OTA index (1 or 2). Flashing the wrong file will either not make any changes, or upload firmware which won't run.

!!! hint
	Because the UART uploading code is programmed in the ROM of the chip, it can't be software-bricked, even if you damage the bootloader.
