# Copyright (c) Kuba Szczodrzyński 2022-06-02.

from io import BytesIO

from tools.uf2ota.models import Opcode
from tools.util.intbin import inttole32, letoint, letosint


def binpatch_diff32(data: bytearray, patch: bytes) -> bytearray:
    diff = letosint(patch[0:4])
    for offs in patch[4:]:
        value = letoint(data[offs : offs + 4])
        value += diff
        data[offs : offs + 4] = inttole32(value)
    return data


def binpatch_apply(data: bytearray, binpatch: bytes) -> bytearray:
    io = BytesIO(binpatch)
    while io.tell() < len(binpatch):
        opcode = io.read(1)[0]
        length = io.read(1)[0]
        bpdata = io.read(length)
        if opcode == Opcode.DIFF32:
            data = binpatch_diff32(data, bpdata)
    return data
