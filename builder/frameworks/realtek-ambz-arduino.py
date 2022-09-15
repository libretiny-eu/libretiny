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
        # LibreTuya configuration
        ("LT_ARD_HAS_WIFI", "1"),
        ("LT_ARD_HAS_MD5", "1"),
        ("LT_ARD_HAS_SOFTSERIAL", "1"),
        # not broken anymore with printf() library
        ("LT_PRINTF_BROKEN", "0"),
    ],
    LINKFLAGS=[
        "-Wl,--undefined=InfraStart",
        # stdio wrappers (port/printf/printf.c)
        "-Wl,-wrap,rtl_printf",
        "-Wl,-wrap,rtl_sprintf",
        "-Wl,-wrap,rtl_snprintf",
        "-Wl,-wrap,rtl_vsnprintf",
        "-Wl,-wrap,rtl_vsnprintf_r",
        "-Wl,-wrap,rtl_vprintf",
        "-Wl,-wrap,rtl_vfprintf",
        "-Wl,-wrap,DiagPrintf",
        "-Wl,-wrap,DiagSPrintf",
        "-Wl,-wrap,DiagSnPrintf",
        "-Wl,-wrap,prvDiagPrintf",
        "-Wl,-wrap,prvDiagSPrintf",
        "-Wl,-wrap,VSprintf",
        "-Wl,-wrap,LOG_PRINTF",
        "-Wl,-wrap,__rtl_vfprintf_r_v1_00",
    ],
)

# Build all libraries
env.BuildLibraries()
