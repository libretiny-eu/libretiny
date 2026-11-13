#!/usr/bin/env python3
"""Compile the proposed C region policy; synthetic tests, no SDK or flash I/O."""
import ctypes
from pathlib import Path
import shutil
import subprocess
import tempfile
import unittest


HEADER = Path(__file__).resolve().parents[1] / "base/port/preferences_layout.h"


class Review(ctypes.Structure):
    _fields_ = [(name, ctypes.c_int) for name in (
        "reviewed_reservation", "snapshot_matches_review", "migrated_native_loader",
        "normal_selector", "inactive_b_reservation", "admitted_images_exclude_kvs")]


class PreferencesLayoutTest(unittest.TestCase):
    @classmethod
    def setUpClass(cls):
        cc = shutil.which("cc")
        if not cc:
            raise unittest.SkipTest("host C compiler unavailable")
        cls.temp = tempfile.TemporaryDirectory(prefix="amb1-preferences-")
        cls.addClassCleanup(cls.temp.cleanup)
        source = Path(cls.temp.name) / "policy.c"
        source.write_text(
            '#include "preferences_layout.h"\n'
            'int region(const amb1_preferences_review_t *r, uint32_t a, uint32_t n, int e) '
            '{ return amb1_preferences_range_allowed(r, a, n, e); }\n'
            'int image(unsigned s, uint32_t n) '
            '{ return amb1_preferences_image_allowed(s, n); }\n')
        library = Path(cls.temp.name) / "policy.so"
        subprocess.run([cc, "-std=c99", "-Wall", "-Wextra", "-Werror", "-shared",
                        "-fPIC", "-I", str(HEADER.parent), str(source), "-o", str(library)],
                       check=True, capture_output=True)
        cls.lib = ctypes.CDLL(str(library))
        cls.lib.region.argtypes = [ctypes.POINTER(Review), ctypes.c_uint32,
                                   ctypes.c_uint32, ctypes.c_int]
        cls.lib.region.restype = ctypes.c_int
        cls.lib.image.argtypes = [ctypes.c_uint, ctypes.c_uint32]
        cls.lib.image.restype = ctypes.c_int

    def setUp(self):
        self.review = Review(1, 1, 1, 1, 1, 1)

    def region(self, start, length, erase=False):
        return self.lib.region(ctypes.byref(self.review), start, length, erase)

    def test_review_is_fail_closed_for_every_fact(self):
        self.assertFalse(self.lib.region(None, 0x275000, 0x1000, 1))
        for field, _ in Review._fields_:
            for denied in (0, -1, 2):
                with self.subTest(field=field, value=denied):
                    self.review = Review(1, 1, 1, 1, 1, 1)
                    setattr(self.review, field, denied)
                    self.assertFalse(self.region(0x275000, 0x1000, True))

    def test_exact_two_sector_geometry(self):
        for start in (0x275000, 0x276000):
            self.assertTrue(self.region(start, 0x1000, True))
        self.assertTrue(self.region(0x275000, 0x2000, True))
        self.assertTrue(self.region(0x276FFF, 1))
        self.assertFalse(self.region(0x276FFF, 2))

    def test_no_implicit_erase_rounding(self):
        for start, size in ((0x275001, 0xFFF), (0x275000, 1),
                            (0x275FFF, 2), (0x276000, 0x1001)):
            with self.subTest(start=start, size=size):
                self.assertFalse(self.region(start, size, True))
        self.assertTrue(self.region(0x275001, 0xFFF))

    def test_every_other_sector_is_denied(self):
        # Includes boot/calibration, admitted images, preserved island, OBK,
        # original GE OTA and all still-unassigned upper-flash sectors.
        for start in range(0, 0x800000, 0x1000):
            with self.subTest(start=hex(start)):
                self.assertEqual(bool(self.region(start, 0x1000, True)),
                                 start in (0x275000, 0x276000))

    def test_empty_overflow_and_boundary_crossing(self):
        for start, size in ((0x275000, 0), (0x274FFF, 2), (0x274000, 0x4000),
                            (0x277000, 1), (0xFFFFFFFF, 2),
                            (0x275000, 0xFFFFFFFF), (0, 0xFFFFFFFF)):
            with self.subTest(start=start, size=size):
                self.assertFalse(self.region(start, size))

    def test_symmetric_ota_admission_and_rounded_extents(self):
        for slot in (0, 1):
            for size in (42, 0xFD001, 0xFE000):
                self.assertTrue(self.lib.image(slot, size))
            for size in (0, 41, 0xFE001, 0x100000, 0xFFFFFFFF):
                self.assertFalse(self.lib.image(slot, size))
        self.assertFalse(self.lib.image(2, 42))

    def test_every_admitted_image_sector_excludes_kvs(self):
        for start in (0xB000, 0x177000):
            for size in range(42, 0xFE001, 997):
                rounded = (size + 0xFFF) // 0x1000 * 0x1000
                self.assertLessEqual(start + rounded, 0x275000)
                for address in (start, start + rounded - 1):
                    self.assertFalse(self.region(address, 1))


if __name__ == "__main__":
    unittest.main()
