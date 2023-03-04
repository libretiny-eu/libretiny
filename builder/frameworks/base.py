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

# Environment variables, include paths, etc.
family: Family = env.ConfigureEnvironment(platform, board)
# Flash layout defines
env.AddFlashLayout(board)

# Flags & linker options
env.Append(
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

# Build a core list to add sources, flags, etc.
cores = {
    "common": "$COMMON_DIR",
}
# Configure each family first (add CPP defines)
for f in family.inheritance:
    cores[f.code] = env.AddFamily(f)

# Add fixups & config for each core
for name, path in cores.items():
    env.AddCoreConfig(path=join(path, "base"))
    if "ARDUINO" in env:
        env.AddCoreConfig(path=join(path, "arduino", "src"))

# Include SDK builder scripts
# The script will call BuildLibraries(safe=True) to secure the include paths
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

# Add sources & include paths for each core
for name, path in cores.items():
    env.AddCoreSources(name=name, path=join(path, "base"))

# Sources - external libraries
env.AddExternalLibrary("ltchiptool")  # uf2ota source code
env.AddExternalLibrary("flashdb")
env.AddExternalLibrary("printf")

# Build everything from the base core
env.BuildLibraries()
