"""Execute the family builder with host doubles; no firmware build is needed."""

import re
import runpy
import sys
import tempfile
import unittest
from pathlib import Path
from types import ModuleType
from unittest.mock import patch

ROOT = Path(__file__).resolve().parents[3]
BUILDER = ROOT / "builder/family/realtek-amb1.py"
DEFAULT_SCRIPT = "rlx8195A-symbol-v02-img2.ld"


class Board(dict):
    def update(self, key, value):
        self[key] = value


class Queue:
    def __init__(self, env):
        self.env = env
        self.linker_paths = []

    def AppendPublic(self, **kwargs):
        if "LINKFLAGS" in kwargs:
            target = self.env["LDSCRIPT_PATH"]
            assert Path(target).is_file(), "Script must exist before link flags"
            assert self.env.board["build.ldscript"] == target
            self.linker_paths.append(target)

    def AppendPrivate(self, **kwargs):
        pass

    def AddLibrary(self, **kwargs):
        pass

    def AddExternalLibrary(self, *args, **kwargs):
        pass

    def BuildLibraries(self):
        pass


class Environment(dict):
    def __init__(self, root, selected):
        super().__init__(
            PROJECT_DIR=str(root / "project"),
            BUILD_DIR=str(root / "build" / "environment"),
            MISC_DIR=str(root / "framework" / "misc"),
            SDK_DIR=str(root / "sdk"),
            FAMILY_NAME="realtek-amb1",
            BUILDERS={},
            LIBPATH=[],
            LDSCRIPT_PATH=[selected],
        )
        self.board = Board({"build.ldscript": selected})
        self.queue = Queue(self)
        self.configured = False
        for name in ("PROJECT_DIR", "MISC_DIR", "SDK_DIR"):
            Path(self[name]).mkdir(parents=True)
        (Path(self["SDK_DIR"]) / "component").mkdir()

    def subst(self, text):
        return re.sub(
            r"\$\{(\w+)\}|\$(\w+)",
            lambda match: str(self[match[1] or match[2]]),
            text,
        )

    def BoardConfig(self):
        return self.board

    def GetProjectOption(self, name, default=None):
        return default

    def AddLibraryQueue(self, name):
        return self.queue

    def ConfigureFamily(self):
        self.configured = True

    def Replace(self, **kwargs):
        if "LDSCRIPT_PATH" in kwargs:
            assert self.configured
        self.update(kwargs)

    def Append(self, **kwargs):
        pass

    def VerboseAction(self, *args):
        return args


def execute_builder(env):
    modules = {
        name: ModuleType(name)
        for name in (
            "platformio",
            "platformio.platform",
            "platformio.platform.board",
            "SCons",
            "SCons.Script",
        )
    }
    modules["platformio.platform.board"].PlatformBoardConfig = Board
    script = modules["SCons.Script"]
    script.Environment = Environment
    script.DefaultEnvironment = lambda: env
    script.Builder = lambda **kwargs: kwargs
    with patch.dict(sys.modules, modules):
        return runpy.run_path(str(BUILDER))


class LinkerSelectionTests(unittest.TestCase):
    def setUp(self):
        self.temp = tempfile.TemporaryDirectory()
        self.addCleanup(self.temp.cleanup)
        self.root = Path(self.temp.name).resolve()
        self.env = Environment(self.root, DEFAULT_SCRIPT)

    def write_script(self, path, contents="SECTIONS { .text : { *(.text*) } }\n"):
        path.parent.mkdir(parents=True, exist_ok=True)
        path.write_text(contents)
        return path

    def assert_staged(self, source):
        original = source.read_bytes()
        execute_builder(self.env)
        target = Path(self.env["BUILD_DIR"]) / source.name
        self.assertEqual(target.read_bytes(), original)
        self.assertEqual(self.env["LDSCRIPT_PATH"], str(target))
        self.assertEqual(self.env.board["build.ldscript"], str(target))
        self.assertEqual(self.env.queue.linker_paths, [str(target)])
        target.write_text("/* framework patch */\n")
        self.assertEqual(source.read_bytes(), original)
        return target

    def test_board_default_is_writable_build_local_copy(self):
        contents = (ROOT / "cores/realtek-amb1/misc" / DEFAULT_SCRIPT).read_text()
        source = self.write_script(
            Path(self.env["MISC_DIR"]) / DEFAULT_SCRIPT, contents
        )
        source.chmod(0o444)
        self.assert_staged(source)

    def test_absolute_custom_path_wins_over_default(self):
        self.write_script(Path(self.env["MISC_DIR"]) / DEFAULT_SCRIPT)
        source = self.write_script(self.root / "custom" / "application.ld")
        self.env.board["build.ldscript"] = str(source)
        self.assert_staged(source)

    def test_project_relative_custom_path_and_variable_expansion(self):
        source = self.write_script(Path(self.env["PROJECT_DIR"]) / "ld/custom.ld")
        self.env["CUSTOM_DIR"] = "ld"
        self.env.board["build.ldscript"] = "${CUSTOM_DIR}/custom.ld"
        self.assert_staged(source)

    def test_project_script_takes_priority_over_family_script(self):
        self.write_script(Path(self.env["MISC_DIR"]) / DEFAULT_SCRIPT, "family\n")
        source = self.write_script(Path(self.env["PROJECT_DIR"]) / DEFAULT_SCRIPT)
        self.assert_staged(source)

    def test_library_search_path(self):
        source = self.write_script(self.root / "library" / "custom.ld")
        self.env["LIBPATH"] = [str(source.parent)]
        self.env.board["build.ldscript"] = source.name
        self.assert_staged(source)

    def test_missing_explicit_script_does_not_fall_back_to_family(self):
        self.write_script(Path(self.env["MISC_DIR"]) / DEFAULT_SCRIPT)
        missing = self.root / "missing" / DEFAULT_SCRIPT
        self.env.board["build.ldscript"] = str(missing)
        with self.assertRaisesRegex(FileNotFoundError, "Linker script not found"):
            execute_builder(self.env)
        self.assertEqual(self.env.queue.linker_paths, [])

    def test_fresh_setup_resets_previous_framework_patch(self):
        source = self.write_script(Path(self.env["MISC_DIR"]) / DEFAULT_SCRIPT)
        target = self.assert_staged(source)
        # PlatformIO reloads board metadata for each build invocation.
        self.env.board["build.ldscript"] = DEFAULT_SCRIPT
        execute_builder(self.env)
        self.assertEqual(target.read_bytes(), source.read_bytes())

    def test_already_build_local_selection_is_not_copied_onto_itself(self):
        target = self.write_script(Path(self.env["BUILD_DIR"]) / "custom.ld")
        self.env.board["build.ldscript"] = str(target)
        execute_builder(self.env)
        self.assertEqual(self.env["LDSCRIPT_PATH"], str(target))
        self.assertIn("SECTIONS", target.read_text())


if __name__ == "__main__":
    unittest.main()
