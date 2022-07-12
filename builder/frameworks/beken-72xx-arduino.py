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
        "-Wno-attributes",
    ],
    CPPDEFINES=[
        # LibreTuya configuration
        ("LT_ARD_HAS_WIFI", "1"),
        ("LT_ARD_HAS_MD5", "1"),
        # macros
        # mbedtls_net_set_nonblock is commented out in tls_net.c
        ("mbedtls_net_set_nonblock", "net_set_nonblock"),
    ],
    LINKFLAGS=[
        # stdio wrappers (port/printf/printf.c)
        "-Wl,-wrap,bk_printf",
    ],
)

# Build all libraries
env.BuildLibraries()
