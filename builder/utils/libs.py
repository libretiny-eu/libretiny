# Copyright (c) Kuba Szczodrzyński 2022-05-04.

import fnmatch
from glob import glob
from os.path import isdir, join
from typing import Dict, Generator, List, Tuple

from SCons.Script import DefaultEnvironment, Environment

env: Environment = DefaultEnvironment()


def add_base_dir(
    env: Environment,
    base_dir: str,
    expressions: List[str],
    subst: bool = False,
):
    out = []
    for expr in expressions:
        if expr == False:
            # support '[cond] and [path]' logical expressions
            continue
        if expr[1] != "<" or expr[-1] != ">":
            raise ValueError(f"Not a valid glob: {expr}")
        if expr[2] == "$":
            # do not append base path
            path = expr[2:-1]
        else:
            path = join(base_dir, expr[2:-1])
        if subst:
            path = env.subst(path)
        out.append(expr[0] + "<" + path + ">")
    return out


def iter_expressions(expressions: List[str]) -> Generator[Tuple[str, str], None, None]:
    for expr in expressions:
        if expr[1] != "<" or expr[-1] != ">":
            yield ("+", expr)
            continue
        yield (expr[0], expr[2:-1])


def env_add_library(
    env: Environment,
    name: str,
    base_dir: str,
    srcs: List[str],
    includes: List[str] = [],
    options: Dict[str, List[str]] = {},
):
    name = env.subst(name)
    # add base dir to all globs
    srcs = add_base_dir(env, base_dir, srcs)
    includes = add_base_dir(env, base_dir, includes, subst=True)

    # allow removing sources from parent builders
    key = f"LIB_{name.upper()}_SKIP"
    if key in env:
        for expr in env[key]:
            srcs.append("-<" + expr + ">")

    # queue library for further env clone and build
    if srcs:
        env.Prepend(
            LIBQUEUE=[
                (join("$BUILD_DIR", name), base_dir, srcs, options),
            ]
        )

    # search all include paths
    for dir, expr in iter_expressions(includes):
        if dir == "-":
            for item in fnmatch.filter(env["CPPPATH"], expr):
                if item in env["CPPPATH"]:
                    env["CPPPATH"].remove(item)
        else:
            for item in glob(expr, recursive=True):
                if not isdir(item):
                    continue
                if dir == "!":
                    env.Prepend(CPPPATH=[item])
                else:
                    env.Append(CPPPATH=[item])


def env_build_libraries(env: Environment, safe: bool = True):
    # add lib targets and clone safe envs
    if not "LIBQUEUE" in env:
        return
    queue = env["LIBQUEUE"]
    env["LIBQUEUE"] = []
    for lib in queue:
        envsafe = env.Clone() if safe else env
        # get env options to add/remove
        options: Dict[str, List[str]] = lib[3]
        # change only safe env options
        for key, values in options.items():
            for dir, value in iter_expressions(values):
                if dir == "+":
                    envsafe.Append(**{key: [value]})
                elif dir == "!":
                    envsafe.Prepend(**{key: [value]})
                elif dir == "-":
                    if value not in envsafe[key]:
                        raise ValueError(f"Invalid option; {value} is not in {key}")
                    envsafe[key].remove(value)
        # build library with (name, base_dir, sources) options
        target = envsafe.BuildLibrary(*lib[0:3])
        env.Prepend(LIBS=[target])


env.AddMethod(env_add_library, "AddLibrary")
env.AddMethod(env_build_libraries, "BuildLibraries")
