# ltchiptool

[ltchiptool](https://github.com/libretiny-eu/ltchiptool) is a universal, easy-to-use GUI flashing/dumping tool for BK7231, RTL8710B and RTL8720C. It also contains some CLI utilities for binary firmware manipulation.

<div align="center" markdown>

[![Code style: black](https://img.shields.io/badge/code%20style-black-000000.svg)](https://github.com/psf/black)
[![PyPI](https://img.shields.io/pypi/v/ltchiptool)](https://pypi.org/project/ltchiptool/)

[![GitHub release (latest by date including pre-releases)](https://img.shields.io/github/v/release/libretuya/ltchiptool?include_prereleases&label=GUI%20release)](https://github.com/libretiny-eu/ltchiptool/releases/latest)

<img src="https://raw.githubusercontent.com/libretiny-eu/ltchiptool/master/.github/screenshot.png" style="height:500px">
</div>

## Installation

<div align="center" markdown>

[:material-download-outline: Download Windows GUI](https://github.com/libretiny-eu/ltchiptool/releases/latest){ .md-button }
</div>

### CLI program

Install the package from PyPI, using `pip install ltchiptool`. Run the CLI using `python -m ltchiptool` or just `ltchiptool`.

### GUI application

???+ quote "Windows 7 and newer"
	Download the latest release .EXE from the [GitHub Releases page](https://github.com/libretiny-eu/ltchiptool/releases/latest). Open the file, and you're ready to go!

??? quote "Windows (manual installation)"
	Install the package from PyPI (**including GUI extras**), using `pip install ltchiptool[gui]`. Note that Python 3.10 or newer is required for the GUI. I recommend Python 3.10 since it has [prebuilt wheels](https://pypi.org/project/wxPython/4.2.0/#files) of wxPython, which doesn't require C++ build dependencies.

??? quote "Linux (Ubuntu)"
	Install the package from PyPI, using `pip install ltchiptool`. Python 3.10 or newer is required.

	Make sure you have wxPython installed. Install it from PyPI (if you have the necessary build dependencies), or refer to the [wxPython Downloads](https://wxpython.org/pages/downloads/index.html) page to install prebuilt wheels (recommended).

	Next, open a terminal and run `ltchiptool gui` (or `python -m ltchiptool gui`).

??? quote "MacOS (untested)"
	Install the package from PyPI, using `pip install ltchiptool`. Python 3.10 or newer is required.

	Install wxPython from PyPI as well. Version 4.2.0 (latest at the time of writing) has some wheels for MacOS, so that should work.

## GUI Usage

The main window is somewhat similar to [NodeMCU PyFlasher](https://github.com/marcelstoer/nodemcu-pyflasher). Available modes of operation are described below.

### Dumping firmware

It is a good idea to dump the stock firmware (full flash contents) of your device before flashing custom firmware.

1. Choose the `Read flash` option. If you've previously chosen an input or output file, it will generate a dump filename based on the current timestamp and chip type. Otherwise, click `Browse` and choose the output file.
2. You need to pick the "family" of your chip (the chip model). If you choose the wrong option, the process will fail, but the device won't be bricked.
3. Now, connect the chip to your PC, according to the [chip connection guides](../../platform/SUMMARY.md). Select the COM port that your UART adapter is using.
4. By default, the tool will attempt to read the entire flash chip (usually 2 MiB). Unless you know what you're doing, the default values don't need to be changed.
5. Hit `Start`. The tool will try to connect to the chip on the selected UART port. The black log window will print any warnings/errors. The dumping process should begin shortly.

### Flashing firmware

If you want to flash custom firmware, or restore stock firmware from a previously dumped file, follow the steps below.

!!! info
	LibreTiny generates multiple firmware files in the build directory. **You usually want to flash the `.uf2` file**, but since ltchiptool can detect file types, you can choose a different firmware file and it'll tell you if that works.

1. Choose `Write flash`. Click `Browse` and select a valid firmware file. The file type and chip type will be auto-detected, along with correct flash offset and length. No need to worry about overwriting the bootloader anymore!
2. Connect the chip to your PC, according to the [chip connection guides](../../platform/SUMMARY.md). Select the COM port that your UART adapter is using.
3. Hit `Start`. The tool will try to connect to the chip on the selected UART port. The black log window will print any warnings/errors. The flashing process should begin shortly.

!!! info
	**It's best to leave `Auto-detect advanced parameters` checked**. If you're an experienced user and want to flash custom areas of the flash, uncheck the box and specify the parameters manually.

	If the file you're selecting is `Unrecognized` or `Not flashable`, it's most likely not a valid firmware file. Refer to usage guides of the custom firmware project of choice, to find which file is meant for flashing.

## CLI Usage

### Flashing/dumping

This is for users, who are more experienced with using a terminal. There are three main commands used for flashing:

- `ltchiptool flash file <FILE>` - detect file type based on its contents (i.e. chip from which a dump was acquired), similar to Linux `file` command
- `ltchiptool flash read <FAMILY> <FILE>` - make a full flash dump of the connected device; specifying the family is required
- `ltchiptool flash write <FILE>` - upload a file to the device; detects file type automatically (just like the `file` command above)

Supported device families can be checked using `ltchiptool list families` command. In the commands above, you can use any of the family names (name/code/short name/etc).

The upload UART port and baud rate should be detected automatically. To override it, use `-d COMx` or `-d /dev/ttyUSBx`. To change the target baud rate, use `-b 460800`.
Note that the baud rate is changed after linking. Linking is performed using chip-default baud rate.

It's not required to specify chip family for writing files - `ltchiptool` tries to recognize contents of the file, and chooses the best settings automatically.
If you want to flash unrecognized/raw binaries (or fine-tune the flashing parameters), specify `-f <FAMILY>` and `-s <START OFFSET>`.

## CLI Reference

!!! note
	If you're here to learn how to flash or dump firmware files, use the instructions above.

	The content below serves as a short documentation page for [`ltchiptool`](https://github.com/libretiny-eu/ltchiptool) and is mostly meant for advanced users.

```console
$ ltchiptool --help
Usage: ltchiptool [OPTIONS] COMMAND [ARGS]...

  Tools for working with LT-supported IoT chips

Options:
  -v, --verbose         Output debugging messages (repeat to output more)
  -T, --traceback       Print complete exception traceback
  -t, --timed           Prepend log lines with timing info
  -r, --raw-log         Output logging messages with no additional styling
  -i, --indent INTEGER  Indent log messages using graph lines
  -V, --version         Show the version and exit.
  -h, --help            Show this message and exit.

Commands:
  dump      Capture or process device dumps
  elf2bin   Generate firmware binaries from ELF file
  flash     Flashing tool - reading/writing
  link2bin  Link code to binary format
  list      List boards, families, etc.
  soc       Run SoC-specific tools
  uf2       Work with UF2 files
```

### UF2 Example

```console
$ ltchiptool uf2 info ./arduinotest_22.08.01_wb2l_BK7231T_lt0.8.0.uf2
Family: BK7231T / Beken 7231T
Tags:
 - BOARD: wb2l
 - DEVICE_ID: d80e20c2
 - LT_VERSION: 0.8.0
 - FIRMWARE: arduinotest
 - VERSION: 22.08.01
 - OTA_VERSION: 01
 - DEVICE: LibreTiny
 - BUILD_DATE: 6d08e862
 - LT_HAS_OTA1: 01
 - LT_HAS_OTA2: 00
 - LT_PART_1: app
 - LT_PART_2:
Data chunks: 1871
Total binary size: 478788
```
