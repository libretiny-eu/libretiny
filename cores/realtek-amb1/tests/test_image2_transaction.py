#!/usr/bin/env python3
"""Execute the actual experimental C staging engine against a bounded NOR mock.

No hardware, SDK binding, credentials or production OTA enablement. Hash policy
uses hashlib SHA-256 here, not a toy checksum; authentication is a mock decision.
"""

import ctypes as C
import hashlib
import shutil
import struct
import subprocess
import tempfile
import unittest
from pathlib import Path


class Range(C.Structure):
    _fields_ = [("start", C.c_uint32), ("end", C.c_uint32)]


class Layout(C.Structure):
    _fields_ = [
        ("flash_size", C.c_uint32),
        ("sector_size", C.c_uint32),
        ("slots", Range * 2),
        ("reviewed", Range),
        ("protected_ranges", Range * 8),
        ("protected_count", C.c_size_t),
        ("sram_base", C.c_uint32),
        ("sram_end", C.c_uint32),
        ("sdram_base", C.c_uint32),
        ("sdram_end", C.c_uint32),
        ("running_slot", C.c_uint),
        ("reviewed_layout", C.c_int),
        ("normal_selector", C.c_int),
    ]


READ = C.CFUNCTYPE(C.c_int, C.c_void_p, C.c_uint32, C.c_void_p, C.c_size_t)
ERASE = C.CFUNCTYPE(C.c_int, C.c_void_p, C.c_uint32, C.c_size_t)
WRITE = READ
AUTH = C.CFUNCTYPE(C.c_int, C.c_void_p, C.c_uint32)
RESET = C.CFUNCTYPE(C.c_int, C.c_void_p)
UPDATE = C.CFUNCTYPE(C.c_int, C.c_void_p, C.c_void_p, C.c_size_t)


class IO(C.Structure):
    _fields_ = [
        ("ctx", C.c_void_p),
        ("read", READ),
        ("erase", ERASE),
        ("write", WRITE),
        ("authorize", AUTH),
        ("hash_reset", RESET),
        ("hash_update", UPDATE),
        ("hash_check", RESET),
    ]


class Transaction(C.Structure):
    _fields_ = [
        ("state", C.c_int),
        ("layout", Layout),
        ("io", IO),
        ("size", C.c_uint32),
        ("received", C.c_uint32),
        ("target", C.c_uint32),
        ("erase_end", C.c_uint32),
        ("signature", C.c_uint8 * 8),
    ]


NEW, OLD, FF = b"81958711", b"01958711", b"\xff" * 8


def candidate():
    ram = struct.pack("<I", 0x1000600B) + b"RTKWin" + bytes(range(240)) * 2
    dram = bytes(range(256)) * 2
    return (
        struct.pack("<II", len(ram), 0x10006000)
        + NEW
        + ram
        + struct.pack("<II", len(dram), 0x30000000)
        + FF
        + dram
    )


