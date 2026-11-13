"""Exercise the family builder with disposable SDK files and queue mocks."""

import ast
from pathlib import Path
import re
from shutil import copyfile
import subprocess
import tempfile
import unittest
from os.path import isdir, join


ROOT = Path(__file__).resolve().parents[3]
BUILDER = ROOT / "builder/family/realtek-amb1.py"
OTA = ROOT / "cores/realtek-amb1/base/ota"
PATCH = OTA / "image2_controller_lock.patch"


def make_sdk(root):
    # Reconstruct each original hunk's exact context, with irrelevant lines
    # filled in. This exercises the shipped patch, not a replacement patch.
    files = {}
    current = None
    position = None
    for line in PATCH.read_text().splitlines(keepends=True):
        if line.startswith("--- a/"):
            current = line[len("--- a/"):].strip()
            files[current] = []
            position = None
        elif line.startswith("@@ "):
            position = int(re.match(r"@@ -(\d+)", line).group(1)) - 1
        elif position is not None and line[:1] in (" ", "-"):
            lines = files[current]
            while len(lines) <= position:
                lines.append("/* unused fixture line */\n")
            lines[position] = line[1:]
            position += 1
    for relative, lines in files.items():
        target = root / relative
        target.parent.mkdir(parents=True, exist_ok=True)
        target.write_text("".join(lines))
    return {relative: (root / relative).read_bytes() for relative in files}


class Queue:
    def __init__(self, env):
        self.env = env
        self.libraries = []

    def AddLibrary(self, **kwargs):
        self.libraries.append(kwargs)

    def AppendPublic(self, **kwargs):
        pass

    AppendPrivate = AppendPublic

    def AddExternalLibrary(self, *args, **kwargs):
        pass

    def BuildLibraries(self):
        self.snapshot = dict(self.env)


class Env(dict):
    def __init__(self, root, option):
        super().__init__(BUILDERS={}, FAMILY_NAME="realtek-amb1")
        self.root, self.option = root, option
        self.queue = Queue(self)
        self.board = {"build.ldscript": str(root / "test.ld")}
        (root / "test.ld").write_text("/* linker fixture */\n")

    def BoardConfig(self):
        return type("Board", (), {
            "get": lambda _, key: self.board[key],
            "update": lambda _, key, value: self.board.update({key: value}),
        })()

    def GetProjectOption(self, key, default):
        assert key == "custom_amb1_flash_qualification" and default == "no"
        return self.option if self.option is not None else default

    def subst(self, value):
        for key, path in {
            "SDK_DIR": self.root / "sdk", "BUILD_DIR": self.root / "build",
            "CORES_DIR": ROOT / "cores",
        }.items():
            value = value.replace("$" + key, str(path))
        return value

    def AddLibraryQueue(self, name):
        return self.queue

    def ConfigureFamily(self):
        pass

    def Replace(self, **kwargs):
        self.update(kwargs)

    def Append(self, **kwargs):
        for key, values in kwargs.items():
            self[key] = self.get(key, []) + values

    def Prepend(self, **kwargs):
        for key, values in kwargs.items():
            self[key] = values + self.get(key, [])

    def VerboseAction(self, *args):
        return args


def load_builder(env=None):
    tree = ast.parse(BUILDER.read_text())
    tree.body = [node for node in tree.body if not isinstance(node, (ast.Import, ast.ImportFrom))
                 and (env is not None or isinstance(node, ast.FunctionDef))]
    namespace = dict(Path=Path, copyfile=copyfile, subprocess=subprocess,
                     isdir=isdir, join=join, Environment=Env,
                     PlatformBoardConfig=object, DefaultEnvironment=lambda: env,
                     Builder=lambda **kwargs: kwargs)
    exec(compile(tree, str(BUILDER), "exec"), namespace)
    return namespace


class SDKLockStagingTests(unittest.TestCase):
    def test_actual_patch_refreshes_copies_and_preserves_originals(self):
        stage_lock = load_builder()["stage_image2_sdk_lock"]
        with tempfile.TemporaryDirectory() as tmp:
            root = Path(tmp)
            originals = make_sdk(root / "sdk")
            header = "component/os/os_dep/include/device_lock.h"
            originals[header] = originals[header].replace(b"\n", b"\r\n")
            (root / "sdk" / header).write_bytes(originals[header])
            stage = stage_lock(root / "sdk", root / "build", PATCH)
            for relative, original in originals.items():
                self.assertEqual((root / "sdk" / relative).read_bytes(), original)
                self.assertIn(b"device_mutex_try_lock", (stage / relative).read_bytes())
            self.assertEqual(stage_lock(root / "sdk", root / "build", PATCH), stage)

    def test_invalid_patch_and_fuzzy_context_fail(self):
        stage_lock = load_builder()["stage_image2_sdk_lock"]
        with tempfile.TemporaryDirectory() as tmp:
            root = Path(tmp)
            originals = make_sdk(root / "sdk")
            bad = root / "bad.patch"
            bad.write_text("not a patch\n")
            with self.assertRaises(RuntimeError):
                stage_lock(root / "sdk", root / "build", bad)
            source = root / "sdk/component/os/os_dep/device_lock.c"
            source.write_text(source.read_text().replace(
                '#include "osdep_service.h"', '#include "changed_context.h"'))
            changed = source.read_bytes()
            with self.assertRaises(RuntimeError):
                stage_lock(root / "sdk", root / "build", PATCH)
            self.assertEqual(source.read_bytes(), changed)
            header = "component/os/os_dep/include/device_lock.h"
            self.assertEqual((root / "sdk" / header).read_bytes(), originals[header])

    def test_builder_opt_in_and_default_queues(self):
        for option in (None, "no", "yes"):
            with self.subTest(option=option), tempfile.TemporaryDirectory() as tmp:
                root = Path(tmp)
                originals = make_sdk(root / "sdk")
                env = Env(root, option)
                load_builder(env)
                libraries = {item["name"]: item for item in env.queue.libraries}
                enabled = option == "yes"
                self.assertEqual("amb1_image2_sdk_adapter" in libraries, enabled)
                self.assertEqual("amb1_image2_sdk_lock" in libraries, enabled)
                exclusions = libraries["amb1_osglue"]["srcs"]
                self.assertEqual("-<component/os/os_dep/device_lock.c>" in exclusions, enabled)
                macro = ("LT_AMB1_EXPERIMENTAL_IMAGE2_SDK_ADAPTER", 1)
                self.assertEqual(macro in env.queue.snapshot.get("CPPDEFINES", []), enabled)
                if enabled:
                    self.assertEqual(Path(env["CPPPATH"][0]), (
                        root / "build/amb1_image2_sdk/component/os/os_dep/include").resolve())
                    self.assertEqual(libraries["amb1_image2_sdk_adapter"]["base_dir"], str(OTA))
                    self.assertEqual(libraries["amb1_image2_sdk_lock"]["srcs"],
                                     ["+<component/os/os_dep/device_lock.c>"])
                else:
                    self.assertFalse((root / "build/amb1_image2_sdk").exists())
                for relative, original in originals.items():
                    self.assertEqual((root / "sdk" / relative).read_bytes(), original)


if __name__ == "__main__":
    unittest.main()
