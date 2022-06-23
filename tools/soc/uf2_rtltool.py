# Copyright (c) Kuba Szczodrzyński 2022-06-02.

from io import BytesIO

from tools.upload.ctx import UploadContext
from tools.upload.rtltool import RTLXMD
from tools.util.intbin import letoint


def upload_uart(
    ctx: UploadContext,
    port: str,
    baud: int = None,
    **kwargs,
) -> bool:
    prefix = "|   |--"
    rtl = RTLXMD(port=port)
    print(prefix, f"Connecting to {port}...")
    if not rtl.connect():
        print(prefix, f"Failed to connect on port {port}")
        return False

    # read system data to get active OTA index
    io = BytesIO()
    if not rtl.ReadBlockFlash(io, offset=0x9000, size=256):
        print(prefix, "Failed to read from 0x9000")
        return False
    # get as bytes
    system = io.getvalue()
    if len(system) != 256:
        print(prefix, f"Length invalid while reading from 0x9000 - {len(system)}")
        return False
    # read OTA switch value
    ota_switch = bin(letoint(system[4:8]))[2:]
    # count 0-bits
    ota_idx = 1 + (ota_switch.count("0") % 2)
    # validate OTA2 address in system data
    if ota_idx == 2:
        ota2_addr = letoint(system[0:4]) & 0xFFFFFF
        part_addr = ctx.get_offset("ota2", 0)
        if ota2_addr != part_addr:
            print(
                prefix,
                f"Invalid OTA2 address on chip - found {ota2_addr}, expected {part_addr}",
            )
            return False

    print(prefix, f"Flashing image to OTA {ota_idx}...")
    # collect continuous blocks of data
    parts = ctx.collect(ota_idx=ota_idx)
    # write blocks to flash
    for offs, data in parts.items():
        offs |= 0x8000000
        length = len(data.getvalue())
        data.seek(0)
        print(prefix, f"Writing {length} bytes to 0x{offs:06x}")
        if not rtl.WriteBlockFlash(data, offs, length):
            print(prefix, f"Writing failed at 0x{offs:x}")
            return False
    return True


def upload(ctx: UploadContext, protocol: str, **kwargs) -> bool:
    if protocol == "uart":
        return upload_uart(ctx, **kwargs)
    print(f"Unknown upload protocol - {protocol}")
    return False
