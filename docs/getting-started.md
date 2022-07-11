# Getting started

Using LibreTuya is simple, just like every other PlatformIO development platform.

## For your own projects

If you're developing your own embedded software, and want it to run on LibreTuya-supported chips, the installation is simple:

1. [Install PlatformIO](https://platformio.org/platformio-ide)
2. `platformio platform install https://github.com/kuba2k2/libretuya`
3. Create a project, build it and upload to the chip

## Community projects

LibreTuya was developed with popular community projects in mind. Currently, unofficial ESPHome port is available (the PR will hopefully be merged into upstream at some point).

### ESPHome

Because ESPHome does not natively support running on non-ESP chips, you need to use a fork of the project.

Assuming you have PlatformIO, git and Python installed:

1. Open a terminal/cmd.exe, create `esphome` directory and `cd` into it.
2. `git clone https://github.com/kuba2k2/libretuya-esphome -b platform/libretuya` (you need the `platform/libretuya` branch)
3. Go to [Boards & CPU list](https://kuba2k2.github.io/libretuya/docs/supported/) and choose your board name.
4. Create a YAML config file for your device. You can either:
	- use `python -m esphome wizard yourdevice.yml` - type answers to the six questions the wizard asks
	- if your board isn't available in the wizard yet, use the manual YAML method below
	- write a config file manually:
		```yaml
		esphome:
		  name: yourdevice

		libretuya:
		  board: wr3

		logger:
		api:
		  password: ""
		ota:
		  password: ""

		wifi:
		  ssid: "YourWiFiSSID"
		  password: "SecretPa$$w0rd"
		  ap:
		    ssid: "Yourdevice Fallback Hotspot"
		    password: "Dv2hZMGZRUvy"
		```
5. Install LibreTuya: `platformio platform install https://github.com/kuba2k2/libretuya`. This step is necessary, as otherwise ESPHome will try to use a version from the registry, which is often outdated.
6. Edit the config file to use your installed LT version:
	```yaml
	libretuya:
	  board: wr3
	  framework:
	    version: latest
	```
7. `python -m esphome compile yourdevice.yml`
8. The binary file, ready for uploading, is now in `.esphome/build/yourdevice/.pioenvs/yourdevice/`. Refer to your board README to find appropriate flashing instructions.
