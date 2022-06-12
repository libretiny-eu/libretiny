# Copyright (c) Kuba Szczodrzyński 2022-05-04.

import fnmatch
from glob import glob
from os.path import isdir, join
from typing import List

from SCons.Script import DefaultEnvironment

env = DefaultEnvironment()


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

    # allow removing sources from parent builders
    key = f"LIB_{name.upper()}_SKIP"
    if key in env:
        for expr in env[key]:
            sources.append("-<" + expr + ">")

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


env.AddMethod(env_add_library, "AddLibrary")
env.AddMethod(env_build_libraries, "BuildLibraries")
