# Copyright (c) Kuba Szczodrzyński 2022-04-20.

import json
import os
import platform
import site
import sys
from os.path import dirname
from pathlib import Path
from typing import Dict, List

import click
from platformio.debug.config.base import DebugConfigBase
from platformio.debug.exception import DebugInvalidOptionsError
from platformio.package.meta import PackageItem
from platformio.platform.base import PlatformBase
from platformio.platform.board import PlatformBoardConfig

site.addsitedir(Path(__file__).absolute().parent.joinpath("tools"))


def get_os_specifiers():
    system = platform.system().lower()
    arch = platform.machine().lower()
    if not arch:  # issue #4353
        arch = "x86"
        bits = platform.architecture()[0]
    if "aarch64" in arch:
        arch = "arm"
        bits = 64
    elif "arm" in arch:
        arch = "arm"
        bits = 32
    elif "64" not in arch:
        arch = "x86"
        bits = 32
    else:
        arch = "x86"
        bits = 64
    return [
        f"{system}_{arch}_{bits}",  # linux_x86_64
        f"{system}_{bits}",  # linux_64
        system,  # windows
        arch,  # arm
        "any",
    ]


class LibretinyPlatform(PlatformBase):
    custom_opts: Dict[str, object] = None
    versions: Dict[str, str] = None

    def __init__(self, manifest_path):
        super().__init__(manifest_path)
        self.custom_opts = {}
        self.versions = {}
        self.verbose = (
            "-v" in sys.argv
            or "--verbose" in sys.argv
            or "PIOVERBOSE=1" in sys.argv
            or os.environ.get("PIOVERBOSE", "0") == "1"
        )

    def print(self, *args, **kwargs):
        if not self.verbose:
            return
        print(f"platform.py({os.getpid()}):", *args, **kwargs)

    def custom(self, key: str) -> object:
        return self.custom_opts.get(key, None)

    def get_package_spec(self, name, version=None):
        # make PlatformIO detach existing package versions instead of overwriting
        # TODO this is an ugly hack, it moves old packages to dirs like "library-lwip@src-21d717f2feaca73533f129ce05c9f4d4"
        # it should be fixed properly at some point. Maybe ask PIO to allow controlling that somehow?
        spec = super().get_package_spec(name, version)
        spec._name_is_custom = False
        return spec

    def configure_default_packages(self, options: dict, targets: List[str]):
        from libretiny import RecursiveDict

        self.print(f"configure_default_packages(targets={targets})")

        pioframework = options.get("pioframework") or ["base"]
        if not pioframework:
            return
        framework: str = pioframework[0]

        # save custom options from env
        self.custom_opts = RecursiveDict()
        for key, value in options.items():
            if not key.startswith("custom_"):
                continue
            key = key[7:]
            parent, sep, child = key.partition(".")
            if parent == "options":
                # allow only one level of nesting ('child' may be a header name, which contains a dot)
                child = child.replace(".", "#")
            self.custom_opts[parent + sep + child] = value
        self.print("Custom options:", json.dumps(self.custom_opts, indent="\t"))

        # update framework names to their new values since v1.0.0
        if framework.endswith("-sdk"):
            click.secho(
                f"Framework '{framework}' is now named 'base'. "
                "Update your platformio.ini to use the new name, then try again.",
                fg="red",
            )
            exit(1)
        if framework.endswith("-arduino"):
            click.secho(
                f"Framework '{framework}' is now named 'arduino'. "
                "Update your platformio.ini to use the new name, then try again.",
                fg="red",
            )
            exit(1)

        # make ArduinoCore-API required
        if framework == "arduino":
            self.packages["framework-arduino-api"]["optional"] = False

        # get framework SDK package
        board = self.get_boards(options.get("board"))
        package = board.get("package")
        package_obj = self.packages.get(package, {})
        # mark framework SDK as required
        package_obj["optional"] = False

        # get user-chosen versions of libraries/toolchains
        versions: RecursiveDict = self.custom("versions") or {}

        # set specific compiler versions
        if "toolchains" in package_obj:
            toolchains = package_obj["toolchains"]
            toolchain_version = None
            specifiers = get_os_specifiers()
            for spec in specifiers:
                toolchain_version = toolchains.get(spec)
                if toolchain_version:
                    break
            if not toolchain_version:
                raise RuntimeError(
                    f"Toolchain not found for the current platform: {specifiers}"
                )
            toolchain, version = toolchain_version.split("@")
            self.packages[f"toolchain-{toolchain}"]["version"] = version

        # gather library dependencies
        pkg_versions = {}
        for package in self.packages.values():
            if "optional" in package and package["optional"]:
                continue
            if "libraries" not in package:
                continue
            for name, lib_versions in package["libraries"].items():
                package = f"library-{name}"
                if isinstance(lib_versions, str):
                    # single version specified as string
                    if name in versions:
                        pkg_versions[package] = versions[name]
                    else:
                        pkg_versions[package] = lib_versions
                    continue
                # mapping of versions to repo branches
                if name in versions and versions[name] in lib_versions:
                    pkg_versions[package] = lib_versions[versions[name]]
                    continue
                if "default" in lib_versions:
                    pkg_versions[package] = lib_versions["default"]

        # gather custom (user-set) versions of other libraries
        for name, version in versions.items():
            if name == "toolchain":
                continue
            name = name.replace("_", "-")
            version = version.lstrip("v")
            # find the package by "library-xxx", "framework-xxx" or "tool-xxx"
            package = f"library-{name}"
            if package not in self.packages:
                package = f"framework-{name}"
            if package not in self.packages:
                package = f"tool-{name}"
            if package not in self.packages:
                click.secho(
                    f"Library '{name}' couldn't be found. "
                    f"Remove 'custom_versions.{name}' from platformio.ini and try again.",
                    fg="red",
                )
                exit(1)
            if package in pkg_versions:
                # skip already added libs
                continue
            pkg_versions[package] = version

        # enable packages required for framework libraries
        for name, version in pkg_versions.items():
            if name not in self.packages:
                raise ValueError(f"Library '{name}' doesn't exist")
            package = self.packages[name]
            if "base_url" not in package:
                if "#" not in package.get("version", ""):
                    click.secho(
                        f"Property 'base_url' is missing for '{name}'. "
                        "The version of this package can't be changed by the user.",
                        fg="red",
                    )
                    exit(1)
                package["base_url"] = package["version"].partition("#")[0]
            if package.get("version_prefix", False):
                version = "v" + version
            package["optional"] = False
            package["version"] = package["base_url"] + "#" + version

        # store version numbers of all used packages
        for package in self.get_installed_packages(with_optional=False):
            package: PackageItem
            version = package.metadata.version
            version = str(version).partition("sha.")[0]
            version = version.strip("+.")
            version = version.rpartition("+")[2]
            self.versions[package.metadata.name] = version

        return super().configure_default_packages(options, targets)

    def get_boards(self, id_=None):
        self.print(f"get_boards(id_={id_})")
        result = PlatformBase.get_boards(self, id_)
        if not result:
            return result
        if id_:
            return self.update_board(result)
        else:
            for key, value in result.items():
                result[key] = self.update_board(value)
        return result

    def update_board(self, board: PlatformBoardConfig):
        from libretiny import Board, Family, merge_dicts

        if "_base" in board:
            board._manifest = Board.get_data(board._manifest)
            board._manifest.pop("_base")

        if self.custom("board"):
            with open(self.custom("board"), "r") as f:
                custom_board = json.load(f)
            board._manifest = merge_dicts(board._manifest, custom_board)

        family = board.get("build.family")
        family = Family.get(short_name=family)
        # add "frameworks" key with the default "base"
        board.manifest["frameworks"] = ["base"]
        # add "arduino" framework if supported
        if family.has_arduino_core:
            board.manifest["frameworks"].append("arduino")
        # add SDK package name
        board.manifest["package"] = family.target_package

        # inspired by platform-ststm32/platform.py
        debug = board.manifest.get("debug", {})
        if not debug:
            return board
        protocols = debug.get("protocols", [])
        if "tools" not in debug:
            debug["tools"] = {}
        if "custom" not in debug["tools"]:
            debug["tools"]["custom"] = {}
        init = debug.get("gdb_init", [])
        if "set mem inaccessible-by-default off" not in init:
            init += ["set mem inaccessible-by-default off"]

        for link in protocols:
            if link == "openocd":
                args = ["-s", "$PACKAGE_DIR/scripts"]
                if debug.get("openocd_config"):
                    args.extend(
                        [
                            "-f",
                            "$LTPATH/platform/$LTFAMILY/openocd/%s"
                            % debug.get("openocd_config"),
                        ]
                    )
                debug["tools"][link] = {
                    "server": {
                        "package": "tool-openocd",
                        "executable": "bin/openocd",
                        "arguments": args,
                    },
                    "extra_cmds": init,
                }
            else:
                continue
            debug["tools"][link]["default"] = link == debug.get("protocol", "")

        debug["tools"]["custom"]["extra_cmds"] = init

        board.manifest["debug"] = debug
        return board

    def configure_debug_session(self, debug_config: DebugConfigBase):
        opts = debug_config.env_options
        server = debug_config.server
        lt_path = dirname(__file__)
        lt_family = opts["framework"][0].rpartition("-")[0]
        if not server:
            debug_tool = opts.get("debug_tool", "custom")
            board = opts.get("board", "<unknown>")
            if debug_tool == "custom":
                return
            exc = DebugInvalidOptionsError(
                f"[LibreTiny] Debug tool {debug_tool} is not supported by board {board}."
            )
            exc.MESSAGE = ""
            raise exc
        if "arguments" in server:
            # allow setting interface via platformio.ini
            if opts.get("openocd_interface"):
                server["arguments"] = [
                    "-f",
                    "interface/%s.cfg" % opts.get("openocd_interface"),
                ] + server["arguments"]
            # replace $LTFAMILY with actual name
            server["arguments"] = [
                arg.replace("$LTFAMILY", lt_family).replace("$LTPATH", lt_path)
                for arg in server["arguments"]
            ]
