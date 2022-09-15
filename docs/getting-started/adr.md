# Auto-download-reboot

*(CEN-less uploading)*

This feature allows to upload code using UART, without needing to ground the CEN wire or power-cycle the device.

It is enabled by default (using the `LT_AUTO_DOWNLOAD_REBOOT` option). It works by listening to incoming UART data, and checking if it matches a command that the flashing program would send. If it does, a chip reboot is performed and the uploading process starts.

!!! note
	ADR will only work if there's already a recent build of LibreTuya flashed to the device (and if the device doesn't bootloop or freeze immediately).

## Beken 72xx

The code listens on UART1 for a link-check command (`01 E0 FC 01 00`). The baudrate configured on the serial port has to be 115200 - it is configured automatically upon booting, but ADR won't work anymore if you change the baudrate manually. Because BK72xx doesn't have a dedicated "persistent" download mode, a normal reboot is performed and the chip waits a few hundred milliseconds for another link-check command.

## Realtek AmebaZ

This is not yet implemented.
