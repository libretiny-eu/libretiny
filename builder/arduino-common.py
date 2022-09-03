# Copyright (c) Kuba Szczodrzyński 2022-04-23.

from os.path import isdir, join

from SCons.Script import DefaultEnvironment

env = DefaultEnvironment()
platform = env.PioPlatform()

API_DIR = platform.get_package_dir("framework-arduino-api")
LT_ARDUINO_DIR = join(platform.get_dir(), "arduino", "libretuya")
assert isdir(API_DIR)
assert isdir(LT_ARDUINO_DIR)

# Flags
env.Append(
    CPPDEFINES=[
        ("LIBRETUYA_ARDUINO", "1"),
        ("ARDUINO", 10812),
        "ARDUINO_SDK",
    ],
    LINKFLAGS=[
        "--specs=nosys.specs",
        "-Wl,--as-needed",
        "-Wl,--build-id=none",
        "-Wl,--cref",
        "-Wl,--no-enum-size-warning",
        "-Wl,--no-undefined",
        "-Wl,--warn-common",
        # wrappers from port/printf/
        "-Wl,-wrap,putchar",
        "-Wl,-wrap,puts",
        # wrappers from posix/time.c
        "-Wl,-wrap,gettimeofday",
        "-Wl,-wrap,settimeofday",
    ],
)
# Arduino core uses __libc_init_array
if "-nostartfiles" in env["LINKFLAGS"]:
    env["LINKFLAGS"].remove("-nostartfiles")

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
        "+<api/**/*.c*>",
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

# Sources - uf2ota library
ltchiptool_dir = platform.get_package_dir(f"tool-ltchiptool")
env.AddLibrary(
    name="uf2ota",
    base_dir=ltchiptool_dir,
    srcs=[
        "+<uf2ota/*.c>",
    ],
    includes=[
        "+<.>",
    ],
)

# Sources - board variant
env.AddLibrary(
    name="board_${VARIANT}",
    base_dir="$BOARD_DIR",
    srcs=[
        "+<variant.cpp>",
    ],
)

for code, base_dir in env["ARDUINO_DIRS"].items():
    code = env.subst(code)
    base_dir = env.subst(base_dir)
    if not code or not isdir(base_dir):
        # Skip unused paths
        continue

    # Sources - Arduino Core
    env.AddLibrary(
        name=f"{code}_arduino_core",
        base_dir=base_dir,
        srcs=[
            # Wiring core
            "+<cores/arduino/*.c>",
            "+<cores/arduino/*.cpp>",
        ],
        includes=[
            # prepend these as the Arduino core may be incorrectly picking some includes from SDKs
            "!<cores/arduino>",
        ],
    )

    # Sources - Arduino libraries
    env.AddLibrary(
        name=f"{code}_arduino_libs",
        base_dir=base_dir,
        srcs=[
            "+<libraries/*/*.cpp>",
        ],
        includes=[
            "+<libraries/*>",
        ],
    )

    # Sources - external library ports
    env.AddLibrary(
        name=f"{code}_arduino_port",
        base_dir=base_dir,
        srcs=[
            "+<port/**/*.c*>",
        ],
        includes=[
            "+<port/*>",
        ],
    )

# Sources - external library ports
env.AddLibraryFlashDB(version="03500fa")
env.AddLibraryPrintf(version="6.0.0")

# Libs & linker config
env.Append(
    LIBS=[
        "stdc++",
        "supc++",
    ],
)
env.Replace(
    # Change to Arduino linker script
    LDSCRIPT_PATH=["${LDSCRIPT_ARDUINO}"],
)

# Build all libraries
env.BuildLibraries(safe=False)
