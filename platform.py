from platformio.managers.platform import PlatformBase
from platformio.package.exception import MissingPackageManifestError
from platformio.package.manager.base import BasePackageManager
from platformio.package.meta import PackageItem, PackageSpec

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
    # put info from spec
    manifest.update(
        {
            "name": spec.name,
            "repository": {
                "type": "git",
                "url": spec.url,
            },
        }
    )
    # save in cache
    cache_key = "load_manifest-%s" % path
    self.memcache_set(cache_key, manifest)
    # result = ManifestParserFactory.new(json.dumps(manifest), ManifestFileType.PACKAGE_JSON).as_dict()
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
    def configure_default_packages(self, options, targets):
        framework = options.get("pioframework")[0]
        # patch find_pkg root to ignore missing manifests and save PackageSpec
        BasePackageManager._find_pkg_root = BasePackageManager.find_pkg_root
        BasePackageManager.find_pkg_root = find_pkg_root
        # patch load_manifest to generate manifests from PackageSpec
        BasePackageManager._load_manifest = BasePackageManager.load_manifest
        BasePackageManager.load_manifest = load_manifest

        # set specific compiler versions
        if framework.startswith("realtek-ambz"):
            self.packages["toolchain-gccarmnoneeabi"]["version"] = "~1.50401.0"

        # make ArduinoCore-API required
        if "arduino" in framework:
            self.packages["framework-arduino-api"]["optional"] = False

        # save platform packages for later
        global libretuya_packages
        libretuya_packages = self.packages

        return super().configure_default_packages(options, targets)
