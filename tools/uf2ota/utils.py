# Copyright (c) Kuba Szczodrzyński 2022-05-27.


from typing import Dict, List, Tuple

from models import Opcode

from tools.util.intbin import intto8, letoint, sinttole32


def bindiff(
    data1: bytes, data2: bytes, width: int = 1, single: bool = False
) -> Dict[int, Tuple[bytes, bytes]]:
    out: Dict[int, Tuple[bytes, bytes]] = {}
    offs = -1
    diff1 = b""
    diff2 = b""
    for i in range(0, len(data1), width):
        block1 = data1[i : i + width]
        block2 = data2[i : i + width]
        if block1 == block2:
            # blocks are equal again
            if offs != -1:
                # store and reset current difference
                out[offs] = (diff1, diff2)
                offs = -1
                diff1 = b""
                diff2 = b""
            continue
        # blocks still differ
        if single:
            # single block per difference, so just store it
            out[i] = (block1, block2)
        else:
            if offs == -1:
                # difference starts here
                offs = i
            diff1 += block1
            diff2 += block2
    return out


def binpatch32(block1: bytes, block2: bytes, bladdr: int = 0) -> bytes:
    # compare blocks:
    # - in 4 byte (32 bit) chunks
    # - report a single chunk in each difference
    diffs = bindiff(block1, block2, width=4, single=True)
    binpatch: Dict[int, List[int]] = {}

    # gather all repeating differences (i.e. memory offsets for OTA1/OTA2)
    for offs, diff in diffs.items():
        (diff1, diff2) = diff
        diff1 = letoint(diff1)
        diff2 = letoint(diff2)
        diff = diff2 - diff1
        if diff in binpatch:
            # difference already in this binpatch, add the offset
            binpatch[diff].append(offs)
        else:
            # a new difference value
            binpatch[diff] = [offs]
        # print(f"Block at 0x{bladdr:x}+{offs:02x} -> {diff1:08x} - {diff2:08x} = {diff2-diff1:x}")
    # print(f"Block at 0x{bladdr:x}: {len(binpatch)} difference(s) at {sum(len(v) for v in binpatch.values())} offsets")

    # write binary patches
    out = b""
    for diff, offs in binpatch.items():
        out += intto8(Opcode.DIFF32.value)
        out += intto8(len(offs) + 4)
        out += sinttole32(diff)
        out += bytes(offs)
    return out
