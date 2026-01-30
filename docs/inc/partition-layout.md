!!! warning "Important"
	When you compile firmware with LibreTiny (either ESPHome or other PlatformIO projects), you need to choose a **board**. Different boards *may* have different **partition layouts** - the main difference is the OTA2 firmware address. Choosing a board with wrong address will make it harder to flash OTA updates.

	Flashing over UART (with ltchiptool) will adjust the on-chip OTA address to match the firmware being flashed - you can safely change boards this way.

	However, **OTA flashing will not update the address** - make sure to choose the correct board, and **keep using the same board** for OTA flashing.

	Using incorrect boards may result in OTA updates having either no effect, resulting in errors, or (in worst cases) bricking the device completely.
