# Copyright (c) Kuba Szczodrzyński 2022-06-19.

from SCons.Script import DefaultEnvironment

env = DefaultEnvironment()
platform = env.PioPlatform()


def env_add_printf(
    env,
    version: str,
):
    package_dir = platform.get_package_dir(f"library-printf@{version}")

    env.AddLibrary(
        name=f"printf{version}",
        base_dir=package_dir,
        srcs=[
            "+<src/printf/printf.c>",
        ],
        includes=[
            "+<src>",
        ],
        options=dict(
            CFLAGS=["-Wno-maybe-uninitialized"],
            CPPDEFINES=[("PRINTF_INCLUDE_CONFIG_H", "1")],
            LINKFLAGS=[
                "-Wl,-wrap,printf",
                "-Wl,-wrap,sprintf",
                "-Wl,-wrap,vsprintf",
                "-Wl,-wrap,snprintf",
                "-Wl,-wrap,vsnprintf",
                "-Wl,-wrap,vprintf",
                "-Wl,-wrap,puts",
                "-Wl,-wrap,putchar",
            ],
        ),
    )


env.AddMethod(env_add_printf, "AddLibraryPrintf")
