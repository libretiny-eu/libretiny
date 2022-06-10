# Copyright (c) Kuba Szczodrzyński 2022-05-31.

import sys
from os.path import dirname, join

sys.path.append(join(dirname(__file__), ".."))

from argparse import ArgumentParser
from enum import Enum
from os import stat, unlink
from os.path import basename, dirname, isfile, join
from shutil import copyfile
from subprocess import PIPE, Popen
from typing import IO, Dict, List, Tuple

from tools.util.fileio import chext, chname, isnewer, readbin
from tools.util.intbin import inttole32


class SocType(Enum):
    UNSET = ()
    AMBZ = (1, "arm-none-eabi-", True)

    def cmd(self, cmd: str) -> IO[bytes]:
        try:
            process = Popen(self.value[1] + cmd, stdout=PIPE)
        except FileNotFoundError:
            print(f"Toolchain not found while running: '{self.value[1] + cmd}'")
            exit(1)
        return process.stdout

    @property
    def dual_ota(self):
        return self.value[2]


soc: "SocType" = SocType.UNSET

#   _    _ _   _ _ _ _   _
#  | |  | | | (_) (_) | (_)
#  | |  | | |_ _| |_| |_ _  ___  ___
#  | |  | | __| | | | __| |/ _ \/ __|
#  | |__| | |_| | | | |_| |  __/\__ \
#   \____/ \__|_|_|_|\__|_|\___||___/
def checkfile(path: str):
    if not isfile(path) or stat(path).st_size == 0:
        exit(1)


#   ____  _             _   _ _
#  |  _ \(_)           | | (_) |
#  | |_) |_ _ __  _   _| |_ _| |___
#  |  _ <| | '_ \| | | | __| | / __|
#  | |_) | | | | | |_| | |_| | \__ \
#  |____/|_|_| |_|\__,_|\__|_|_|___/
def nm(input: str) -> Dict[str, int]:
    out = {}
    stdout = soc.cmd(f"gcc-nm {input}")
    for line in stdout.readlines():
        line = line.decode().strip().split(" ")
        if len(line) != 3:
            continue
        out[line[2]] = int(line[0], 16)
    return out


def objcopy(
    input: str,
    output: str,
    sections: List[str],
    fmt: str = "binary",
) -> str:
    # print graph element
    print(f"|   |   |-- {basename(output)}")
    if isnewer(input, output):
        sections = " ".join(f"-j {section}" for section in sections)
        soc.cmd(f"objcopy {sections} -O {fmt} {input} {output}").read()
    return output


#   ______ _      ______   _          ____ _____ _   _
#  |  ____| |    |  ____| | |        |  _ \_   _| \ | |
#  | |__  | |    | |__    | |_ ___   | |_) || | |  \| |
#  |  __| | |    |  __|   | __/ _ \  |  _ < | | | . ` |
#  | |____| |____| |      | || (_) | | |_) || |_| |\  |
#  |______|______|_|       \__\___/  |____/_____|_| \_|
def elf2bin_ambz(input: str, ota_idx: int = 1) -> Tuple[int, str]:
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
    nmap = nm(input)
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
    ram = objcopy(input, out_ram, sections_ram)
    xip = objcopy(input, out_xip, sections_xip)
    objcopy(input, out_rdp, sections_rdp)
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


def elf2bin(input: str, ota_idx: int = 1) -> Tuple[int, str]:
    checkfile(input)
    if soc == SocType.AMBZ:
        return elf2bin_ambz(input, ota_idx)
    raise NotImplementedError(f"SoC ELF->BIN not implemented: {soc}")


#   _      _       _
#  | |    (_)     | |
#  | |     _ _ __ | | _____ _ __
#  | |    | | '_ \| |/ / _ \ '__|
#  | |____| | | | |   <  __/ |
#  |______|_|_| |_|_|\_\___|_|
def ldargs_parse(
    args: List[str],
    ld_ota1: str,
    ld_ota2: str,
) -> List[Tuple[str, List[str]]]:
    args1 = list(args)
    args2 = list(args)
    elf1 = elf2 = None
    for i, arg in enumerate(args):
        arg = arg.strip('"').strip("'")
        if ".elf" in arg:
            if not ld_ota1:
                # single-OTA chip, return the output name
                return [(arg, args)]
            # append OTA index in filename
            elf1 = chext(arg, "ota1.elf")
            elf2 = chext(arg, "ota2.elf")
            args1[i] = '"' + elf1 + '"'
            args2[i] = '"' + elf2 + '"'
        if arg.endswith(".ld") and ld_ota1:
            # use OTA2 linker script
            args2[i] = arg.replace(ld_ota1, ld_ota2)
    if not elf1 or not elf2:
        print("Linker output .elf not found in arguments")
        return None
    return [(elf1, args1), (elf2, args2)]


def link2bin(
    args: List[str],
    ld_ota1: str = None,
    ld_ota2: str = None,
) -> List[str]:
    elfs = []
    if soc.dual_ota:
        # process linker arguments for dual-OTA chips
        elfs = ldargs_parse(args, ld_ota1, ld_ota2)
    else:
        # just get .elf output name for single-OTA chips
        elfs = ldargs_parse(args, None, None)

    if not elfs:
        return None

    ota_idx = 1
    for elf, ldargs in elfs:
        # print graph element
        print(f"|-- Image {ota_idx}: {basename(elf)}")
        if isfile(elf):
            unlink(elf)
        ldargs = " ".join(ldargs)
        soc.cmd(f"gcc {ldargs}").read()
        checkfile(elf)
        # generate a set of binaries for the SoC
        elf2bin(elf, ota_idx)
        ota_idx += 1

    if soc.dual_ota:
        # copy OTA1 file as firmware.elf to make PIO understand it
        elf, _ = ldargs_parse(args, None, None)[0]
        copyfile(elfs[0][0], elf)


if __name__ == "__main__":
    parser = ArgumentParser(
        prog="link2bin",
        description="Link to BIN format",
        prefix_chars="@",
    )
    parser.add_argument("soc", type=str, help="SoC name/family short name")
    parser.add_argument("ota1", type=str, help=".LD file OTA1 pattern")
    parser.add_argument("ota2", type=str, help=".LD file OTA2 pattern")
    parser.add_argument("args", type=str, nargs="*", help="Linker arguments")
    args = parser.parse_args()
    try:
        soc = next(soc for soc in SocType if soc.name == args.soc)
    except StopIteration:
        print(f"Not a valid SoC: {args.soc}")
        exit(1)
    if not args.args:
        print(f"Linker arguments must not be empty")
        exit(1)
    link2bin(args.args, args.ota1, args.ota2)
