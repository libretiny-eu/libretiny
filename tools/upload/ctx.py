# Copyright (c) Kuba Szczodrzyński 2022-06-02.

from datetime import datetime
from io import BytesIO
from typing import Dict, Tuple

from tools.uf2ota.models import Tag
from tools.uf2ota.uf2 import UF2
from tools.upload.binpatch import binpatch_apply
from tools.util.intbin import letoint
from tools.util.obj import get
from tools.util.platform import get_board_manifest


class UploadContext:

    uf2: UF2

    seq: int = 0

    part1: str = None
    part2: str = None

    has_ota1: bool
    has_ota2: bool

    board_manifest: dict = None

    def __init__(self, uf2: UF2) -> None:
        self.uf2 = uf2
        self.has_ota1 = uf2.tags.get(Tag.LT_HAS_OTA1, None) == b"\x01"
        self.has_ota2 = uf2.tags.get(Tag.LT_HAS_OTA2, None) == b"\x01"

    @property
    def fw_name(self) -> str:
        return self.uf2.tags.get(Tag.FIRMWARE, b"").decode()

    @property
    def fw_version(self) -> str:
        return self.uf2.tags.get(Tag.VERSION, b"").decode()

    @property
    def lt_version(self) -> str:
        return self.uf2.tags.get(Tag.LT_VERSION, b"").decode()

    @property
    def board(self) -> str:
        return self.uf2.tags.get(Tag.BOARD, b"").decode()

    @property
    def build_date(self) -> datetime:
        if Tag.BUILD_DATE not in self.uf2.tags:
            return None
        return datetime.fromtimestamp(letoint(self.uf2.tags[Tag.BUILD_DATE]))

    @property
    def baudrate(self) -> int:
        if not self.board_manifest:
            self.board_manifest = get_board_manifest(self.board)
        return get(self.board_manifest, "upload.speed")

    def get_offset(self, part: str, offs: int) -> int:
        if not self.board_manifest:
            self.board_manifest = get_board_manifest(self.board)
        part = get(self.board_manifest, f"flash.{part}")
        (offset, length) = map(lambda x: int(x, 16), part.split("+"))
        if offs >= length:
            return None
        return offset + offs

    def read(self, ota_idx: int = 1) -> Tuple[str, int, bytes]:
        """Read next available data block for the specified OTA scheme.

        Returns:
            Tuple[str, int, bytes]: target partition, relative offset, data block
        """

        if ota_idx not in [1, 2]:
            print(f"Invalid OTA index - {ota_idx}")
            return None

        if ota_idx == 1 and not self.has_ota1:
            print(f"No data for OTA index - {ota_idx}")
            return None
        if ota_idx == 2 and not self.has_ota2:
            print(f"No data for OTA index - {ota_idx}")
            return None

        for _ in range(self.seq, len(self.uf2.data)):
            block = self.uf2.data[self.seq]
            self.seq += 1

            part1 = block.tags.get(Tag.LT_PART_1, None)
            part2 = block.tags.get(Tag.LT_PART_2, None)

            if part1 is not None and part2 is not None:
                # decode empty tags too
                self.part1 = part1.decode()
                self.part2 = part2.decode()
            elif part1 or part2:
                print(f"Only one target partition specified - {part1} / {part2}")
                return None

            if not block.data:
                continue

            part = None
            if ota_idx == 1:
                part = self.part1
            elif ota_idx == 2:
                part = self.part2
            if not part:
                continue

            # got data and target partition
            offs = block.address
            data = block.data

            if ota_idx == 2 and Tag.LT_BINPATCH in block.tags:
                binpatch = block.tags[Tag.LT_BINPATCH]
                data = bytearray(data)
                data = binpatch_apply(data, binpatch)
                data = bytes(data)

            return (part, offs, data)
        return (None, 0, None)

    def collect(self, ota_idx: int = 1) -> Dict[int, BytesIO]:
        """Read all UF2 blocks. Gather continuous data parts into sections
        and their flashing offsets.

        Returns:
            Dict[int, BytesIO]: map of flash offsets to streams with data
        """

        out: Dict[int, BytesIO] = {}
        while True:
            ret = self.read(ota_idx)
            if not ret:
                return False
            (part, offs, data) = ret
            if not data:
                break
            offs = self.get_offset(part, offs)
            if offs is None:
                return False

            # find BytesIO in the dict
            for io_offs, io_data in out.items():
                if io_offs + len(io_data.getvalue()) == offs:
                    io_data.write(data)
                    offs = 0
                    break
            if offs == 0:
                continue

            # create BytesIO at specified offset
            io = BytesIO()
            io.write(data)
            out[offs] = io
        # rewind BytesIO back to start
        for io in out.values():
            io.seek(0)
        return out
