# Copyright (c) Kuba Szczodrzyński 2022-05-04.

import fnmatch
from glob import glob
from os.path import isdir, join
from typing import List

from SCons.Script import DefaultEnvironment

env = DefaultEnvironment()
platform = env.PioPlatform()
board = env.BoardConfig()


def env_add_defaults(env, platform_name: str, sdk_name: str):
    vars = dict(
        SDK_DIR=platform.get_package_dir(sdk_name),
        LT_DIR=platform.get_dir(),
        # Root dirs
        BOARD_DIR=join("${LT_DIR}", "boards", "${VARIANT}"),
        ARDUINO_DIR=join("${LT_DIR}", "arduino", platform_name),
        PLATFORM_DIR=join("${LT_DIR}", "platform", platform_name),
        TOOLS_DIR=join("${LT_DIR}", "tools"),
        # Platform-specific dirs
        BIN_DIR=join("${PLATFORM_DIR}", "bin"),
        FIXUPS_DIR=join("${PLATFORM_DIR}", "fixups"),
        LD_DIR=join("${PLATFORM_DIR}", "ld"),
        OPENOCD_DIR=join("${PLATFORM_DIR}", "openocd"),
        # Board config variables
        MCU=board.get("build.mcu").upper(),
        FAMILY=board.get("build.family").upper(),
        VARIANT=board.get("build.variant"),
        LDSCRIPT_SDK=board.get("build.ldscript_sdk"),
        LDSCRIPT_ARDUINO=board.get("build.ldscript_arduino"),
    )
    env.Replace(**vars)
    for k, v in vars.items():
        if "DIR" in k:
            assert isdir(env.subst(v)), f"{env.subst(v)} is not a directory"
    env.Prepend(
        CPPPATH=[
            "$BOARD_DIR",
            "$FIXUPS_DIR",
            "$FIXUPS_DIR/inc",
        ],
        LIBPATH=[
            "$LD_DIR",
        ],
        CPPDEFINES=[
            ("LT_VERSION", platform.version),
            ("LT_BOARD", board.get("build.variant")),
        ],
    )


def env_add_library(
    env,
    name: str,
    base_dir: str,
    srcs: List[str],
    includes: List[str] = [],
):
    name = env.subst(name)
    # add base dir to all source globs
    sources = []
    for src in srcs:
        if src[1] != "<" or src[-1] != ">":
            raise ValueError(f"Not a source glob: {src}")

        if src[2] == "$":  # do not append base path
            expr = src[2:-1]
        else:
            expr = join(base_dir, src[2:-1])

        sources.append(src[0] + "<" + expr + ">")
    # queue library for further env clone and build
    env.Prepend(LIBQUEUE=[[join("$BUILD_DIR", name), base_dir, sources]])

    base_dir = env.subst(base_dir)  # expand base_dir for includes
    for dir in includes:
        if dir[1] != "<" or dir[-1] != ">":
            env.Append(CPPPATH=[dir])
            continue

        if dir[2] == "$":  # do not append base path
            expr = dir[2:-1]
        else:
            expr = join(base_dir, dir[2:-1])
        expr = env.subst(expr)

        if dir[0] == "-":
            for item in fnmatch.filter(env["CPPPATH"], expr):
                if item in env["CPPPATH"]:
                    env["CPPPATH"].remove(item)
        else:
            for item in glob(expr, recursive=True):
                if not isdir(item):
                    continue
                if dir[0] == "!":
                    env.Prepend(CPPPATH=[item])
                else:
                    env.Append(CPPPATH=[item])


def env_build_libraries(env, safe: bool = True):
    # add lib targets and clone safe envs
    if not "LIBQUEUE" in env:
        return
    queue = env["LIBQUEUE"]
    env["LIBQUEUE"] = []
    envsafe = env.Clone() if safe else env
    for lib in queue:
        target = envsafe.BuildLibrary(*lib)
        env.Prepend(LIBS=[target])


env.AddMethod(env_add_defaults, "AddDefaults")
env.AddMethod(env_add_library, "AddLibrary")
env.AddMethod(env_build_libraries, "BuildLibraries")
