# Copyright (c) Kuba Szczodrzyński 2022-04-22.

from SCons.Script import DefaultEnvironment

env = DefaultEnvironment()

# SDK options
env.Replace(AMBZ_NO_POLARSSL=True)
env.Replace(
    LIB_AMBZ_SDK_SKIP=[
        "component/common/api/wifi/wifi_ind.c",
    ]
)

env.SConscript("realtek-ambz-sdk.py", exports="env")
env.SConscript("../arduino-common.py", exports="env")

# Flags
env.Append(
    CPPDEFINES=[
        "ARDUINO_AMEBA",
        "ARDUINO_ARCH_AMBZ",
        # the SDK declares bool if not defined before
        # which conflicts with C++ built-in bool
        # so it's either -fpermissive or this:
        ("bool", "bool"),
        # implemented features
        ("LT_ARD_HAS_WIFI", "1"),
        ("LT_ARD_HAS_MD5", "1"),
    ],
    LINKFLAGS=[
        "-Wl,--undefined=InfraStart",
    ],
)

# Build all libraries
env.BuildLibraries()
