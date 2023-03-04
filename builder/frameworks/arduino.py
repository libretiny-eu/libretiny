# Copyright (c) Kuba Szczodrzyński 2022-04-23.

from os.path import join

import click
from ltchiptool import Family
from SCons.Script import DefaultEnvironment, Environment

# Let everyone know we're using the Arduino framework
env: Environment = DefaultEnvironment()
env["ARDUINO"] = True

# Add base cores' sources first
env.SConscript("base.py")

# Flags & linker options
env.Append(
    CPPDEFINES=[
        ("LIBRETUYA_ARDUINO", 1),
        ("ARDUINO", 10812),
        ("ARDUINO_SDK", 1),
        ("ARDUINO_ARCH_${FAMILY_CODE}", 1),
    ],
    LINKFLAGS=[
        "--specs=nosys.specs",
        "-Wl,--as-needed",
        "-Wl,--build-id=none",
        "-Wl,--cref",
        "-Wl,--no-enum-size-warning",
        "-Wl,--no-undefined",
        "-Wl,--warn-common",
        # wrappers from posix/time.c
        "-Wl,-wrap,gettimeofday",
        "-Wl,-wrap,settimeofday",
    ],
)

family: Family = env["FAMILY_OBJ"]

# Add common sources among all families
env.AddCoreSources(
    name="common_arduino",
    path=join("$COMMON_DIR", "arduino", "src"),
)
env.AddArduinoLibraries(
    name="common_arduino",
    path=join("$COMMON_DIR", "arduino", "libraries"),
)
# Add sources for this family and each parent
found = False
for f in family.inheritance:
    code = f"{f.code}_arduino"
    path = join("$CORES_DIR", f.name, "arduino")
    # Add libraries first, to put the include paths after core sources
    env.AddArduinoLibraries(name=code, path=join(path, "libraries"))
    found = found or env.AddCoreSources(name=code, path=join(path, "src"))

# Fail if Arduino core wasn't found
if not found:
    click.secho(
        f"Platform '{family.name}' doesn't support Arduino framework - "
        "the Arduino core source files are absent.",
        fg="red",
    )
    exit(1)

# Sources - ArduinoCore-API
env.AddExternalLibrary("arduino_api")

# Sources - board variant
env.AddLibrary(
    name="board_${VARIANT}",
    base_dir="$BOARD_DIR",
    srcs=[
        "+<variant.cpp>",
    ],
    # not adding includes since they're added in base.py
)

# Build all libraries
env.BuildLibraries()
