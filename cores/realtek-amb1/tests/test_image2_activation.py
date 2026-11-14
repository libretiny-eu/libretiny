#!/usr/bin/env python3
"""Host-only coordinator/transaction tests; no SDK or hardware acceptance.

The selector model classifies signatures only, assuming both bodies loadable.
It is not a full loader emulator, GPIO selector model, or crash rollback proof.
"""

import ctypes as C
import shutil
import subprocess
import tempfile
import unittest
from pathlib import Path

from test_image2_transaction import FF, IO, NEW, OLD, Layout, Mock, Transaction

DEMOTE = C.CFUNCTYPE(C.c_int, C.c_void_p)


class Activation(C.Structure):
    _fields_ = [
        ("transaction", Transaction),
        ("activation_attempted", C.c_uint),
        ("trusted_running_slot", C.c_uint),
        ("expected_source_signature", C.c_uint8 * 8),
    ]


def normal_choose(signatures):
    # Reviewed NORMAL preference: new over old, B wins same-class ties.
    for signature in (NEW, OLD):
        choices = [i for i, value in enumerate(signatures) if value == signature]
        if choices:
            return max(choices)
    return None


class ActivationMock(Mock):
    def __init__(self, lib, running=0):
        super().__init__(lib, running)
        self.activation = Activation()
        self.activating = False
        self.fault = None
        self.fault_index = None
        self.torn_bits = 0
        self.reenter = False
        self.status = 0

        def demote(_):
            # Unit-only capability; the linked session tests use the actual
            # backend's target/source rechecks and exact-byte operation.
            sig = C.create_string_buffer(8)
            return int(
                self.tx.io.write(self.tx.io.ctx, self.active + 8, OLD, 1) == 1
                and self.tx.io.read(self.tx.io.ctx, self.active + 8, sig, 8) == 1
                and sig.raw == OLD
            )

        self.demote = DEMOTE(demote)

    @property
    def tx(self):
        return self.activation.transaction

    @tx.setter
    def tx(self, value):
        # Base fixture initializes a transaction before coordinator construction.
        pass

    def begin(self, size=None):
        return self.lib.image2_activation_begin(
            C.byref(self.activation),
            C.byref(self.layout),
            C.byref(self.callbacks),
            len(self.payload) if size is None else size,
            self.layout.running_slot,
            NEW,
        )

    def upload(self):
        return (
            self.begin()
            and self.lib.image2_activation_write(
                C.byref(self.activation), self.payload, len(self.payload)
            )
            and self.lib.image2_activation_verify(C.byref(self.activation))
        )

    def commit(self):
        self.activating = True
        return self.lib.image2_activation_commit(
            C.byref(self.activation), self.demote, None
        )

    def mutation_range(self, address, n):
        if self.activating:
            assert (address, n) in ((self.target + 8, 8), (self.active + 8, 1))
        else:
            super().mutation_range(address, n)

    def event(self, kind, action, *details):
        if not self.activating:
            return super().event(kind, action, *details)
        self.events.append((kind, *details))
        try:
            assert self.activation.activation_attempted == 1
            assert kind != "erase", "activation erased flash"
            if self.reenter:
                count = len(self.events)
                assert self.commit() == 0
                assert self.begin() == 0
                assert (
                    self.lib.image2_activation_write(C.byref(self.activation), b"x", 1)
                    == 0
                )
                assert self.lib.image2_activation_verify(C.byref(self.activation)) == 0
                assert len(self.events) == count
            failed = len(self.events) == self.fault_index
            if failed and self.fault == "before":
                return self.status
            if failed and self.fault == "torn" and kind == "write":
                address, n = details
                data = NEW if n == 8 else OLD[:1]
                bits = [
                    (i, b)
                    for i in range(n)
                    for b in range(8)
                    if self.memory[address + i] & (1 << b) and not data[i] & (1 << b)
                ]
                for i, b in bits[: self.torn_bits]:
                    self.memory[address + i] &= ~(1 << b)
                return self.status
            result = action()
            return self.status if failed else result
        except Exception as exc:
            self.errors.append(str(exc))
            return 0

    def selected(self):
        return normal_choose(
            [bytes(self.memory[s.start + 8 : s.start + 16]) for s in self.layout.slots]
        )


