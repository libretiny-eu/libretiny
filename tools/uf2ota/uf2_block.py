# Copyright (c) Kuba Szczodrzyński 2022-05-27.

from math import ceil
from typing import Dict

from models import Flags, Tag

from tools.util.intbin import align_up, intto8, inttole24, inttole32, letoint
from tools.util.models import Family
from tools.util.platform import get_family


class Block:
    flags: Flags

    address: int = 0
    length: int = 0

    block_seq: int = 0
    block_count: int = 0

    file_size: int = 0
    family: Family

    data: bytes = None
    md5_data: bytes = None
    tags: Dict[Tag, bytes] = {}

    def __init__(self, family: Family = None) -> None:
        self.flags = Flags()
        self.family = family
        self.flags.has_family_id = not not self.family

    def encode(self) -> bytes:
        self.flags.has_tags = not not self.tags
        # UF2 magic 1 and 2
        data = b"\x55\x46\x32\x0A\x57\x51\x5D\x9E"
        # encode integer variables
        data += inttole32(self.flags.encode())
        data += inttole32(self.address)
        data += inttole32(self.length)
        data += inttole32(self.block_seq)
        data += inttole32(self.block_count)
        if self.flags.file_container:
            data += inttole32(self.file_size)
        elif self.flags.has_family_id:
            data += inttole32(self.family.id)
        else:
            data += b"\x00\x00\x00\x00"
        if not self.data:
            self.data = b""
        # append tags
        tags = b""
        if self.flags.has_tags:
            for k, v in self.tags.items():
                tag_size = 4 + len(v)
                tags += intto8(tag_size)
                tags += inttole24(k.value)
                tags += v
                tag_size %= 4
                if tag_size:
                    tags += b"\x00" * (4 - tag_size)
        # append block data with padding
        data += self.data
        data += tags
        data += b"\x00" * (476 - len(self.data) - len(tags))
        data += b"\x30\x6F\xB1\x0A"  # magic 3
        return data

    def decode(self, data: bytes) -> bool:
        # check block size
        if len(data) != 512:
            print(f"Invalid block size ({len(data)})")
            return False
        # check Magic 1
        if letoint(data[0:4]) != 0x0A324655:
            print(f"Invalid Magic 1 ({data[0:4]})")
            return False
        # check Magic 2
        if letoint(data[4:8]) != 0x9E5D5157:
            print(f"Invalid Magic 2 ({data[4:8]})")
            return False
        # check Magic 3
        if letoint(data[508:512]) != 0x0AB16F30:
            print(f"Invalid Magic 13({data[508:512]})")
            return False

        self.flags.decode(letoint(data[8:12]))
        self.address = letoint(data[12:16])
        self.length = letoint(data[16:20])
        self.block_seq = letoint(data[20:24])
        self.block_count = letoint(data[24:28])
        if self.flags.file_container:
            self.file_size = letoint(data[28:32])
        if self.flags.has_family_id:
            self.family = get_family(id=letoint(data[28:32]))

        if self.flags.has_md5:
            self.md5_data = data[484:508]  # last 24 bytes of data[]

        # decode tags
        self.tags = {}
        if self.flags.has_tags:
            tags = data[32 + self.length :]
            i = 0
            while i < len(tags):
                length = tags[i]
                if not length:
                    break
                tag_type = letoint(tags[i + 1 : i + 4])
                tag_data = tags[i + 4 : i + length]
                self.tags[Tag(tag_type)] = tag_data
                i += length
                i = int(ceil(i / 4) * 4)

        self.data = data[32 : 32 + self.length]
        return True

    @staticmethod
    def get_tags_length(tags: Dict[Tag, bytes]) -> int:
        out = 0
        # add tag headers
        out += 4 * len(tags)
        # add all tag lengths, padded to 4 bytes
        out += sum(align_up(l, 4) for l in map(len, tags.values()))
        # add final 0x00 tag
        out += 4
        return out

    def __str__(self) -> str:
        flags = self.flags
        address = hex(self.address)
        length = hex(self.length)
        block_seq = self.block_seq
        block_count = self.block_count
        file_size = self.file_size
        family = self.family.short_name
        tags = [(k.name, v) for k, v in self.tags.items()]
        return f"Block[{block_seq}/{block_count}](flags={flags}, address={address}, length={length}, file_size={file_size}, family={family}, tags={tags})"
