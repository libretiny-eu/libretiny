# Copyright (c) Kuba Szczodrzyński 2022-04-23.

from os.path import isdir, join

from SCons.Script import DefaultEnvironment

env = DefaultEnvironment()
platform = env.PioPlatform()

API_DIR = platform.get_package_dir("framework-arduino-api")
LT_ARDUINO_DIR = join(platform.get_dir(), "arduino", "libretuya")
assert isdir(API_DIR)
assert isdir(LT_ARDUINO_DIR)

# Sources - ArduinoCore-API
env.AddLibrary(
    name="arduino_api",
    base_dir=API_DIR,
    srcs=[
        "+<api/Common.cpp>",
        "+<api/IPAddress.cpp>",
        "+<api/PluggableUSB.cpp>",
        "+<api/Print.cpp>",
        "+<api/Stream.cpp>",
        "+<api/String.cpp>",
    ],
    includes=[
        "!<.>",
        "!<api/deprecated>",
    ],
)

# Sources - LibreTuya API
env.AddLibrary(
    name="libretuya_api",
    base_dir=LT_ARDUINO_DIR,
    srcs=[
        "+<api/*.c*>",
        "+<common/*.c*>",
        "+<core/*.c*>",
        "+<libraries/**/*.c*>",
        "+<port/**/*.c*>",
        "+<posix/*.c>",
    ],
    includes=[
        "!<.>",
        "!<compat>",
        "!<core>",
        "!<libraries/*>",
        "!<port/*>",
        "!<posix>",
    ],
)

# Sources - external library ports
env.AddLibraryFlashDB(version="03500fa")

# Build all libraries
env.BuildLibraries(safe=False)
