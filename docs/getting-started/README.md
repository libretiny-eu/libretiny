# Getting started

Using LibreTiny is simple, just like every other PlatformIO development platform.

1. [Install PlatformIO](https://platformio.org/platformio-ide)
2. `platformio platform install -f https://github.com/kuba2k2/libretiny`

!!! tip
	See the [Cloudcutter video guide](https://www.youtube.com/watch?v=sSj8f-HCHQ0) for a complete tutorial on flashing with [Cloudcutter](https://github.com/tuya-cloudcutter/tuya-cloudcutter) and installing [LibreTiny-ESPHome](../projects/esphome.md). **Includes Home Assistant Add-On setup.**

## Board selection

- Go to [Boards & CPU list](../status/supported.md).
- Find the board your device has (usually, the model number is written on the silkscreen).
	- If your board isn't available yet, use one of the "Generic" boards that matches the CPU you have.
- Click on the board name. From the documentation page, note the board code.
- Use this code to create a PlatformIO project.

## Run community projects

LibreTiny was developed with popular community projects in mind. Currently, unofficial [ESPHome port](../projects/esphome.md) is available ([the PR](https://github.com/esphome/esphome/pull/3509) will hopefully be merged into upstream at some point).

## Develop your own project

If you're developing your own embedded software, and want it to run on LibreTiny-supported chips, create a project.

- use PlatformIO IDE (PIO Home -> Open -> New Project)
- run `pio project init` in your desired project directory

Next, read one of the [flashing guides](../flashing/SUMMARY.md) to run your project!

### LT configuration

LibreTiny has a few configuration options that change its behavior or features. Refer to [LT configuration](../dev/config.md) for details.

### GPIO usage - important change

!!! important
	Since v1.0.0, GPIOs are no longer meant to be referenced by `D#` numbers.

	If your program is using Arduino I/O functions, refer to the [Migration guide](../dev/migration_v1.0.0.md) to modify your program accordingly.

### Examples

{%
	include-markdown "../../examples/SUMMARY.md"
   start="# Examples\n"
%}
