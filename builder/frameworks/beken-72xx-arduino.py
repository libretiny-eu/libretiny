# Copyright (c) Kuba Szczodrzyński 2022-06-14.

from SCons.Script import DefaultEnvironment

env = DefaultEnvironment()

# SDK options
env.Replace(
    LIB_BDK_DRIVER_SKIP=[
        # using printf library wrappers instead
        "uart/printf.c",
    ]
)

env.SConscript("beken-72xx-sdk.py", exports="env")
env.SConscript("../arduino-common.py", exports="env")

# Flags
env.Append(
    CCFLAGS=[
        "-Wno-write-strings",
        "-Wno-char-subscripts",
        "-Wno-missing-braces",
    ],
    LINKFLAGS=[
        # stdio wrappers (port/printf/printf.c)
        "-Wl,-wrap,bk_printf",
    ],
)

# Build all libraries
env.BuildLibraries()
