# Getting started

Using LibreTuya is simple, just like every other PlatformIO development platform.

1. [Install PlatformIO](https://platformio.org/platformio-ide)
2. `platformio platform install https://github.com/kuba2k2/libretuya`

## Board selection

- Go to [Boards & CPU list](../status/supported.md).
- Find the board your device has (usually, the model number is written on the silkscreen).
	- If your board isn't available yet, use one of the "Generic" boards that matches the CPU you have.
- Click on the board name. From the documentation page, note the board code.
- Use this code to create a PlatformIO project.

## GPIO usage

!!! important
	This can be confusing at first, so make sure to read this part carefully to understand it.

	Input/output pin numbers in Arduino code (i.e. `digitalWrite()`) use Arduino pin numbers - for example `D1`, `D3`. This is the same as simply `1` or `3`, but it cannot be confused with CPU GPIO numbers.

	On the board pinout page, the purple blocks represent Arduino pins, while the dark red blocks refer to GPIO numbers.

	Projects like ESPHome also use Arduino pin numbering.

## Develop your own project

If you're developing your own embedded software, and want it to run on LibreTuya-supported chips, create a project.

- use PlatformIO IDE (PIO Home -> Open -> New Project)
- run `pio project init` in your desired project directory

Next, read [Uploading](uploading.md) guide to run your project!

## Run community projects

LibreTuya was developed with popular community projects in mind. Currently, unofficial [ESPHome port](../projects/esphome.md) is available ([the PR](https://github.com/esphome/esphome/pull/3509) will hopefully be merged into upstream at some point).
