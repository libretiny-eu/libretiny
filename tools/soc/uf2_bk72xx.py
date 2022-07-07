# Copyright (c) Kuba Szczodrzyński 2022-06-23.

import sys

try:
    from platformio.package.manager.tool import ToolPackageManager

    manager = ToolPackageManager()
    pkg = manager.get_package("tool-bk7231tools")
    sys.path.append(pkg.path)
    from bk7231tools.serial import BK7231Serial
except (ImportError, AttributeError):
    print("You need PlatformIO and tool-bk7231tools package to run this program.")
    exit(1)


from tools.upload.ctx import UploadContext


def upload_uart(
    ctx: UploadContext,
    port: str,
    baud: int = None,
    **kwargs,
) -> bool:
    prefix = "|   |--"
    # connect to chip
    bk = BK7231Serial(port=port, baudrate=baud or ctx.baudrate or 115200)

    # collect continuous blocks of data
    parts = ctx.collect(ota_idx=1)
    # write blocks to flash
    for offs, data in parts.items():
        length = len(data.getvalue())
        data.seek(0)
        print(prefix, f"Writing {length} bytes to 0x{offs:06x}")
        try:
            bk.program_flash(
                data,
                length,
                offs,
                verbose=False,
                crc_check=True,
                dry_run=False,
                really_erase=True,
            )
        except ValueError as e:
            print(prefix, f"Writing failed: {e.args[0]}")
            return False
    # reboot the chip
    bk.reboot_chip()
    return True


def upload(ctx: UploadContext, protocol: str, **kwargs) -> bool:
    if protocol == "uart":
        return upload_uart(ctx, **kwargs)
    print(f"Unknown upload protocol - {protocol}")
    return False
