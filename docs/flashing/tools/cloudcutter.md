# Converting with tuya-cloudcutter

!!! tip
	See the [Cloudcutter video guide](https://www.youtube.com/watch?v=sSj8f-HCHQ0) for a complete tutorial on flashing with [Cloudcutter](https://github.com/tuya-cloudcutter/tuya-cloudcutter) and installing [LibreTiny-ESPHome](../../projects/esphome.md). **Includes Home Assistant Add-On setup.**

!!! note
	This currently applies to BK7231T and BK7231N only. `tuya-cloudcutter` can't be used for other chips.

[`tuya-cloudcutter`](https://github.com/tuya-cloudcutter/tuya-cloudcutter) is a tool that disconnects IoT devices from the Tuya cloud, while also allowing **remote firmware flashing**. This means you can flash ESPHome without even disassembling it.

!!! warning
	This guide might be outdated. For an up-to-date guide, always refer to [tuya-cloudcutter/INSTRUCTIONS.md](https://github.com/tuya-cloudcutter/tuya-cloudcutter/blob/main/INSTRUCTIONS.md).

If your device doesn't have a profile yet, it will probably not work. You can contribute by [taking a device dump](https://github.com/tuya-cloudcutter/tuya-cloudcutter#device-dumps) and posting it on cloudcutter's issues page.

## Instructions

### Preparation

1. Get a laptop (or a PC with Wi-Fi) with Linux and Docker installed. This was tested on Ubuntu 20.04, but you should be able to use another Debian-based distribution with NetworkManager.
	- To install Docker, run `sudo apt-get install docker.io`. When it completes, run `sudo adduser <your username> docker` and reboot the machine.
	- This was also successfully performed on a VirtualBox VM, with a USB Wi-Fi adapter redirected to the VM.
2. Install `git`, if you haven't already (`sudo apt-get install git`).
3. `git clone https://github.com/tuya-cloudcutter/tuya-cloudcutter`

### Firmware building

1. [Compile ESPHome](../../projects/esphome.md), or your custom firmware based on LibreTiny.
2. Get the firmware binary, named `bk7231x_app.ota.ug.bin` from the build directory (`.pio/build/<board>/` or `.esphome/build/<board>/.pioenvs/<board>/`).
3. Put it in the `custom-firmware` directory of tuya-cloudcutter.

### Pairing and flashing

1. Run `./tuya-cloudcutter.sh` from the cloudcutter directory.
2. Answer questions about the desired firmware file, the device vendor and profile.
3. Put the device to AP mode: (**)
	- Bulbs (devices without buttons) usually need to be power-cycled a few times, until they start blinking slowly.
	- Switches, plugs, relays (devices with buttons) usually enable AP after pressing the button for a few seconds.
	- If the device (bulb or switch LED) is blinking quickly (~2 times per second), do the procedure again.
4. Cloudcutter will scan for APs, connect to the device and send a payload to it.
5. The device will most likely hang (not respond). Reboot it again to AP mode (just like in step 9).
6. Cloudcutter will scan for APs again, configure the device to talk to it, then begin the OTA update.
7. After around 30 seconds, the device will boot new firmware 👏

\*\* Use a smartphone with the Wi-Fi screen open and scanning, so that you can see if AP mode got enabled.
