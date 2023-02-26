# Copyright (c) Kuba Szczodrzyński 2023-02-26.

from dataclasses import dataclass
from typing import Dict, List, Optional, Union

from platformio.package.meta import PackageItem
from platformio.platform.base import PlatformBase
from SCons.Script import DefaultEnvironment, Environment

env: Environment = DefaultEnvironment()
platform: PlatformBase = env.PioPlatform()


@dataclass
class Library:
    package: str
    sources: List[str]
    includes: List[str]
    flags: List[str] = None
    linkflags: List[str] = None
    defines: Dict[str, Union[str, int]] = None

    def __post_init__(self):
        self.flags = self.flags or []
        self.linkflags = self.linkflags or []
        self.defines = self.defines or {}


def env_add_external_library(
    env: Environment,
    name: str,
    port: Optional[str] = None,
):
    if port:
        name += f"-{port}"
    external_libs = env["EXTERNAL_LIBS"]
    lib = Library(**external_libs[name])
    version = platform.versions.get(lib.package, None)

    package: PackageItem = platform.pm.get_package(
        platform.get_package_spec(lib.package, version)
    )
    if not package:
        raise ValueError(
            f"Version '{version}' of library '{name}' ({lib.package}) is not installed"
        )

    env.AddLibrary(
        name=name.replace("-", "_"),
        base_dir=package.path,
        srcs=lib.sources,
        includes=lib.includes,
        options=dict(
            CFLAGS=lib.flags,
            CPPDEFINES=[(k, v) for k, v in lib.defines.items()],
            LINKFLAGS=lib.linkflags,
        ),
    )


env.AddMethod(env_add_external_library, "AddExternalLibrary")
