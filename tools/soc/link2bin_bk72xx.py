# Copyright (c) Kuba Szczodrzyński 2022-06-14.

from datetime import datetime
from os import stat
from os.path import basename
from typing import Tuple

from tools.util.bkutil import RBL, BekenBinary
from tools.util.fileio import chext, chname, isnewer, writebin, writejson
from tools.util.models import Family
from tools.util.obj import get


def calc_offset(addr: int) -> int:
    return int(addr + (addr // 32) * 2)


def elf2bin_bk72xx(
    soc,
    family: Family,
    board: dict,
    input: str,
    ota_idx: int = 1,
) -> Tuple[int, str]:
    mcu = get(board, "build.mcu")
    coeffs = get(board, "build.bkcrypt_coeffs") or ("0" * 32)
    rbl_size = get(board, "build.bkrbl_size_app")
    version = datetime.now().strftime("%y.%m.%d")

    nmap = soc.nm(input)
    app_addr = nmap["_vector_start"]
    app_offs = calc_offset(app_addr)
    app_size = int(rbl_size, 16)

    # build output name
    output = chname(input, f"{mcu}_app_0x{app_offs:06X}.bin")
    fw_bin = chext(input, "bin")
    # print graph element
    print(f"|   |-- {basename(output)}")
    # objcopy ELF -> raw BIN
    soc.objcopy(input, fw_bin)
    # return if images are up to date
    if not isnewer(fw_bin, output):
        return (app_offs, output)

    bk = BekenBinary(coeffs)
    rbl = RBL(
        name="app",
        version=f"{version}-{mcu}",
        container_size=app_size,
    )

    fw_size = stat(fw_bin).st_size
    raw = open(fw_bin, "rb")
    out = open(output, "wb")
    for data in bk.package(raw, app_addr, fw_size, rbl):
        out.write(data)
