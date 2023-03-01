# Copyright (c) Kuba Szczodrzyński 2023-02-26.

from os.path import join

import click
from ltchiptool import Family
from platformio.platform.base import PlatformBase
from platformio.platform.board import PlatformBoardConfig
from SCons.Errors import UserError
from SCons.Script import DefaultEnvironment, Environment

env: Environment = DefaultEnvironment()
board: PlatformBoardConfig = env.BoardConfig()
platform: PlatformBase = env.PioPlatform()
family: Family = env["FAMILY_OBJ"]

# TODO remove include path prepending ("!<...>")
# Move common core sources (env.AddCoreSources()) and Arduino libs
# below per-family sources (to maintain child families taking precedence)

# Include SDK builder scripts
# No environment options that follow later will be considered
found = False
for f in family.inheritance:
    try:
        env.SConscript(f"../family/{f.name}.py", must_exist=True)
        found = True
    except UserError:
        pass
# Fail if no SDK builder was found
if not found:
    click.secho(
        f"Platform '{family.name}' is currently not supported - "
        "no SDK builder script could be found.",
        fg="red",
    )
    exit(1)

# Build a safe environment for this script
queue = env.AddLibraryQueue("base", prepend_includes=True)
# Add sources & include paths for each core
env.AddCoreSources(queue, name="common", path=join("$COMMON_DIR", "base"))
for f in family.inheritance:
    env.AddCoreSources(queue, name=f.code, path=join("$CORES_DIR", f.name, "base"))

    if f.short_name:
        env.Prepend(CPPDEFINES=[(f"LT_{f.short_name}", "1")])
    if f.code:
        env.Prepend(CPPDEFINES=[(f"LT_{f.code.upper()}", "1")])
    env.Prepend(LIBPATH=[join("$CORES_DIR", f.name, "misc")])

# Sources - external libraries
queue.AddExternalLibrary("ltchiptool")  # uf2ota source code
queue.AddExternalLibrary("flashdb")
queue.AddExternalLibrary("printf")

# Flags & linker options
queue.AppendPublic(
    CFLAGS=[
        "-Werror=implicit-function-declaration",
    ],
    CPPDEFINES=[
        ("LIBRETUYA", 1),
        ("LT_VERSION", env.ReadLTVersion(platform.get_dir(), platform.version)),
        ("LT_BOARD", "${VARIANT}"),
        ("F_CPU", board.get("build.f_cpu")),
        ("MCU", "${MCU}"),
        ("FAMILY", "F_${FAMILY}"),
        # Add flash layout defines created in env.AddFlashLayout()
        *env["FLASH_DEFINES"].items(),
    ],
    CPPPATH=[
        "$BOARD_DIR",
    ],
    LINKFLAGS=[
        '"-Wl,-Map=' + join("$BUILD_DIR", "${PROGNAME}.map") + '"',
    ],
    LIBS=[
        "stdc++",
        "supc++",
    ],
)

# Build everything from the base core
queue.BuildLibraries()