class Image2ActivationTest(unittest.TestCase):
    @classmethod
    def setUpClass(cls):
        cls.temp = tempfile.TemporaryDirectory(prefix="image2-activation-")
        ota = Path(__file__).resolve().parents[1] / "base/ota"
        library = Path(cls.temp.name) / "activation.so"
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
                str(ota / "image2_transaction.c"),
                str(ota / "image2_activation.c"),
                "-o",
                str(library),
            ],
            check=True,
        )
        cls.lib = C.CDLL(str(library))
        cls.lib.image2_activation_begin.argtypes = [
            C.POINTER(Activation),
            C.POINTER(Layout),
            C.POINTER(IO),
            C.c_uint32,
            C.c_uint,
            C.c_void_p,
        ]
        cls.lib.image2_activation_write.argtypes = [
            C.POINTER(Activation),
            C.c_void_p,
            C.c_size_t,
        ]
        for name in ("verify", "commit", "abort"):
            getattr(cls.lib, "image2_activation_" + name).argtypes = [
                C.POINTER(Activation)
            ]
        cls.lib.image2_activation_abort.restype = None
        cls.lib.image2_activation_commit.argtypes = [
            C.POINTER(Activation),
            DEMOTE,
            C.c_void_p,
        ]

    @classmethod
    def tearDownClass(cls):
        cls.temp.cleanup()

    def ready(self, running):
        m = ActivationMock(self.lib, running)
        self.assertEqual(m.upload(), 1)
        self.assertEqual(m.memory[m.target + 8 : m.target + 16], FF)
        m.events.clear()
        return m

    def assert_latched(self, m):
        count = len(m.events)
        self.assertEqual(m.activation.activation_attempted, 1)
        self.assertEqual(m.begin(), 0)
        self.assertEqual(m.commit(), 0)
        self.assertEqual(
            self.lib.image2_activation_write(C.byref(m.activation), b"x", 1), 0
        )
        self.assertEqual(self.lib.image2_activation_verify(C.byref(m.activation)), 0)
        self.lib.image2_activation_abort(C.byref(m.activation))
        self.lib.image2_activation_abort(C.byref(m.activation))
        self.assertEqual(len(m.events), count)
        self.assertEqual(m.errors, [])

    def assert_stable(self, m, before):
        # Only the nine explicitly allowed signature bytes may ever change.
        for start, end in sorted(
            ((m.active + 8, m.active + 9), (m.target + 8, m.target + 16))
        ):
            self.assertEqual(m.memory[:start], before[:start])
            before = before[:start] + bytes(m.memory[start:end]) + before[end:]
        self.assertEqual(bytes(m.memory), before)
        self.assertTrue(all(e[0] != "erase" for e in m.events))

    def test_success_both_directions_and_serialized_reentry(self):
        for running in (0, 1):
            m = self.ready(running)
            before = bytes(m.memory)
            m.reenter = True
            self.assertEqual(m.commit(), 1)
            self.assertEqual(
                m.events,
                [
                    ("read", m.active + 8, 8),
                    ("read", m.target + 8, 8),
                    ("write", m.target + 8, 8),
                    ("read", m.target + 8, 8),
                    ("write", m.active + 8, 1),
                    ("read", m.active + 8, 8),
                ],
            )
            self.assertEqual(m.selected(), 1 - running)
            self.assertEqual(m.memory[m.target : m.target + len(m.payload)], m.payload)
            self.assertEqual(m.memory[m.active + 8 : m.active + 16], OLD)
            self.assert_stable(m, before)
            self.assert_latched(m)

    def test_each_callback_failure_before_and_after_effect(self):
        for running in (0, 1):
            for index in range(1, 7):
                for fault in ("before", "after"):
                    for status in (0, 2, -1):
                        with self.subTest(
                            running=running, index=index, fault=fault, status=status
                        ):
                            m = self.ready(running)
                            before = bytes(m.memory)
                            m.fault_index, m.fault, m.status = index, fault, status
                            self.assertEqual(m.commit(), 0)
                            self.assertEqual(len(m.events), index)
                            published = index > 3 or (index == 3 and fault == "after")
                            demoted = index > 5 or (index == 5 and fault == "after")
                            expected = (
                                1 - running
                                if demoted
                                else (1 if published else running)
                            )
                            self.assertEqual(m.selected(), expected)
                            self.assert_stable(m, before)
                            self.assert_latched(m)

    def test_torn_program_each_cleared_bit(self):
        target_bits = sum(8 - byte.bit_count() for byte in NEW)
        for running in (0, 1):
            for index, count in ((3, target_bits), (5, 1)):
                for bits in range(count + 1):
                    with self.subTest(running=running, index=index, bits=bits):
                        m = self.ready(running)
                        before = bytes(m.memory)
                        m.fault_index, m.fault, m.torn_bits = index, "torn", bits
                        self.assertEqual(m.commit(), 0)
                        expected = 1 if index == 3 and bits == count else running
                        if index == 5:
                            expected = 1 - running if bits else 1
                        self.assertEqual(m.selected(), expected)
                        self.assert_stable(m, before)
                        self.assert_latched(m)

    def test_explicit_identity_expected_source_and_selector(self):
        for field in ("identity", "expected", "selector", "layout"):
            m = ActivationMock(self.lib)
            expected, running = NEW, 0
            if field == "identity":
                running = 1
            if field == "expected":
                expected = OLD
            if field == "selector":
                m.layout.normal_selector = 0
            if field == "layout":
                m.layout.reviewed_layout = 0
            before = bytes(m.memory)
            self.assertEqual(
                self.lib.image2_activation_begin(
                    C.byref(m.activation),
                    C.byref(m.layout),
                    C.byref(m.callbacks),
                    len(m.payload),
                    running,
                    expected,
                ),
                0,
            )
            self.assertEqual(m.begin(), 0)
            self.assertEqual(bytes(m.memory), before)
            self.assertEqual(m.events, [])

    def test_partial_program_with_false_success_is_readback_checked(self):
        for index in (3, 5):
            for running in (0, 1):
                m = self.ready(running)
                m.fault_index, m.fault, m.torn_bits, m.status = index, "torn", 0, 1
                self.assertEqual(m.commit(), 0)
                self.assertEqual(len(m.events), index + 1)
                self.assertEqual(m.selected(), running if index == 3 else 1)
                self.assert_latched(m)

    def test_source_rechecked_not_used_to_infer_running(self):
        for running in (0, 1):
            for signature in (OLD, FF, b"x1958711"):
                m = self.ready(running)
                m.memory[m.active + 8 : m.active + 16] = signature
                before = bytes(m.memory)
                self.assertEqual(m.commit(), 0)
                self.assertEqual(m.events, [("read", m.active + 8, 8)])
                self.assertEqual(bytes(m.memory), before)
                self.assert_latched(m)

    def test_bad_target_readback_and_source_readback(self):
        for address_kind in ("target", "source"):
            m = self.ready(1)
            original = m.callbacks.read
            # Retain the Python-bound callback explicitly for the C call lifetime.
            from test_image2_transaction import READ

            def mismatched(ctx, address, out, n):
                result = m.read(ctx, address, out, n)
                if len(m.events) == (4 if address_kind == "target" else 6):
                    C.memmove(out, b"?" * n, n)
                return result

            callback = READ(mismatched)
            m.tx.io.read = callback
            self.assertEqual(m.commit(), 0)
            self.assertEqual(len(m.events), 4 if address_kind == "target" else 6)
            self.assertEqual(m.selected(), 1 if address_kind == "target" else 0)
            self.assert_latched(m)
            self.assertIsNotNone(original)

    def test_invalid_commit_permanently_consumes_shared_owner(self):
        for phase in ("idle", "staging", "aborted"):
            m = ActivationMock(self.lib)
            if phase != "idle":
                self.assertEqual(m.begin(), 1)
            if phase == "aborted":
                self.lib.image2_activation_abort(C.byref(m.activation))
            m.events.clear()
            self.assertEqual(m.commit(), 0)
            self.assertEqual(m.events, [])
            self.assert_latched(m)

    def test_signature_model_scope(self):
        self.assertEqual(normal_choose([NEW, NEW]), 1)
        self.assertEqual(normal_choose([OLD, OLD]), 1)
        self.assertEqual(normal_choose([NEW, OLD]), 0)
        self.assertEqual(normal_choose([OLD, NEW]), 1)
        self.assertEqual(normal_choose([b"invalid!", OLD]), 1)
        self.assertIsNone(normal_choose([FF, b"invalid!"]))


if __name__ == "__main__":
    unittest.main()
