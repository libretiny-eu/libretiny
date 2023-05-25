# Auto-download-reboot

*(CEN-less uploading)*

This feature allows to upload code using UART, without needing to ground the CEN wire or power-cycle the device.

It is enabled by default (using the `LT_AUTO_DOWNLOAD_REBOOT` option). It works by listening to incoming UART data, and checking if it matches a command that the flashing program would send. If it does, a chip reboot is performed and the uploading process starts.

!!! note
	ADR will only work if there's already a recent build of LibreTiny flashed to the device (and if the device doesn't bootloop or freeze immediately).

## Beken 72xx

The code listens on UART1 for a link-check command (`01 E0 FC 01 00`). The baudrate configured on the serial port has to be 115200 - it is configured automatically upon booting, but ADR won't work anymore if you change the baudrate manually. Because BK72xx doesn't have a dedicated "persistent" download mode, a normal reboot is performed and the chip waits a few hundred milliseconds for another link-check command.

## Realtek AmebaZ

This only works when using [ltchiptool](ltchiptool.md) for flashing. Upon starting UART communication, the tool sends `55 AA 22 E0 D6 FC` (0x55AA followed by the `realtek-ambz` family ID). After detecting that pattern, the chip proceeds to reboot into UART download mode (using [`lt_reboot_download_mode()`](../../../ltapi/lt__device_8h.md))

## Realtek AmebaZ2

The code listens on UART2 for a `ping\n` command, that is sent by [ltchiptool](ltchiptool.md) (and possibly by the vendor flasher, too). The device is then rebooted to download mode.
