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
env.ConfigureEnvironment(platform, board)
family: Family = env["FAMILY_OBJ"]

# Flash layout defines
env.AddFlashLayout(board)

# Configure each family first (add CPP defines, prepend fixups' paths)
for f in family.inheritance:
    path = env.AddFamily(f)
    env.AddCoreConfig(name=f.code, path=join(path, "base"))
    if "ARDUINO" in env:
        env.AddCoreConfig(name=f"{f.code}_arduino", path=join(path, "arduino", "src"))

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

# Add common sources among all families
env.AddCoreSources(
    name="common",
    path=join("$COMMON_DIR", "base"),
)
# Add sources for this family and each parent
for f in family.inheritance:
    path = join("$CORES_DIR", f.name, "base")
    env.AddCoreSources(name=f.code, path=path)

# Sources - external libraries
env.AddExternalLibrary("ltchiptool")  # uf2ota source code
env.AddExternalLibrary("flashdb")
env.AddExternalLibrary("printf")

# Flags & linker options
env.Append(
    CPPDEFINES=[
        ("LIBRETUYA", 1),
        ("LT_VERSION", env.ReadLTVersion(platform.get_dir(), platform.version)),
        ("LT_BOARD", "${VARIANT}"),
        ("F_CPU", board.get("build.f_cpu")),
        ("MCU", "${MCU}"),
        ("FAMILY", "F_${FAMILY}"),
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
env.BuildLibraries()
