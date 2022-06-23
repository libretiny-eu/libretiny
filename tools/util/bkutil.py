# Copyright (c) Kuba Szczodrzyński 2022-06-10.

import sys
from os.path import dirname, join

sys.path.append(join(dirname(__file__), "..", ".."))

from argparse import ArgumentParser, FileType
from binascii import crc32
from dataclasses import dataclass, field
from enum import Enum, IntFlag
from io import SEEK_SET, FileIO
from os import stat
from struct import Struct
from time import time
from typing import Generator, Tuple, Union

from tools.util.bitint import BitInt
from tools.util.bkcrypto import BekenCrypto
from tools.util.crc16 import CRC16
from tools.util.fileio import readbin, writebin
from tools.util.intbin import (
    ByteGenerator,
    align_up,
    betoint,
    biniter,
    fileiter,
    geniter,
    inttobe16,
    inttole32,
    letoint,
    pad_data,
    pad_up,
)


class DataType(Enum):
    BINARY = "BINARY"
    PADDING_SIZE = "PADDING_SIZE"
    RBL = "RBL"


DataTuple = Tuple[DataType, Union[bytes, int]]
DataUnion = Union[bytes, DataTuple]
DataGenerator = Generator[DataUnion, None, None]


class OTAAlgorithm(IntFlag):
    NONE = 0
    CRYPT_XOR = 1
    CRYPT_AES256 = 2
    COMPRESS_GZIP = 256
    COMPRESS_QUICKLZ = 512
    COMPRESS_FASTLZ = 768


