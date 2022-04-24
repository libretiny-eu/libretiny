# Copyright 2022-04-24 kuba2k2

import json
from argparse import ArgumentParser, FileType
from binascii import crc32
from os import makedirs
from os.path import basename, dirname, isfile, join


def crc16(data):
    # https://gist.github.com/pintoXD/a90e398bba5a1b6c121de4e1265d9a29
    crc = 0x0000
    for b in data:
        crc ^= b
        for j in range(0, 8):
            if (crc & 0x0001) > 0:
                crc = (crc >> 1) ^ 0xA001
            else:
                crc = crc >> 1
    return crc


if __name__ == "__main__":
    parser = ArgumentParser("dumptool", description="Convert flash dump images")
    parser.add_argument("action", choices=["split"])
    parser.add_argument("board", help="Board name or board.json file", type=str)
    parser.add_argument("file", help="Input file", type=FileType("rb"))
    parser.add_argument("--output", "-o", help="Output directory", type=str)
    parser.add_argument("--no-trim", "-n", help="Don't trim output binaries")
    parser.add_argument("--no-checksum", "-c", help="Don't append checksum to filename")
    args = parser.parse_args()

    if isfile(args.board):
        board = args.board
    else:
        board = join(dirname(__file__), "..", "boards", f"{args.board}.json")
    if not isfile(board):
        print("Board not found")
        exit()

    with open(board, "r") as f:
        data = json.load(f)

    if "flash" not in data:
        print("Flash layout not defined")
        exit()
    flash_layout = data["flash"]

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
                cs = crc16(part)
                cs = cs.to_bytes(length=2, byteorder="big")
            filename = f"{offset}_{name}_{cs.hex().upper()}.bin"
        print(f"Writing {filename}")
        with open(join(output, filename), "wb") as f:
            f.write(part)