class Mock:
    def __init__(
        self, lib, running=0, payload=None, fail_at=None, after=False, status=0
    ):
        self.lib = lib
        self.payload = candidate() if payload is None else payload
        self.digest = hashlib.sha256(self.payload).digest()
        self.memory = bytearray(b"\xff" * 0x8000)
        self.layout = Layout(
            0x8000,
            0x100,
            (Range * 2)(Range(0x1000, 0x3000), Range(0x4000, 0x6000)),
            Range(),
            (Range * 8)(),
            2,
            0x10006000,
            0x10070000,
            0x30000000,
            0x30200000,
            running,
            1,
            1,
        )
        self.active = self.layout.slots[running].start
        self.target = self.layout.slots[1 - running].start
        self.layout.reviewed = Range(self.target, self.target + 0x1000)
        self.layout.protected_ranges[0] = Range(0, 0x1000)
        self.layout.protected_ranges[1] = Range(0x6000, 0x8000)
        self.memory[self.active : self.active + len(self.payload)] = self.payload
        self.memory[self.active + 8 : self.active + 16] = NEW
        self.memory[self.target + 8 : self.target + 16] = OLD
        self.initial = bytes(self.memory)
        self.events = []
        self.fail_at, self.after, self.status = fail_at, after, status
        self.errors = []
        self.hasher = hashlib.sha256()
        self.tx = Transaction()
        self.callbacks = IO(
            None,
            READ(self.read),
            ERASE(self.erase),
            WRITE(self.write),
            AUTH(self.auth),
            RESET(self.reset),
            UPDATE(self.update),
            RESET(self.check),
        )

    def event(self, kind, action, *details):
        self.events.append((kind, *details))
        failed = len(self.events) == self.fail_at
        if failed and not self.after:
            return self.status
        try:
            result = action()
        except Exception as exc:  # ctypes callbacks must not swallow assertions.
            self.errors.append(str(exc))
            return 0
        return self.status if failed else result

    def read(self, ctx, address, out, n):
        def action():
            assert address + n <= len(self.memory)
            C.memmove(out, bytes(self.memory[address : address + n]), n)
            return 1

        return self.event("read", action, address, n)

    def mutation_range(self, address, n):
        rounded = ((len(self.payload) + 255) // 256) * 256
        assert self.target <= address < address + n <= self.target + rounded

    def erase(self, ctx, address, n):
        def action():
            self.mutation_range(address, n)
            assert address % 256 == 0 and n == 256
            self.memory[address : address + n] = b"\xff" * n
            return 1

        return self.event("erase", action, address, n)

    def write(self, ctx, address, p, n):
        def action():
            self.mutation_range(address, n)
            data = C.string_at(p, n)
            for i, b in enumerate(data):
                assert self.memory[address + i] & b == b, "NOR 0->1 attempt"
                self.memory[address + i] &= b
            return 1

        return self.event("write", action, address, n)

    def auth(self, ctx, size):
        return self.event("authorize", lambda: int(size == len(self.payload)))

    def reset(self, ctx):
        def action():
            self.hasher = hashlib.sha256()
            return 1

        return self.event("hash_reset", action)

    def update(self, ctx, p, n):
        def action():
            self.hasher.update(C.string_at(p, n))
            return 1

        return self.event("hash_update", action, n)

    def check(self, ctx):
        return self.event(
            "hash_check", lambda: int(self.hasher.digest() == self.digest)
        )

    def begin(self, size=None):
        return self.lib.image2_begin(
            C.byref(self.tx),
            C.byref(self.layout),
            C.byref(self.callbacks),
            len(self.payload) if size is None else size,
        )

    def upload(self, chunks=(3, 6, 4, 7, 239, 1, 400, 9999)):
        if not self.begin():
            return 0
        offset = 0
        for n in chunks:
            data = self.payload[offset : offset + n]
            if not self.lib.image2_write(C.byref(self.tx), data, len(data)):
                return 0
            offset += len(data)
        assert offset == len(self.payload)
        return self.lib.image2_verify(C.byref(self.tx))

    def commit(self):
        return self.lib.image2_commit(C.byref(self.tx))

    def assert_immutable(self, test):
        rounded = ((len(self.payload) + 255) // 256) * 256
        test.assertEqual(self.memory[: self.target], self.initial[: self.target])
        test.assertEqual(
            self.memory[self.target + rounded :], self.initial[self.target + rounded :]
        )
        test.assertEqual(self.errors, [])


class Image2Test(unittest.TestCase):
    @classmethod
    def setUpClass(cls):
        cls.temp = tempfile.TemporaryDirectory(prefix="image2-transaction-")
        source = Path(__file__).resolve().parents[1] / "base/ota/image2_transaction.c"
        library = Path(cls.temp.name) / "image2.so"
        subprocess.run(
            [
                shutil.which("cc") or "cc",
                "-std=c99",
                "-Wall",
                "-Wextra",
                "-Werror",
                "-pedantic",
                "-shared",
                "-fPIC",
                str(source),
                "-o",
                str(library),
            ],
            check=True,
        )
        cls.lib = C.CDLL(str(library))
        cls.lib.image2_begin.argtypes = [
            C.POINTER(Transaction),
            C.POINTER(Layout),
            C.POINTER(IO),
            C.c_uint32,
        ]
        cls.lib.image2_write.argtypes = [C.POINTER(Transaction), C.c_void_p, C.c_size_t]
        for name in ("verify", "commit", "abort"):
            getattr(cls.lib, "image2_" + name).argtypes = [C.POINTER(Transaction)]
        cls.lib.image2_abort.restype = None

    @classmethod
    def tearDownClass(cls):
        cls.temp.cleanup()

    def test_both_directions_target_only_normal_selector(self):
        for running in (0, 1):
            m = Mock(self.lib, running)
            self.assertEqual(m.upload(), 1)
            self.assertEqual(m.tx.state, 2)
            self.assertEqual(m.memory[m.target + 8 : m.target + 16], FF)
            self.assertEqual(m.memory[m.active + 8 : m.active + 16], NEW)
            self.assertEqual(m.commit(), 1)
            self.assertEqual(m.tx.state, 6)
            self.assertEqual(m.memory[m.target : m.target + len(m.payload)], m.payload)
            # Both new prefers second (B) under reviewed NORMAL selector only.
            signatures = [
                bytes(m.memory[s.start + 8 : s.start + 16]) for s in m.layout.slots
            ]
            selected = max(i for i, sig in enumerate(signatures) if sig == NEW)
            self.assertEqual(selected, 1)
            self.assertEqual(selected == 1 - running, running == 0)
            self.assert_latched(m)

    def assert_latched(self, m):
        events = len(m.events)
        self.assertEqual(m.begin(), 0)
        self.assertEqual(self.lib.image2_write(C.byref(m.tx), b"x", 1), 0)
        self.assertEqual(self.lib.image2_verify(C.byref(m.tx)), 0)
        self.assertEqual(m.commit(), 0)
        self.lib.image2_abort(C.byref(m.tx))
        self.lib.image2_abort(C.byref(m.tx))
        self.assertEqual(len(m.events), events)
        m.assert_immutable(self)

    def test_fault_every_callback_before_and_after_effect(self):
        baseline = Mock(self.lib)
        self.assertEqual(baseline.upload(), 1)
        verify_end = len(baseline.events)
        self.assertEqual(baseline.commit(), 1)
        for running in (0, 1):
            for index in range(1, len(baseline.events) + 1):
                for after in (False, True):
                    for status in (0, 2):  # SDK-style success must be exactly 1.
                        with self.subTest(
                            running=running, index=index, after=after, status=status
                        ):
                            m = Mock(
                                self.lib,
                                running,
                                fail_at=index,
                                after=after,
                                status=status,
                            )
                            verified = m.upload()
                            if index <= verify_end:
                                self.assertEqual(verified, 0)
                                self.assertNotEqual(m.tx.state, 2)
                                self.assertNotEqual(
                                    m.memory[m.target + 8 : m.target + 16], NEW
                                )
                            else:
                                self.assertEqual(verified, 1)
                                self.assertEqual(m.commit(), 0)
                                self.assertEqual(m.tx.state, 5)
                            self.assert_latched(m)

    def test_layout_denials_before_callbacks(self):
        def protected(m):
            m.layout.protected_ranges[0] = Range(m.target + 256, m.target + 512)

        def active_target(m):
            m.layout.slots[1 - m.layout.running_slot] = m.layout.slots[
                m.layout.running_slot
            ]

        changes = [
            lambda m: setattr(m.layout, "reviewed_layout", 0),
            lambda m: setattr(m.layout, "normal_selector", 0),
            lambda m: setattr(m.layout, "running_slot", 2),
            lambda m: setattr(m.layout, "sector_size", 0),
            lambda m: setattr(m.layout, "protected_count", 9),
            lambda m: setattr(m.layout.reviewed, "end", m.target + 256),
            lambda m: setattr(m.layout.reviewed, "start", m.target + 256),
            lambda m: setattr(m.layout.reviewed, "end", m.target + 0x3000),
            lambda m: setattr(m.layout.slots[1], "end", 0xFFFFFFFF),
            lambda m: setattr(m.layout.slots[1], "start", 0x4001),
            protected,
            active_target,
        ]
        for change in changes:
            m = Mock(self.lib)
            change(m)
            self.assertEqual(m.begin(), 0)
            self.assertEqual(m.events, [])
            self.assertEqual(bytes(m.memory), m.initial)
        for size in (0, 41, 0x1001, 0xFFFFFFFF):
            m = Mock(self.lib)
            self.assertEqual(m.begin(size), 0)
            self.assertEqual(m.events, [])

    def test_bad_initial_signatures_and_unauthenticated_metadata(self):
        for which, sig in (
            ("active", OLD),
            ("active", FF),
            ("target", NEW),
            ("target", b"81958710"),
            ("target", b"\x00" * 8),
        ):
            m = Mock(self.lib)
            base = getattr(m, which)
            m.memory[base + 8 : base + 16] = sig
            before = bytes(m.memory)
            self.assertEqual(m.begin(), 0)
            self.assertEqual(bytes(m.memory), before)
            self.assertFalse(any(e[0] in ("erase", "write") for e in m.events))
        m = Mock(self.lib, fail_at=1)
        self.assertEqual(m.begin(), 0)
        self.assertEqual(bytes(m.memory), m.initial)

    def test_duplicate_begin_and_abort_never_activate(self):
        for stage in ("begin", "verify"):
            m = Mock(self.lib)
            self.assertEqual(m.begin() if stage == "begin" else m.upload(), 1)
            events = len(m.events)
            self.assertEqual(m.begin(), 0)
            self.assertEqual(len(m.events), events)
            self.lib.image2_abort(C.byref(m.tx))
            self.assertEqual(m.tx.state, 3)
            self.assertEqual(m.memory[m.target + 8 : m.target + 16], FF)
            self.assert_latched(m)
        m = Mock(self.lib)
        self.assertEqual(m.commit(), 0)  # Invalid attempt also consumes object.
        self.assertEqual(m.tx.state, 5)
        self.assert_latched(m)

    def test_truncated_and_extra_streams(self):
        for n in (0, 8, 16, len(candidate()) - 1):
            m = Mock(self.lib)
            self.assertEqual(m.begin(), 1)
            self.assertEqual(self.lib.image2_write(C.byref(m.tx), m.payload[:n], n), 1)
            self.assertEqual(self.lib.image2_verify(C.byref(m.tx)), 0)
            self.assertEqual(m.memory[m.target + 8 : m.target + 16], FF)
            self.assert_latched(m)
        m = Mock(self.lib)
        self.assertEqual(m.begin(), 1)
        extra = m.payload + b"x"
        self.assertEqual(self.lib.image2_write(C.byref(m.tx), extra, len(extra)), 0)
        self.assert_latched(m)

    def test_native_header_validation_independent_of_hash(self):
        raw = candidate()
        second = 16 + struct.unpack_from("<I", raw)[0]
        variants = []
        for offset, value in (
            (0, 9),
            (0, 0xFFFFFFFF),
            (4, 0x10006004),
            (16, 0x1000600A),
            (16, 0x10006001 + struct.unpack_from("<I", raw)[0]),
            (second, 0xFFFFFFFF),
            (second + 4, 0x30000004),
        ):
            data = bytearray(raw)
            struct.pack_into("<I", data, offset, value)
            variants.append(bytes(data))
        for offset in (8, 20, second + 8):
            data = bytearray(raw)
            data[offset] ^= 1
            variants.append(bytes(data))
        variants.extend((raw[:-1], raw + b"x"))
        for data in variants:
            m = Mock(self.lib, payload=data)  # Digest matches malformed payload.
            self.assertEqual(m.upload(), 0)
            self.assertEqual(m.memory[m.target + 8 : m.target + 16], FF)
            self.assert_latched(m)

    def test_stream_digest_and_readback_corruption(self):
        m = Mock(self.lib)
        m.digest = b"\x00" * 32
        self.assertEqual(m.upload(), 0)
        self.assert_latched(m)
        for offset in (0, 8, 27, len(candidate()) - 1):
            m = Mock(self.lib)
            self.assertEqual(m.begin(), 1)
            self.assertEqual(
                self.lib.image2_write(C.byref(m.tx), m.payload, len(m.payload)), 1
            )
            m.memory[m.target + offset] ^= 1
            self.assertEqual(self.lib.image2_verify(C.byref(m.tx)), 0)
            self.assert_latched(m)

    def test_erased_candidate_and_blank_check(self):
        m = Mock(self.lib)
        m.memory[m.target + 8 : m.target + 16] = FF
        self.assertEqual(m.upload(), 1)
        self.assertEqual(m.memory[m.target + 8 : m.target + 16], FF)
        self.lib.image2_abort(C.byref(m.tx))
        m.assert_immutable(self)
        m = Mock(self.lib)

        def not_blank(ctx, address, n):
            result = m.erase(ctx, address, n)
            m.memory[address + 20] = 0
            return result

        broken = ERASE(not_blank)
        m.callbacks.erase = broken
        self.assertEqual(m.begin(), 0)
        self.assert_latched(m)

    def test_torn_commit_and_uncertain_success_never_reopen(self):
        for written in range(9):
            for running in (0, 1):
                m = Mock(self.lib, running)
                self.assertEqual(m.upload(), 1)

                def torn(ctx, address, p, n):
                    self.assertEqual(address, m.target + 8)
                    self.assertEqual(n, 8)
                    self.assertEqual(m.tx.state, 5)
                    m.memory[address : address + written] = C.string_at(p, written)
                    return 0

                callback = WRITE(torn)
                # The transaction owns a copy of callbacks after begin.
                m.tx.io.write = callback
                self.assertEqual(m.commit(), 0)
                self.assertEqual(m.tx.state, 5)
                self.assertEqual(m.memory[m.active + 8 : m.active + 16], NEW)
                self.assert_latched(m)

    def test_commit_rechecks_withheld_marker_without_cleanup(self):
        m = Mock(self.lib)
        self.assertEqual(m.upload(), 1)
        m.memory[m.target + 8] = 0
        events = len(m.events)
        before = bytes(m.memory)
        self.assertEqual(m.commit(), 0)
        self.assertEqual(m.events[events:][0][0], "read")
        self.assertEqual(len(m.events), events + 1)
        self.assertEqual(bytes(m.memory), before)
        self.assert_latched(m)


if __name__ == "__main__":
    unittest.main()
