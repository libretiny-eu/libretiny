# Copyright (c) Kuba Szczodrzyński 2022-06-02.

import sys
from os.path import dirname, join
from time import time

sys.path.append(join(dirname(__file__), "..", ".."))
sys.path.append(join(dirname(__file__), "..", "uf2ota"))

from argparse import ArgumentParser, FileType

from tools.uf2ota.uf2 import UF2
from tools.upload.ctx import UploadContext

# TODO document this tool

if __name__ == "__main__":
    parser = ArgumentParser("uf2upload", description="UF2 uploader")
    parser.add_argument("file", type=FileType("rb"), help=".uf2 file")

    subp = parser.add_subparsers(dest="protocol", help="Upload protocol", required=True)

    parser_uart = subp.add_parser("uart", help="UART uploader")
    parser_uart.add_argument("port", type=str, help="Serial port device")
    parser_uart.add_argument("-b", "--baud", type=int, help="Serial baudrate")

    args = parser.parse_args()

    uf2 = UF2(args.file)
    if not uf2.read(block_tags=False):
        exit(1)

    ctx = UploadContext(uf2)

    print(
        f"|-- {ctx.fw_name} {ctx.fw_version} @ {ctx.build_date} -> {ctx.board} via {args.protocol}"
    )

    start = time()

    args = dict(args._get_kwargs())
    if uf2.family.code == "ambz":
        from tools.soc.uf2_rtltool import upload

        if not upload(ctx, **args):
            exit(1)
    elif uf2.family.parent_code == "bk72xx":
        from tools.soc.uf2_bk72xx import upload

        if not upload(ctx, **args):
            exit(1)
    else:
        print(f"Unsupported upload family - {uf2.family.name}")
        exit(1)

    duration = time() - start
    print(f"|-- Finished in {duration:.3f} s")
    exit(0)
