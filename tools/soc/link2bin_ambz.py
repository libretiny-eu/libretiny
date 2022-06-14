# Copyright (c) Kuba Szczodrzyński 2022-06-14.

from os.path import basename
from typing import IO, Tuple

from tools.util.fileio import chname, isnewer, readbin
from tools.util.intbin import inttole32
from tools.util.models import Family


def elf2bin_ambz(
    soc,
    family: Family,
    board: dict,
    input: str,
    ota_idx: int = 1,
) -> Tuple[int, str]:
    def write_header(f: IO[bytes], start: int, end: int):
        f.write(b"81958711")
        f.write(inttole32(end - start))
        f.write(inttole32(start))
        f.write(b"\xff" * 16)

    sections_ram = [
        ".ram_image2.entry",
        ".ram_image2.data",
        ".ram_image2.bss",
        ".ram_image2.skb.bss",
        ".ram_heap.data",
    ]
    sections_xip = [".xip_image2.text"]
    sections_rdp = [".ram_rdp.text"]
    nmap = soc.nm(input)
    ram_start = nmap["__ram_image2_text_start__"]
    ram_end = nmap["__ram_image2_text_end__"]
    xip_start = nmap["__flash_text_start__"] - 0x8000020
    # build output name
    output = chname(input, f"image_0x{xip_start:06X}.ota{ota_idx}.bin")
    out_ram = chname(input, f"ota{ota_idx}.ram_2.r.bin")
    out_xip = chname(input, f"ota{ota_idx}.xip_image2.bin")
    out_rdp = chname(input, f"ota{ota_idx}.rdp.bin")
    # print graph element
    print(f"|   |-- {basename(output)}")
    # objcopy required images
    ram = soc.objcopy(input, out_ram, sections_ram)
    xip = soc.objcopy(input, out_xip, sections_xip)
    soc.objcopy(input, out_rdp, sections_rdp)
    # return if images are up to date
    if not isnewer(ram, output) and not isnewer(xip, output):
        return (xip_start, output)

    # read and trim RAM image
    ram = readbin(ram).rstrip(b"\x00")
    # read XIP image
    xip = readbin(xip)
    # align images to 4 bytes
    ram += b"\x00" * (((((len(ram) - 1) // 4) + 1) * 4) - len(ram))
    xip += b"\x00" * (((((len(xip) - 1) // 4) + 1) * 4) - len(xip))
    # write output file
    with open(output, "wb") as f:
        # write XIP header
        write_header(f, 0, len(xip))
        # write XIP image
        f.write(xip)
        # write RAM header
        write_header(f, ram_start, ram_end)
        # write RAM image
        f.write(ram)
    return (xip_start, output)
