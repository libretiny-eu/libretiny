# Copyright (c) Kuba Szczodrzyński 2022-05-28.


from io import BytesIO, FileIO
from os import makedirs
from os.path import join
from typing import Dict, Tuple

from models import Opcode, Tag
from uf2 import UF2

from tools.util.intbin import inttole32, letoint, letosint

fs: Dict[str, Tuple[int, FileIO]] = {}
output_dir = ""
output_basename = ""
part1 = ""
part2 = ""


def write(part: str, offs: int, data: bytes):
    global fs

    if part not in fs or fs[part][0] != offs:
        path = join(output_dir, output_basename + part + f"_0x{offs:x}.bin")
        f = open(path, "wb")
        if part in fs:
            fs[part][1].close()
    else:
        f = fs[part][1]
    fs[part] = (offs + f.write(data), f)


def update_parts(tags: Dict[Tag, bytes]):
    global part1, part2
    if Tag.LT_PART_1 in tags:
        part1 = tags[Tag.LT_PART_1].decode()
        part1 = ("1_" + part1) if part1 else None
    if Tag.LT_PART_2 in tags:
        part2 = tags[Tag.LT_PART_2].decode()
        part2 = ("2_" + part2) if part2 else None


def uf2_dump(uf2: UF2, outdir: str):
    global output_dir, output_basename

    makedirs(outdir, exist_ok=True)
    if Tag.LT_VERSION not in uf2.tags:
        raise RuntimeError("Can only dump LibreTuya firmware images")

    output_dir = outdir
    output_basename = "_".join(
        filter(
            None,
            [
                uf2.tags.get(Tag.FIRMWARE, b"").decode(),
                uf2.tags.get(Tag.VERSION, b"").decode(),
                "lt" + uf2.tags[Tag.LT_VERSION].decode(),
                uf2.tags.get(Tag.BOARD, b"").decode(),
            ],
        )
    )
    output_basename += "_"

    update_parts(uf2.tags)
    for block in uf2.data:
        # update target partition info
        update_parts(block.tags)
        # skip empty blocks
        if not block.length:
            continue

        data1 = block.data if part1 else None
        data2 = block.data if part2 else None

        if Tag.LT_BINPATCH in block.tags:
            # type 5, 6
            data2 = bytearray(data2)
            tag = block.tags[Tag.LT_BINPATCH]
            binpatch = BytesIO(tag)
            while binpatch.tell() < len(tag):
                opcode = Opcode(binpatch.read(1)[0])
                length = binpatch.read(1)[0]
                data = binpatch.read(length)
                if opcode == Opcode.DIFF32:
                    value = letosint(data[0:4])
                    for offs in data[4:]:
                        chunk = data2[offs : offs + 4]
                        chunk = letoint(chunk)
                        chunk += value
                        chunk = inttole32(chunk)
                        data2[offs : offs + 4] = chunk
            data2 = bytes(data2)

        if data1:
            # types 1, 3, 4
            write(part1, block.address, data1)
        if data2:
            # types 2, 3, 4
            write(part2, block.address, data2)
