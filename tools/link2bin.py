# Copyright (c) Kuba Szczodrzyński 2022-05-31.

import sys
from os.path import dirname, join

sys.path.append(join(dirname(__file__), ".."))

import shlex
from argparse import ArgumentParser
from enum import Enum
from os import stat, unlink
from os.path import basename, dirname, isfile, join
from shutil import copyfile
from subprocess import PIPE, Popen
from typing import IO, Dict, List, Tuple

from tools.util.fileio import chext, isnewer, readtext
from tools.util.models import Family
from tools.util.obj import get
from tools.util.platform import get_board_manifest, get_family


class SocType(Enum):
    UNSET = ()
    # (index, toolchain prefix, has dual-OTA, argument count)
    AMBZ = (1, "arm-none-eabi-", True, 0)
    BK72XX = (2, "arm-none-eabi-", False, 0)

    def cmd(self, program: str, args: List[str] = []) -> IO[bytes]:
        program = self.prefix + program
        cmd = [program] + args
        try:
            process = Popen(cmd, stdout=PIPE)
        except FileNotFoundError:
            if isinstance(cmd, list):
                cmd = " ".join(cmd)
            print(f"Toolchain not found while running: '{cmd}'")
            exit(1)
        return process.stdout

    @property
    def prefix(self) -> str:
        return self.value[1]

    @property
    def dual_ota(self) -> bool:
        return self.value[2]

    @property
    def soc_argc(self) -> int:
        return self.value[3]

    def nm(self, input: str) -> Dict[str, int]:
        out = {}
        stdout = self.cmd("gcc-nm", [input])
        for line in stdout.readlines():
            line = line.decode().strip().split(" ")
            if len(line) != 3:
                continue
            out[line[2]] = int(line[0], 16)
        return out

    def objcopy(
        self,
        input: str,
        output: str,
        sections: List[str] = [],
        fmt: str = "binary",
    ) -> str:
        # print graph element
        print(f"|   |   |-- {basename(output)}")
        if isnewer(input, output):
            args = []
            for section in sections:
                args += ["-j", section]
            args += ["-O", fmt]
            args += [input, output]
            self.cmd("objcopy", args).read()
        return output


#   _    _ _   _ _ _ _   _
#  | |  | | | (_) (_) | (_)
#  | |  | | |_ _| |_| |_ _  ___  ___
#  | |  | | __| | | | __| |/ _ \/ __|
#  | |__| | |_| | | | |_| |  __/\__ \
#   \____/ \__|_|_|_|\__|_|\___||___/
def checkfile(path: str):
    if not isfile(path) or stat(path).st_size == 0:
        print(f"Generated file not found: {path}")
        exit(1)


#   ______ _      ______   _          ____ _____ _   _
#  |  ____| |    |  ____| | |        |  _ \_   _| \ | |
#  | |__  | |    | |__    | |_ ___   | |_) || | |  \| |
#  |  __| | |    |  __|   | __/ _ \  |  _ < | | | . ` |
#  | |____| |____| |      | || (_) | | |_) || |_| |\  |
#  |______|______|_|       \__\___/  |____/_____|_| \_|
def elf2bin(
    soc: SocType,
    family: Family,
    board: dict,
    input: str,
    ota_idx: int = 1,
    args: List[str] = [],
) -> Tuple[int, str]:
    checkfile(input)
    func = None

    if soc == SocType.AMBZ:
        from tools.soc.link2bin_ambz import elf2bin_ambz

        func = elf2bin_ambz
    elif soc == SocType.BK72XX:
        from tools.soc.link2bin_bk72xx import elf2bin_bk72xx

        func = elf2bin_bk72xx

    if func:
        return func(soc, family, board, input, ota_idx, *args)
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
        if ".elf" in arg:
            if not ld_ota1:
                # single-OTA chip, return the output name
                return [(arg, args)]
            # append OTA index in filename
            args1[i] = elf1 = chext(arg, "ota1.elf")
            args2[i] = elf2 = chext(arg, "ota2.elf")
        if arg.endswith(".ld") and ld_ota1:
            # use OTA2 linker script
            args2[i] = arg.replace(ld_ota1, ld_ota2)
    if not elf1 or not elf2:
        print("Linker output .elf not found in arguments")
        return None
    return [(elf1, args1), (elf2, args2)]


def link2bin(
    soc: SocType,
    family: Family,
    board: dict,
    ld_args: List[str],
    ld_ota1: str = None,
    ld_ota2: str = None,
    soc_args: List[str] = [],
) -> List[str]:
    elfs = []
    if soc.dual_ota:
        # process linker arguments for dual-OTA chips
        elfs = ldargs_parse(ld_args, ld_ota1, ld_ota2)
    else:
        # just get .elf output name for single-OTA chips
        elfs = ldargs_parse(ld_args, None, None)

    if not elfs:
        return None

    ota_idx = 1
    for elf, ldargs in elfs:
        # print graph element
        print(f"|-- Image {ota_idx}: {basename(elf)}")
        if isfile(elf):
            unlink(elf)
        soc.cmd(f"gcc", args=ldargs).read()
        checkfile(elf)
        # generate a set of binaries for the SoC
        elf2bin(soc, family, board, elf, ota_idx, soc_args)
        ota_idx += 1

    if soc.dual_ota:
        # copy OTA1 file as firmware.elf to make PIO understand it
        elf, _ = ldargs_parse(ld_args, None, None)[0]
        copyfile(elfs[0][0], elf)


if __name__ == "__main__":
    parser = ArgumentParser(
        prog="link2bin",
        description="Link to BIN format",
        prefix_chars="#",
    )
    parser.add_argument("board", type=str, help="Target board name")
    parser.add_argument("ota1", type=str, help=".LD file OTA1 pattern")
    parser.add_argument("ota2", type=str, help=".LD file OTA2 pattern")
    parser.add_argument("args", type=str, nargs="*", help="SoC+linker arguments")
    args = parser.parse_args()

    try:
        board = get_board_manifest(args.board)
    except FileNotFoundError:
        print(f"Board not found: {args.board}")
        exit(1)

    family = get_family(short_name=get(board, "build.family"))
    soc_types = {soc.name.lower(): soc for soc in SocType}
    soc = soc_types.get(family.code, soc_types.get(family.parent_code, None))
    if not soc:
        print(f"SoC type not found. Tried {family.code}, {family.parent_code}")
        exit(1)

    if not args.args:
        print(f"Linker arguments must not be empty")
        exit(1)

    try:
        while True:
            i = next(i for i, a in enumerate(args.args) if a.startswith("@"))
            arg = args.args.pop(i)
            argv = readtext(arg[1:])
            argv = shlex.split(argv)
            args.args = args.args[0:i] + argv + args.args[i:]
    except StopIteration:
        pass

    link2bin(
        soc,
        family,
        board,
        args.args[soc.soc_argc :],
        args.ota1,
        args.ota2,
        args.args[: soc.soc_argc],
    )
