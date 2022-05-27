# Copyright (c) Kuba Szczodrzyński 2022-05-27.

from argparse import ArgumentParser
from zlib import crc32

from models import Family, Tag
from uf2 import UF2


class Input:
    ota1_part: str = None
    ota1_offs: int = 0
    ota1_file: str = None
    ota2_part: str = None
    ota2_offs: int = 0
    ota2_file: str = None

    def __init__(self, input: str) -> None:
        input = input.split(":")
        n = len(input)
        if n not in [2, 4]:
            print("Incorrect input format - should be part+offs:file[:part+offs:file]")
            exit()
        # just spread the same image twice for single-OTA scheme
        if n == 2:
            input += input

        if input[0] and input[1]:
            if "+" in input[0]:
                (self.ota1_part, self.ota1_offs) = input[0].split("+")
                self.ota1_offs = int(self.ota1_offs, 16)
            else:
                self.ota1_part = input[0]
            self.ota1_file = input[1]
        if input[2] and input[3]:
            if "+" in input[2]:
                (self.ota2_part, self.ota2_offs) = input[2].split("+")
                self.ota2_offs = int(self.ota2_offs, 16)
            else:
                self.ota2_part = input[2]
            self.ota2_file = input[3]

        if self.is_simple and self.ota1_offs != self.ota2_offs:
            # currently, offsets cannot differ when storing one image only
            # (this would require to actually store it twice)
            print(
                f"Offsets cannot differ in single-image/two-partition scheme ({self.ota1_file})"
            )
            exit()

    @property
    def is_single(self) -> bool:
        return self.ota1_part == self.ota2_part and self.ota1_file == self.ota2_file

    @property
    def single_part(self) -> str:
        return self.ota1_part or self.ota2_part

    @property
    def single_offs(self) -> int:
        return self.ota1_offs or self.ota2_offs

    @property
    def single_file(self) -> str:
        return self.ota1_file or self.ota2_file

    @property
    def has_ota1(self) -> bool:
        return not not (self.ota1_part and self.ota1_file)

    @property
    def has_ota2(self) -> bool:
        return not not (self.ota2_part and self.ota2_file)

    @property
    def is_simple(self) -> bool:
        return self.ota1_file == self.ota2_file or not (self.has_ota1 and self.has_ota2)


def cli():
    parser = ArgumentParser("uf2ota", description="UF2 OTA update format")
    parser.add_argument("action", choices=["dump", "write"])
    parser.add_argument("inputs", nargs="+", type=str)
    parser.add_argument("--output", help="Output .uf2 binary", type=str)
    parser.add_argument("--family", help="Family name", type=str)
    parser.add_argument("--board", help="Board name/code", type=str)
    parser.add_argument("--version", help="LibreTuya core version", type=str)
    parser.add_argument("--fw", help="Firmware name:version", type=str)
    args = parser.parse_args()

    if args.action == "dump":
        with open(args.inputs[0], "rb") as f:
            uf2 = UF2(f)
            if uf2.read():
                uf2.dump()
        return

    out = args.output or "out.uf2"
    with open(out, "wb") as f:
        uf2 = UF2(f)

        try:
            uf2.family = next(f for f in Family if f.name == args.family)
        except:
            families = ", ".join(f.name for f in Family)[9:]
            print(f"Invalid family name - should be one of {families}")
            return

        # store global tags (for entire file)
        if not args.board:
            print("Missing board name (--board)")
            return
        uf2.put_str(Tag.BOARD, args.board.lower())

        if not args.version:
            print("Missing LT version (--version)")
            return
        uf2.put_str(Tag.LT_VERSION, args.version)

        if args.fw:
            (fw_name, fw_ver) = args.fw.split(":")
            uf2.put_str(Tag.FIRMWARE, fw_name)
            uf2.put_str(Tag.VERSION, fw_ver)

        uf2.put_str(Tag.DEVICE, "LibreTuya")
        key = f"LibreTuya {args.board.lower()}"
        uf2.put_int32le(Tag.DEVICE_ID, crc32(key.encode()))

        any_ota1 = False
        any_ota2 = False

        for input in args.inputs:
            input = Input(input)

            any_ota1 = any_ota1 or input.has_ota1
            any_ota2 = any_ota2 or input.has_ota2

            if input.is_simple:
                # single input image:
                # - same image and partition (2 args)
                # - same image but different partitions (4 args)
                # - only OTA1 image
                # - only OTA2 image
                with open(input.single_file, "rb") as f:
                    data = f.read()
                # store local tags (for this image only)
                tags = {}
                tags[Tag.LT_PART_1] = (
                    input.ota1_part.encode() if input.has_ota1 else b""
                )
                tags[Tag.LT_PART_2] = (
                    input.ota2_part.encode() if input.has_ota2 else b""
                )
                uf2.store(input.single_offs, data, tags)
                continue

            # different images and partitions for both OTA schemes
            raise NotImplementedError("Image binary patching is not yet implemented")

        uf2.put_int8(Tag.LT_HAS_OTA1, any_ota1 * 1)
        uf2.put_int8(Tag.LT_HAS_OTA2, any_ota2 * 1)
        uf2.write()


if __name__ == "__main__":
    cli()