@dataclass
class RBL:
    ota_algo: OTAAlgorithm = OTAAlgorithm.NONE
    timestamp: float = field(default_factory=time)
    name: Union[str, bytes] = "app"
    version: Union[str, bytes] = "1.00"
    sn: Union[str, bytes] = "0" * 23
    data_crc: int = 0
    data_hash: int = 0x811C9DC5  # https://github.com/znerol/py-fnvhash/blob/master/fnvhash/__init__.py
    raw_size: int = 0
    data_size: int = 0
    container_size: int = 0
    has_part_table: bool = False

    @property
    def container_size_crc(self) -> int:
        return int(self.container_size + (self.container_size // 32) * 2)

    def update(self, data: bytes):
        self.data_crc = crc32(data, self.data_crc)
        for byte in data:
            if self.data_size < self.raw_size:
                self.data_hash ^= byte
                self.data_hash *= 0x01000193
                self.data_hash %= 0x100000000
            self.data_size += 1

    def serialize(self) -> bytes:
        if isinstance(self.name, str):
            self.name = self.name.encode()
        if isinstance(self.version, str):
            self.version = self.version.encode()
        if isinstance(self.sn, str):
            self.sn = self.sn.encode()
        # based on https://github.com/khalednassar/bk7231tools/blob/main/bk7231tools/analysis/rbl.py
        struct = Struct("<4sII16s24s24sIIII")  # without header CRC
        rbl = struct.pack(
            b"RBL\x00",
            self.ota_algo,
            int(self.timestamp),
            pad_data(self.name, 16, 0x00),
            pad_data(self.version, 24, 0x00),
            pad_data(self.sn, 24, 0x00),
            self.data_crc,
            self.data_hash,
            self.raw_size,
            self.data_size,
        )
        return rbl + inttole32(crc32(rbl))

    @classmethod
    def deserialize(cls, data: bytes) -> "RBL":
        crc_found = letoint(data[-4:])
        data = data[:-4]
        crc_expected = crc32(data)
        if crc_expected != crc_found:
            raise ValueError(
                f"Invalid RBL CRC (expected {crc_expected:X}, found {crc_found:X})"
            )
        struct = Struct("<II16s24s24sIIII")  # without magic and header CRC
        rbl = cls(*struct.unpack(data[4:]))
        rbl.ota_algo = OTAAlgorithm(rbl.ota_algo)
        rbl.name = rbl.name.partition(b"\x00")[0].decode()
        rbl.version = rbl.version.partition(b"\x00")[0].decode()
        rbl.sn = rbl.sn.partition(b"\x00")[0].decode()
        return rbl


class BekenBinary:
    crypto: BekenCrypto

    def __init__(self, coeffs: Union[bytes, str] = None) -> None:
        if coeffs:
            if isinstance(coeffs, str):
                coeffs = bytes.fromhex(coeffs)
            if len(coeffs) != 16:
                raise ValueError(
                    f"Invalid length of encryption coefficients: {len(coeffs)}"
                )
            coeffs = list(map(BitInt, map(betoint, biniter(coeffs, 4))))
            self.crypto = BekenCrypto(coeffs)

    def crc(self, data: ByteGenerator, type: DataType = None) -> DataGenerator:
        for block in geniter(data, 32):
            crc = CRC16.CMS.calc(block)
            block += inttobe16(crc)
            if type:
                yield (type, block)
            else:
                yield block

    def uncrc(self, data: ByteGenerator, check: bool = True) -> ByteGenerator:
        for block in geniter(data, 34):
            if check:
                crc = CRC16.CMS.calc(block[0:32])
                crc_found = betoint(block[32:34])
                if crc != crc_found:
                    print(f"CRC invalid: expected={crc:X}, found={crc_found:X}")
                    return
            yield block[0:32]

    def crypt(self, addr: int, data: ByteGenerator) -> ByteGenerator:
        for word in geniter(data, 4):
            word = letoint(word)
            word = self.crypto.encrypt_u32(addr, word)
            word = inttole32(word)
            yield word
            addr += 4

    def package(
        self,
        f: FileIO,
        addr: int,
        size: int,
        rbl: RBL,
        partial: bool = False,
    ) -> DataGenerator:
        if not rbl.container_size:
            raise ValueError("RBL must have a total size when packaging")
        crc_total = 0

        # yield all data as (type, bytes) tuples, if partial mode enabled
        type_binary = DataType.BINARY if partial else None
        type_padding = DataType.PADDING_SIZE if partial else None
        type_rbl = DataType.RBL if partial else None

        # when to stop reading input data
        data_end = size
        if rbl.has_part_table:
            data_end = size - 0xC0  # do not encrypt the partition table

        # set RBL size including one 16-byte padding
        rbl.raw_size = align_up(size + 16, 32) + 16

        # encrypt the input file, padded to 32 bytes
        data_crypt_gen = self.crypt(
            addr, fileiter(f, size=32, padding=0xFF, count=data_end)
        )
        # iterate over encrypted 32-byte blocks
        for block in geniter(data_crypt_gen, 32):
            # add CRC16 and yield
            yield from self.crc(block, type_binary)
            crc_total += 2
            rbl.update(block)

        # temporary buffer for small-size operations
        buf = b"\xff" * 16  # add 16 bytes of padding

        if rbl.has_part_table:
            # add an unencrypted partition table
            buf += f.read(0xC0)

        # update RBL
        rbl.update(buf)
        # add last padding with different values
        rbl.update(b"\x10" * 16)

        # add last padding with normal values
        buf += b"\xff" * 16
        # yield the temporary buffer
        yield from self.crc(buf, type_binary)
        crc_total += 2 * (len(buf) // 32)

        # pad the entire container with 0xFF, excluding RBL and its CRC16
        pad_size = pad_up(rbl.data_size + crc_total, rbl.container_size_crc) - 102
        if type_padding:
            yield (type_padding, pad_size)
        else:
            for _ in range(pad_size):
                yield b"\xff"

        # yield RBL with CRC16
        yield from self.crc(rbl.serialize(), type_rbl)


def auto_int(x):
    return int(x, 0)


def add_common_args(parser):
    parser.add_argument(
        "coeffs", type=str, help="Encryption coefficients (hex string, 32 chars)"
    )
    parser.add_argument("input", type=FileType("rb"), help="Input file")
    parser.add_argument("output", type=FileType("wb"), help="Output file")
    parser.add_argument("addr", type=auto_int, help="Memory address (dec/hex)")


if __name__ == "__main__":
    parser = ArgumentParser(description="Encrypt/decrypt Beken firmware binaries")
    sub = parser.add_subparsers(dest="action", required=True)

    encrypt = sub.add_parser("encrypt", help="Encrypt binary files without packaging")
    add_common_args(encrypt)
    encrypt.add_argument("-c", "--crc", help="Include CRC16", action="store_true")

    decrypt = sub.add_parser("decrypt", description="Decrypt unpackaged binary files")
    add_common_args(decrypt)
    decrypt.add_argument(
        "-C",
        "--no-crc-check",
        help="Do not check CRC16 (if present)",
        action="store_true",
    )

    package = sub.add_parser(
        "package", description="Package raw binary files as RBL containers"
    )
    add_common_args(package)
    package.add_argument(
        "size", type=auto_int, help="RBL total size (excl. CRC) (dec/hex)"
    )
    package.add_argument(
        "-n",
        "--name",
        type=str,
        help="Firmware name (default: app)",
        default="app",
        required=False,
    )
    package.add_argument(
        "-v",
        "--version",
        type=str,
        help="Firmware version (default: 1.00)",
        default="1.00",
        required=False,
    )

    unpackage = sub.add_parser(
        "unpackage", description="Unpackage a single RBL container"
    )
    add_common_args(unpackage)
    unpackage.add_argument(
        "offset", type=auto_int, help="Offset in input file (dec/hex)"
    )
    unpackage.add_argument(
        "size", type=auto_int, help="Container total size (incl. CRC) (dec/hex)"
    )

    args = parser.parse_args()
    bk = BekenBinary(args.coeffs)
    f: FileIO = args.input
    size = stat(args.input.name).st_size
    start = time()

    if args.action == "encrypt":
        print(f"Encrypting '{f.name}' ({size} bytes)")
        if args.crc:
            print(f" - calculating 32-byte block CRC16...")
            gen = bk.crc(bk.crypt(args.addr, f))
        else:
            print(f" - as raw binary, without CRC16...")
            gen = bk.crypt(args.addr, f)

    if args.action == "decrypt":
        print(f"Decrypting '{f.name}' ({size} bytes)")
        if size % 34 == 0:
            if args.no_crc_check:
                print(f" - has CRC16, skipping checks...")
            else:
                print(f" - has CRC16, checking...")
            gen = bk.crypt(args.addr, bk.uncrc(f, check=not args.no_crc_check))
        elif size % 4 != 0:
            raise ValueError("Input file has invalid length")
        else:
            print(f" - raw binary, no CRC")
            gen = bk.crypt(args.addr, f)

    if args.action == "package":
        print(f"Packaging {args.name} '{f.name}' for memory address 0x{args.addr:X}")
        rbl = RBL(name=args.name, version=args.version)
        if args.name == "bootloader":
            rbl.has_part_table = True
            print(f" - in bootloader mode; partition table unencrypted")
        rbl.container_size = args.size
        print(f" - container size (excl. CRC): 0x{rbl.container_size:X}")
        print(f" - container size (incl. CRC): 0x{rbl.container_size_crc:X}")
        gen = bk.package(f, args.addr, size, rbl)

    if args.action == "unpackage":
        print(f"Unpackaging '{f.name}' (at 0x{args.offset:X}, size 0x{args.size:X})")
        f.seek(args.offset + args.size - 102, SEEK_SET)
        rbl = f.read(102)
        rbl = b"".join(bk.uncrc(rbl))
        rbl = RBL.deserialize(rbl)
        print(f" - found '{rbl.name}' ({rbl.version}), size {rbl.data_size}")
        f.seek(0, SEEK_SET)
        crc_size = (rbl.data_size - 16) // 32 * 34
        gen = bk.crypt(args.addr, bk.uncrc(fileiter(f, 32, 0xFF, crc_size)))

    written = 0
    for data in gen:
        args.output.write(data)
        written += len(data)
    print(f" - wrote {written} bytes in {time()-start:.3f} s")
