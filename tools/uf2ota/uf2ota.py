# Copyright (c) Kuba Szczodrzyński 2022-05-27.

import sys
from os.path import dirname, join

sys.path.append(join(dirname(__file__), "..", ".."))

from argparse import ArgumentParser
from datetime import datetime
from zlib import crc32

from dump import uf2_dump
from models import Input, Tag
from uf2 import UF2
from uf2_block import Block
from utils import binpatch32

from tools.util.platform import get_family

BLOCK_SIZE = 256


def cli():
    parser = ArgumentParser("uf2ota", description="UF2 OTA update format")
    parser.add_argument("action", choices=["info", "dump", "write"])
    parser.add_argument("inputs", nargs="+", type=str)
    parser.add_argument("--output", help="Output .uf2 binary", type=str)
    parser.add_argument("--family", help="Family name", type=str)
    parser.add_argument("--board", help="Board name/code", type=str)
    parser.add_argument("--version", help="LibreTuya core version", type=str)
    parser.add_argument("--fw", help="Firmware name:version", type=str)
    parser.add_argument("--date", help="Build date (Unix, default now)", type=int)
    args = parser.parse_args()

    if args.action == "info":
        with open(args.inputs[0], "rb") as f:
            uf2 = UF2(f)
            if not uf2.read():
                raise RuntimeError("Reading UF2 failed")
        uf2.dump()
        return

    if args.action == "dump":
        input = args.inputs[0]
        outdir = input + "_dump"
        with open(input, "rb") as f:
            uf2 = UF2(f)
            if not uf2.read(block_tags=False):
                raise RuntimeError("Reading UF2 failed")
        uf2_dump(uf2, outdir)
        return

    out = args.output or "out.uf2"
    with open(out, "wb") as f:
        uf2 = UF2(f)

        uf2.family = get_family(args.family)

        # store global tags (for entire file)
        if args.board:
            uf2.put_str(Tag.BOARD, args.board.lower())
            key = f"LibreTuya {args.board.lower()}"
            uf2.put_int32le(Tag.DEVICE_ID, crc32(key.encode()))

        if args.version:
            uf2.put_str(Tag.LT_VERSION, args.version)

        if args.fw:
            if ":" in args.fw:
                (fw_name, fw_ver) = args.fw.split(":")
                uf2.put_str(Tag.FIRMWARE, fw_name)
                uf2.put_str(Tag.VERSION, fw_ver)
            else:
                uf2.put_str(Tag.FIRMWARE, args.fw)

        uf2.put_int8(Tag.OTA_VERSION, 1)
        uf2.put_str(Tag.DEVICE, "LibreTuya")
        uf2.put_int32le(Tag.BUILD_DATE, args.date or int(datetime.now().timestamp()))

        any_ota1 = False
        any_ota2 = False

        for input in args.inputs:
            input = Input(input)

            any_ota1 = any_ota1 or input.has_ota1
            any_ota2 = any_ota2 or input.has_ota2

            # store local tags (for this image only)
            tags = {
                Tag.LT_PART_1: input.ota1_part.encode() if input.has_ota1 else b"",
                Tag.LT_PART_2: input.ota2_part.encode() if input.has_ota2 else b"",
            }

            if input.is_simple:
                # single input image:
                # - same image and partition (2 args)
                # - same image but different partitions (4 args)
                # - only OTA1 image
                # - only OTA2 image
                with open(input.single_file, "rb") as f:
                    data = f.read()
                uf2.store(input.single_offs, data, tags, block_size=BLOCK_SIZE)
                continue

            # different images and partitions for both OTA schemes
            with open(input.ota1_file, "rb") as f:
                data1 = f.read()
            with open(input.ota2_file, "rb") as f:
                data2 = f.read()

            if len(data1) != len(data2):
                raise RuntimeError(
                    f"Images must have same lengths ({len(data1)} vs {len(data2)})"
                )

            for i in range(0, len(data1), 256):
                block1 = data1[i : i + 256]
                block2 = data2[i : i + 256]
                if block1 == block2:
                    # blocks are identical, simply store them
                    uf2.store(
                        input.single_offs + i, block1, tags, block_size=BLOCK_SIZE
                    )
                    tags = {}
                    continue
                # calculate max binpatch length (incl. existing tags and binpatch tag header)
                max_length = 476 - BLOCK_SIZE - Block.get_tags_length(tags) - 4
                # try 32-bit binpatch for best space optimization
                binpatch = binpatch32(block1, block2, bladdr=i)
                if len(binpatch) > max_length:
                    raise RuntimeError(
                        f"Binary patch too long - {len(binpatch)} > {max_length}"
                    )
                tags[Tag.LT_BINPATCH] = binpatch
                uf2.store(input.single_offs + i, block1, tags, block_size=BLOCK_SIZE)
                tags = {}

        uf2.put_int8(Tag.LT_HAS_OTA1, any_ota1 * 1)
        uf2.put_int8(Tag.LT_HAS_OTA2, any_ota2 * 1)
        uf2.write()


if __name__ == "__main__":
    cli()
