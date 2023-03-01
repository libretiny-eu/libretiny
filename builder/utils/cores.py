# Copyright (c) Kuba Szczodrzyński 2023-02-26.

from os.path import isdir, join

from ltchiptool import Family
from SCons.Script import DefaultEnvironment, Environment

env: Environment = DefaultEnvironment()


def env_configure_family(env: Environment):
    env.Prepend(
        CPPPATH=[
            join("$COMMON_DIR", "base", "fixups"),
            join("$COMMON_DIR", "base", "config"),
            join("$COMMON_DIR", "base", "compat"),
        ],
    )

    family: Family = env["FAMILY_OBJ"]
    for f in family.inheritance:
        path = join("$CORES_DIR", f.name, "base")
        if not isdir(env.subst(path)):
            continue
        env.Prepend(
            CPPPATH=[
                join(path, "fixups"),
                join(path, "config"),
                join(path, "compat"),
            ],
            LIBPATH=[
                join(path, "fixups"),
            ],
        )


def env_add_core_sources(env: Environment, queue, name: str, path: str) -> bool:
    if not isdir(env.subst(path)):
        return False
    try:
        env.LoadDefines(join(path, "lt_defs.h"))
    except FileNotFoundError:
        pass
    queue.AddLibrary(
        name=f"core_{name}",
        base_dir=path,
        srcs=[
            "+<*.c*>",
            "+<common/*.c*>",
            "+<compat/*.c*>",
            "+<fixups/*.c*>",
            "+<port/*.c*>",
            "+<posix/*.c>",
            "+<wraps/*.c>",
        ],
        includes=[
            # prepend the paths before SDK directories
            "!<.>",
            "!<compat>",
            "!<config>",
            "!<fixups>",
            "!<port>",
        ],
    )
    return True


def env_add_arduino_libraries(env: Environment, queue, name: str, path: str) -> bool:
    if not isdir(env.subst(path)):
        return False
    queue.AddLibrary(
        name=f"core_{name}_libraries",
        base_dir=path,
        srcs=[
            "+<**/*.c*>",
        ],
        includes=[
            "!<*/.>",
            "!<*/*>",
        ]
        if name.startswith("common")
        else [
            "!<.>",
            "!<*>",
        ],
    )
    return True


env.AddMethod(env_configure_family, "ConfigureFamily")
env.AddMethod(env_add_core_sources, "AddCoreSources")
env.AddMethod(env_add_arduino_libraries, "AddArduinoLibraries")
