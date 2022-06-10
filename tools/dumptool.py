# Copyright 2022-04-24 kuba2k2

import sys
from os.path import dirname, join

sys.path.append(join(dirname(__file__), ".."))

from argparse import ArgumentParser, FileType
from binascii import crc32
from os import makedirs
from os.path import basename, dirname, join

from tools.util.crc16 import CRC16
from tools.util.platform import get_board_manifest

if __name__ == "__main__":
    parser = ArgumentParser("dumptool", description="Convert flash dump images")
    parser.add_argument("action", choices=["split"])
    parser.add_argument("board", help="Board name or board.json file", type=str)
    parser.add_argument("file", help="Input file", type=FileType("rb"))
    parser.add_argument("--output", "-o", help="Output directory", type=str)
    parser.add_argument("--no-trim", "-n", help="Don't trim output binaries")
    parser.add_argument("--no-checksum", "-c", help="Don't append checksum to filename")
    args = parser.parse_args()

    board = get_board_manifest(args.board)

    if "flash" not in board:
        print("Flash layout not defined")
        exit()
    flash_layout = board["flash"]

    output = join(dirname(args.file.name), basename(args.file.name) + ".split")
    output = args.output or output
    makedirs(output, exist_ok=True)

    with args.file as f:
        dump = f.read()

    for name, layout in flash_layout.items():
        (offset, _, length) = layout.partition("+")
        offset = int(offset, 16)
        length = int(length, 16)

        if offset + length > len(dump):
            print(f"Partition '{name}' is out of bounds!")
            print(f" - Dump size: {hex(len(dump))}")
            print(f" - Partition start: {hex(offset)}")
            print(f" - Partition length: {hex(length)}")
            print(f" - Partition end: {hex(offset+length)}")
            exit()

        part = dump[offset : offset + length]
        offset = offset.to_bytes(length=3, byteorder="big").hex().upper()
        filename = f"{offset}_{name}.bin"

        if not args.no_trim:
            part = part.rstrip(b"\xff")
        if not args.no_checksum:
            if length > 0x10000:  # 64 KiB
                cs = crc32(part)
                cs = cs.to_bytes(length=4, byteorder="big")
            else:
                cs = CRC16.ARC.calc(part)
                cs = cs.to_bytes(length=2, byteorder="big")
            filename = f"{offset}_{name}_{cs.hex().upper()}.bin"
        print(f"Writing {filename}")
        with open(join(output, filename), "wb") as f:
            f.write(part)
