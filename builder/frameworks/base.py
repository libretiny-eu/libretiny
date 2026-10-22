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

# Parse custom options
env.ParseCustomOptions(platform)
# Parse custom flash layout
env.ParseCustomFlashLayout(platform, board)
# Add flash layout C defines
env.AddFlashLayout(board)
# Export board manifest for ltchiptool
env.ExportBoardData(board)
# Print information about versions and custom options
env.PrintInfo(platform, board)

# TODO remove include path prepending ("!<...>")
# Move common core sources (env.AddCoreSources()) and Arduino libs
# below per-family sources (to maintain child families taking precedence)

# Global public flags
# Refer to https://gcc.gnu.org/onlinedocs/gcc/Option-Summary.html
env.Append(
    CCFLAGS=[
        # C Language Options
        "-fsigned-char",  # Let the type char be signed
        # Debugging Options
        "-g2",  # produce debugging information; the default level is 2
        # Optimization Options
        "-Os",  # optimize for size; enables all -O2 optimizations except those that often increase code size
        "-fdata-sections",  # place each function or data item into its own section
        "-ffunction-sections",  # place each function or data item into its own section
        "-fno-strict-aliasing",  # (don't) assume the strictest aliasing rules applicable
        "-fno-inline-functions",  # (don't) consider all functions for inlining
        "-fno-delete-null-pointer-checks",  # assume that programs can safely dereference null pointers
        # Preprocessor Options
        "-MMD",  # output a rule suitable for make describing the dependencies of the main source file
        # Code Generation Options
        "-fno-common",  # place uninitialized global variables in the BSS section of the object file
        "-fno-exceptions",  # disable exception handling
        # Developer Options
        "-fstack-usage",  # output stack usage information for the program, on a per-function basis
    ],
    CFLAGS=[
        "-std=gnu99",
    ],
    CXXFLAGS=[
        "-std=gnu++11",
        "-fno-rtti",  # disable generation of information about every class with virtual functions
    ],
)

# Include SDK builder scripts
# No environment options that follow later will be considered
found = False
for f in family.inheritance:
    try:
        env.Prepend(LIBPATH=[join("$CORES_DIR", f.name, "misc")])
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

# Sources - external libraries
queue.AddExternalLibrary("uf2ota")
queue.AddExternalLibrary("flashdb")
queue.AddExternalLibrary("printf")

# Find optimization level and add __OPTIMIZE_LEVEL__ macro
for flag in env["CCFLAGS"]:
    if not flag.startswith("-O"):
        continue
    env.Append(CPPDEFINES=[("__OPTIMIZE_LEVEL__", flag[2])])

# Non-SDK defines & linker options
queue.AppendPublic(
    CCFLAGS=[
        "-Wreturn-type",
        "-Wno-undef",
    ],
    CFLAGS=[
        "-Werror=implicit-function-declaration",
    ],
    CXXFLAGS=[
        "-Wno-literal-suffix",
        "-Wno-write-strings",
        "-Wno-psabi",  # parameter passing for argument of type ... changed in GCC 7.1
    ],
    CPPDEFINES=[
        ("LIBRETUYA", 1),
        ("LIBRETINY", 1),
        ("LT_VERSION", env["LT_VERSION"]),
        ("LT_BOARD", "${VARIANT}"),
        ("LT_VARIANT_H", r"\"${VARIANT}.h\""),
        ("F_CPU", board.get("build.f_cpu")),
        ("MCU", "${MCU}"),
        ("MCULC", "${MCULC}"),
        ("FAMILY", "F_${FAMILY_SHORT_NAME}"),
        # Add flash layout defines created in env.AddFlashLayout()
        *env["FLASH_DEFINES"].items(),
    ],
    CPPPATH=[
        "$VARIANTS_DIR",
    ],
    LINKFLAGS=[
        "-g2",
        "-Os",
        "-Wl,--as-needed",
        "-Wl,--build-id=none",
        "-Wl,--cref",
        "-Wl,--gc-sections",
        "-Wl,--no-enum-size-warning",
        "-Wl,--no-wchar-size-warning",
        "-Wl,--no-undefined",
        "-Wl,--warn-common",
        # malloc.c wrappers
        "-Wl,-wrap,malloc",
        "-Wl,-wrap,calloc",
        "-Wl,-wrap,zalloc",
        "-Wl,-wrap,realloc",
        "-Wl,-wrap,free",
        "-Wl,-wrap,_malloc_r",
        "-Wl,-wrap,_calloc_r",
        "-Wl,-wrap,_realloc_r",
        "-Wl,-wrap,_free_r",
        # linker map path
        '"-Wl,-Map=' + join("$BUILD_DIR", "${PROGNAME}.map") + '"',
    ],
    LIBS=[
        "stdc++",
        "supc++",
    ],
)

# Build everything from the base core
queue.BuildLibraries()
