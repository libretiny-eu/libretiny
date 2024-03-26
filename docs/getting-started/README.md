# Getting started

Using LibreTiny is simple, just like every other PlatformIO development platform.

1. [Install PlatformIO](https://platformio.org/platformio-ide)
2. `pio pkg install --platform libretiny`

!!! tip
	See the [Cloudcutter video guide](https://www.youtube.com/watch?v=sSj8f-HCHQ0) for a complete tutorial on flashing with [Cloudcutter](https://github.com/tuya-cloudcutter/tuya-cloudcutter) and installing [LibreTiny-ESPHome](../projects/esphome.md). **Includes Home Assistant Add-On setup.**

	More information on Cloudcutter can be found in the [Tuya Cloudcutter GitHub repository](https://github.com/tuya-cloudcutter/tuya-cloudcutter).

## Board selection

1. Navigate to the [supported boards & CPUs](../status/supported.md) list.
2. Find the board your device has (usually, the model number is written on the silkscreen). \
	*If your board isn't available yet, use one of the "Generic" boards that matches the CPU you your board has.*
3. Click on the board name. A new page opens with information about the selected board name.
4. Scroll down to the "Usage" section of the page, and copy the configuration section that is relevant to your use case.

## Run community projects

LibreTiny was developed with popular community projects in mind.

There is an official [ESPHome port](../projects/esphome.md) available and integrated into the ESPHome project. No extra downloads or code compilations are needed to use ESPHome with LibreTiny-supported platforms.

## Develop your own project

Developing your own embedded software that runs on LibreTiny-supported platforms.

To get started with LibreTiny on your chosen platform, create a new project using your preferred method.

There's a few ways to create a new PlatformIO Project. For example:

- using the PlatformIO IDE graphical interface (PIO Home -> Open -> New Project)
- by running the `pio project init` command in your desired project directory \
	*See the [PlatformIO Core (CLI) documentation] for information on `pio project init` and other command line usage.*

Next, read one of the [flashing guides](../flashing/SUMMARY.md) to upload and run your project!

### Config options (platformio.ini)

LibreTiny has a few configuration options that allow you to change its behavior or features. These can be defined, specified or changed in the platformio.ini configuration file for your project.

Refer to the LibreTiny [Configuration](../dev/config.md) manual for details.

### GPIO usage - important change

!!! important
	As of LibreTiny release v1.0.0, references to GPIO pins using their `D#` numbers has been **deprecated** and should **no longer be used** in new projects.

	If your program is using Arduino I/O functions, please refer to the [migration guide](../dev/migration_v1.0.0.md) on how to modify your code accordingly.

### Examples

{%
	include-markdown "../../examples/SUMMARY.md"
	start="# Examples\n"
%}

<!-- These are reference URL's, which can be used to shorten inline hyperlinks to just their contextual title or text. -->
[PlatformIO Core (CLI) documentation]: https://docs.platformio.org/en/stable/core/quickstart.html#setting-up-the-project
