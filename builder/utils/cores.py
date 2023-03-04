# Copyright (c) Kuba Szczodrzyński 2023-02-26.

from os.path import isdir, join

from ltchiptool import Family
from SCons.Script import DefaultEnvironment, Environment

env: Environment = DefaultEnvironment()


def env_add_family(env: Environment, family: Family) -> str:
    if family.short_name:
        env.Prepend(CPPDEFINES=[(f"LT_{family.short_name}", "1")])
    if family.code:
        env.Prepend(CPPDEFINES=[(f"LT_{family.code.upper()}", "1")])
    path = join("$CORES_DIR", family.name)
    if not isdir(env.subst(path)):
        return path
    env.Prepend(
        LIBPATH=[join(path, "misc")],
    )
    return path


def env_add_core_config(env: Environment, path: str) -> bool:
    if not isdir(env.subst(path)):
        return False
    env.Prepend(
        CPPPATH=[
            join(path, "compat"),
            join(path, "config"),
            join(path, "fixups"),
        ],
        LIBPATH=[
            join(path, "fixups"),
        ],
    )
    try:
        env.LoadDefines(join(path, "lt_defs.h"))
    except FileNotFoundError:
        pass
    return True


def env_add_core_sources(env: Environment, name: str, path: str) -> bool:
    if not isdir(env.subst(path)):
        return False
    env.AddLibrary(
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


def env_add_arduino_libraries(env: Environment, name: str, path: str) -> bool:
    if not isdir(env.subst(path)):
        return False
    env.AddLibrary(
        name=f"core_{name}_libraries",
        base_dir=path,
        srcs=[
            "+<**/*.c*>",
        ],
        includes=[
            "!<*/*>" if name.startswith("common") else "!<*>",
        ],
    )
    return True


env.AddMethod(env_add_family, "AddFamily")
env.AddMethod(env_add_core_config, "AddCoreConfig")
env.AddMethod(env_add_core_sources, "AddCoreSources")
env.AddMethod(env_add_arduino_libraries, "AddArduinoLibraries")
