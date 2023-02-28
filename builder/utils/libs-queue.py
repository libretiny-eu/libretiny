# Copyright (c) Kuba Szczodrzyński 2023-02-28.

import fnmatch
from dataclasses import InitVar, dataclass, field
from glob import glob
from os.path import isdir, join
from typing import Dict, Generator, List, Optional, Tuple

from ltchiptool.util.dict import merge_dicts
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
        if expr[1:2] != "<" or expr[-1:] != ">":
            yield ("+", expr)
            continue
        yield (expr[0], expr[2:-1])


def apply_options(env: Environment, options: Dict[str, List[str]]):
    non_expr_keys = ["CPPDEFINES"]
    for key, values in options.items():
        if not values:
            continue
        if key in non_expr_keys or not isinstance(values[0], str):
            env.Append(**{key: values})
            continue
        for dir, value in iter_expressions(values):
            if dir == "+":
                env.Append(**{key: [value]})
            elif dir == "!":
                env.Prepend(**{key: [value]})
            elif dir == "-":
                if value not in env[key]:
                    raise ValueError(f"Invalid option; {value} is not in {key}")
                env[key].remove(value)


@dataclass
class Library:
    env: InitVar[Environment]
    name: str
    base_dir: str
    srcs: List[str]
    includes: List[str] = field(default_factory=lambda: [])
    options: Dict[str, List[str]] = field(default_factory=lambda: {})

    def __post_init__(self, env: Environment):
        # add base dir to all globs
        self.srcs = add_base_dir(env, self.base_dir, self.srcs)
        self.includes = add_base_dir(env, self.base_dir, self.includes, subst=True)


class LibraryQueue:
    env: Environment
    name: str
    queue: List[Library]
    includes: List[str]
    options_public: dict
    options_private: dict
    prepend_includes: bool
    built: bool = False

    def __init__(
        self,
        env: Environment,
        name: str,
        prepend_includes: bool = False,
    ) -> None:
        self.env = env
        self.name = name
        self.queue = []
        self.includes = []
        self.options_public = {}
        self.options_private = {}
        self.prepend_includes = prepend_includes

    def AddLibrary(self, **kwargs):
        lib = Library(env=self.env, **kwargs)
        # search all include paths
        for dir, expr in iter_expressions(lib.includes):
            if dir == "-":
                for item in fnmatch.filter(self.includes, expr):
                    if item in self.includes:
                        self.includes.remove(item)
                continue
            for item in glob(expr, recursive=True):
                if not isdir(item):
                    continue
                if dir == "!":
                    self.includes.insert(0, item)
                else:
                    self.includes.append(item)
        self.queue.append(lib)

    def AddExternalLibrary(self, name: str, port: Optional[str] = None):
        return self.env.AddExternalLibrary(self, name, port)

    def AppendPublic(self, **kwargs):
        if "CPPPATH" in kwargs:
            self.includes += kwargs["CPPPATH"]
            kwargs.pop("CPPPATH")
        self.options_public = merge_dicts(self.options_public, kwargs)

    def AppendPrivate(self, **kwargs):
        public_only = ["CPPPATH", "LIBPATH", "LIBS", "LINKFLAGS"]
        if any(key in public_only for key in kwargs.keys()):
            raise ValueError("Cannot set these as private options")
        self.options_private = merge_dicts(self.options_private, kwargs)

    def Print(self):
        def print_list(items):
            print(
                "\n".join(
                    f"{i+1: 4d}. {self.env.subst(str(item))}"
                    for i, item in enumerate(items)
                )
            )

        print()
        print(f"Library Queue - {self.name}")
        print("Environment paths:")
        print_list(self.env["CPPPATH"])
        print(
            "Include paths (%s):" % ("prepend" if self.prepend_includes else "append")
        )
        print_list(self.includes)
        print("Environment options:")
        opts = ["CFLAGS", "CCFLAGS", "CXXFLAGS", "CPPDEFINES", "ASFLAGS"]
        opts = {k: v for k, v in self.env.items() if k in opts}
        print_list(opts.items())
        print("Options - public:")
        print_list(self.options_public.items())
        print("Options - private:")
        print_list(self.options_private.items())
        print("Libraries:")
        print_list(self.queue)

    def BuildLibraries(self):
        if self.built:
            raise RuntimeError("Cannot build a library queue twice")
        self.Print()

        # add public options to the environment
        apply_options(self.env, self.options_public)
        # treat all include paths as public
        if self.prepend_includes:
            self.env.Prepend(CPPPATH=self.includes)
        else:
            self.env.Append(CPPPATH=self.includes)

        # clone the environment for the whole library queue
        queue_env = self.env.Clone()
        # add private options to the cloned environment
        apply_options(queue_env, self.options_private)

        for lib in self.queue:
            if lib.options:
                # clone the environment separately for each library
                lib_env = queue_env.Clone()
                # add library-scoped options
                apply_options(lib_env, lib.options)
            else:
                # no library-scoped options, just use the base env
                lib_env = queue_env
            # build library with (name, base_dir, sources) options
            target = lib_env.BuildLibrary(
                join("$BUILD_DIR", lib.name), lib.base_dir, lib.srcs
            )
            self.env.Prepend(LIBS=[target])

        self.built = True


def env_add_library_queue(
    env: Environment,
    name: str,
    prepend_includes: bool = False,
) -> LibraryQueue:
    return LibraryQueue(env, name, prepend_includes)


env.AddMethod(env_add_library_queue, "AddLibraryQueue")
