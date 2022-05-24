# Copyright (c) Kuba Szczodrzyński 2022-05-24.

from SCons.Script import DefaultEnvironment

env = DefaultEnvironment()
platform = env.PioPlatform()


def env_add_flashdb(
    env,
    version: str,
):
    package_dir = platform.get_package_dir(f"library-flashdb@{version}")

    env.AddLibrary(
        name=f"flashdb{version}",
        base_dir=package_dir,
        srcs=[
            "+<src/*.c>",
            "+<port/fal/src/*.c>",
        ],
        includes=[
            "+<inc>",
            "+<port/fal/inc>",
        ],
    )


env.AddMethod(env_add_flashdb, "AddLibraryFlashDB")
