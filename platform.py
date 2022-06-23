# Copyright (c) Kuba Szczodrzyński 2022-04-20.

import json
from os.path import dirname, join
from typing import Dict

from platformio import util
from platformio.debug.config.base import DebugConfigBase
from platformio.debug.exception import DebugInvalidOptionsError
from platformio.managers.platform import PlatformBase
from platformio.package.exception import MissingPackageManifestError
from platformio.package.manager.base import BasePackageManager
from platformio.package.meta import PackageItem, PackageSpec
from platformio.platform.board import PlatformBoardConfig

libretuya_packages = None
manifest_default = {"version": "0.0.0", "description": "", "keywords": []}


def load_manifest(self, src):
    try:
        return BasePackageManager._load_manifest(self, src)
    except MissingPackageManifestError:
        # ignore all exceptions
        pass
    # get the installation temporary path
    path = src.path if isinstance(src, PackageItem) else src
    # raise the exception if this package is not from libretuya
    if (
        not hasattr(self, "spec_map")
        or path not in self.spec_map
        or not libretuya_packages
    ):
        raise MissingPackageManifestError(", ".join(self.manifest_names))
    # get the saved spec
    spec: PackageSpec = self.spec_map[path]
    # read package data from platform.json
    manifest: dict = libretuya_packages[spec.name]
    # find additional manifest info
    manifest = manifest.get("manifest", manifest_default)
    # extract tag version
    url = getattr(spec, "url", None) or getattr(spec, "uri", None) or ""
    if "#" in url:
        manifest["version"] = url.rpartition("#")[2].lstrip("v")
    # put info from spec
    manifest.update(
        {
            "name": spec.name,
            "repository": {
                "type": "git",
                "url": url,
            },
        }
    )
    # save in cache
    cache_key = "load_manifest-%s" % path
    self.memcache_set(cache_key, manifest)
    # result = ManifestParserFactory.new(json.dumps(manifest), ManifestFileType.PACKAGE_JSON).as_dict()
    with open(join(path, self.manifest_names[0]), "w") as f:
        json.dump(manifest, f)
    return manifest


def find_pkg_root(self, path: str, spec: PackageSpec):
    try:
        return BasePackageManager._find_pkg_root(self, path, spec)
    except MissingPackageManifestError as e:
        # raise the exception if this package is not from libretuya
        if not libretuya_packages or spec.name not in libretuya_packages:
            raise e
    # save the spec for later
    if not hasattr(self, "spec_map"):
        self.spec_map = {}
    self.spec_map[path] = spec
    return path


class LibretuyaPlatform(PlatformBase):
    boards_base: Dict[str, dict] = {}
    custom_opts: Dict[str, object] = {}

    def configure_default_packages(self, options, targets):
        framework = options.get("pioframework")[0]
        # patch find_pkg root to ignore missing manifests and save PackageSpec
        if not hasattr(BasePackageManager, "_find_pkg_root"):
            BasePackageManager._find_pkg_root = BasePackageManager.find_pkg_root
            BasePackageManager.find_pkg_root = find_pkg_root
        # patch load_manifest to generate manifests from PackageSpec
        if not hasattr(BasePackageManager, "_load_manifest"):
            BasePackageManager._load_manifest = BasePackageManager.load_manifest
            BasePackageManager.load_manifest = load_manifest

        # allow using "arduino" as framework
        if framework == "arduino":
            board = self.get_boards(options.get("board"))
            frameworks = board.get("frameworks")
            framework = next(fw for fw in frameworks if framework in fw)
            options.get("pioframework")[0] = framework

        framework_obj = self.frameworks[framework]
        package_obj = self.packages[framework_obj["package"]]

        # set specific compiler versions
        if "toolchain" in package_obj:
            (toolchain, version) = package_obj["toolchain"].split("@")
            self.packages[f"toolchain-{toolchain}"]["version"] = version

        # make ArduinoCore-API required
        if "arduino" in framework:
            self.packages["framework-arduino-api"]["optional"] = False

        # require bk7231tools
        if "beken-72xx" in framework:
            self.packages["tool-bk7231tools"]["optional"] = False

        # mark framework SDK as required
        package_obj["optional"] = False

        # gather library dependencies
        libraries = package_obj["libraries"] if "libraries" in package_obj else {}
        for name, package in self.packages.items():
            if "optional" in package and package["optional"]:
                continue
            if "libraries" not in package:
                continue
            libraries.update(package["libraries"])

        # use appropriate vendor library versions
        packages_new = {}
        for name, package in self.packages.items():
            if not name.startswith("library-"):
                continue
            name = name[8:]  # strip "library-"
            if name not in libraries:
                continue
            lib_version = libraries[name][-1]  # get latest version tag
            package = dict(**package)  # clone the base package
            package["version"] = (
                package["base_url"] + "#" + lib_version
            )  # use the specific version
            package["optional"] = False  # make it required
            lib_version = lib_version.lstrip("v")  # strip "v" in target name
            name = f"library-{name}@{lib_version}"
            packages_new[name] = package  # put the package under a new name
        self.packages.update(packages_new)

        # save platform packages for later
        global libretuya_packages
        libretuya_packages = self.packages

        # save custom options from env
        self.custom_opts = {}
        for key, value in options.items():
            if not key.startswith("custom_"):
                continue
            self.custom_opts[key[7:]] = value

        return super().configure_default_packages(options, targets)

    def custom(self, key: str) -> object:
        return self.custom_opts.get(key, None)

    def get_boards(self, id_=None):
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

        if "_base" in board:
            base = board.get("_base")
            if not isinstance(base, list):
                base = [base]

            result = None
            for base_name in base:
                if base_name not in self.boards_base:
                    file = join(
                        dirname(__file__), "boards", "_base", f"{base_name}.json"
                    )
                    with open(file, encoding="utf-8") as f:
                        self.boards_base[base_name] = json.load(f)

                if not result:
                    result = self.boards_base[base_name]
                else:
                    util.merge_dicts(result, self.boards_base[base_name])
            util.merge_dicts(result, board._manifest)
            board._manifest = result

        # add "arduino" framework
        has_arduino = any("arduino" in fw for fw in board.manifest["frameworks"])
        if has_arduino:
            board.manifest["frameworks"].append("arduino")

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
                f"[LibreTuya] Debug tool {debug_tool} is not supported by board {board}."
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
