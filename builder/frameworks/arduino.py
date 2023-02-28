# Copyright (c) Kuba Szczodrzyński 2022-04-23.

from os.path import join

import click
from ltchiptool import Family
from SCons.Script import DefaultEnvironment, Environment

# Let everyone know we're using the Arduino framework
env: Environment = DefaultEnvironment()
env["ARDUINO"] = True
family: Family = env["FAMILY_OBJ"]

# Add base cores' sources first
env.SConscript("base.py")

# Build a safe environment for this script
queue = env.AddLibraryQueue("arduino", prepend_includes=True)

# Add sources common among all families
env.AddCoreSources(
    queue=queue,
    name="common_arduino",
    path=join("$COMMON_DIR", "arduino", "src"),
)
env.AddArduinoLibraries(
    queue=queue,
    name="common_arduino",
    path=join("$COMMON_DIR", "arduino", "libraries"),
)
# Add sources for this family and each parent
found = False
for f in family.inheritance:
    code = f"{f.code}_arduino"
    path = join("$CORES_DIR", f.name, "arduino")

    found = found or env.AddCoreSources(queue, name=code, path=join(path, "src"))
    env.AddArduinoLibraries(queue, name=code, path=join(path, "libraries"))

    if f.short_name:
        env.Prepend(CPPDEFINES=[(f"ARDUINO_ARCH_{f.short_name}", "1")])
    if f.code:
        env.Prepend(CPPDEFINES=[(f"ARDUINO_ARCH_{f.code.upper()}", "1")])

# Fail if Arduino core wasn't found
if not found:
    click.secho(
        f"Platform '{family.name}' doesn't support Arduino framework - "
        "the Arduino core source files are absent.",
        fg="red",
    )
    exit(1)

# Sources - ArduinoCore-API
queue.AddExternalLibrary("arduino_api")

# Sources - board variant
queue.AddLibrary(
    name="board_${VARIANT}",
    base_dir="$BOARD_DIR",
    srcs=[
        "+<variant.cpp>",
    ],
    # not adding includes since they're added with the base core
)

# Flags & linker options
queue.AppendPublic(
    CPPDEFINES=[
        ("LIBRETUYA_ARDUINO", 1),
        ("ARDUINO", 10812),
        ("ARDUINO_SDK", 1),
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

# Build all libraries
queue.BuildLibraries()
