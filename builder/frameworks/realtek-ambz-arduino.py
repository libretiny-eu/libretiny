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
    CXXFLAGS=[
        # borrowed from RtlDuino/development/rtl87xx/platform.txt
        "-std=c++11",
        "-mcpu=cortex-m4",
        "-mthumb",
        "-mfloat-abi=hard",
        "-mfpu=fpv4-sp-d16",
        "-g2",
        "-w",
        "-O2",
        "-MMD",
        "-fdata-sections",
        "-ffunction-sections",
        "-fmessage-length=0",
        "-fno-common",
        "-fno-exceptions",
        "-fno-rtti",
        "-fno-short-enums",
        "-fomit-frame-pointer",
        "-fsigned-char",
    ],
    CPPDEFINES=[
        ("ARDUINO", 10812),
        "ARDUINO_AMEBA",
        "ARDUINO_SDK",
        "ARDUINO_ARCH_AMBZ",
        # the SDK declares bool if not defined before
        # which conflicts with C++ built-in bool
        # so it's either -fpermissive or this:
        ("bool", "bool"),
    ],
    LINKFLAGS=[
        "--specs=nosys.specs",
        "-Wl,--as-needed",
        "-Wl,--build-id=none",
        "-Wl,--cref",
        "-Wl,--no-enum-size-warning",
        "-Wl,--no-undefined",
        "-Wl,--undefined=InfraStart",
        "-Wl,--warn-common",
    ],
)
# Arduino core uses __libc_init_array
env["LINKFLAGS"].remove("-nostartfiles")

# Build all libraries
env.BuildLibraries()
