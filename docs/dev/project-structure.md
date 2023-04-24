# Project structure

```
arduino/
├─ <family name>/               Arduino Core for a specific SoC family
│  ├─ cores/                        Wiring core files
│  ├─ libraries/                    Supported built-in family libraries
│  ├─ port/                     External library port units
├─ libretiny/
│  ├─ api/                      Library interfaces
│  ├─ common/                   Units common to all families
│  ├─ compat/                   Fixes for compatibility with ESP32 framework
│  ├─ core/                     LibreTiny API for Arduino cores
│  ├─ libraries/                Built-in family-independent libraries
|  ├─ port/                     External library port units
|  ├─ posix/                    POSIX-like C utility functions
boards/
├─ _base/                       Base board manifests
├─ <board name>/                Board-specific code
│  ├─ variant.cpp                   Arduino variant initialization
│  ├─ variant.h                     Arduino variant pin configs
├─ <board name>.json            PlatformIO board description
builder/
├─ frameworks/                  Framework builders for PlatformIO
│  ├─ <family name>-sdk.py          Vanilla SDK build system
│  ├─ <family name>-arduino.py      Arduino Core build system
├─ libs/                        Builders for external libraries
├─ utils/                       SCons utils used during the build
├─ arduino-common.py            Builder to provide ArduinoCore-API and LibreTiny APIs
├─ main.py                      Main PlatformIO builder
docs/                           Project documentation, guides, tips, etc.
platform/
├─ <family name>/               Family-specific configurations
│  ├─ bin/                          Binary blobs (bootloaders, etc.)
│  ├─ fixups/                       Code fix-ups to replace SDK parts
│  ├─ ld/                           Linker scripts
│  ├─ openocd/                      OpenOCD configuration files
tools/
├─ <tool name>/                     Tools used during the build
├─ util/                            Utilities used by CLI tools and the builders
families.json                   List of supported device families
platform.json                   PlatformIO manifest
platform.py                     Custom PlatformIO script
```
