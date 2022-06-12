# Copyright (c) Kuba Szczodrzyński 2022-05-31.

import sys
from os.path import dirname, join

sys.path.append(join(dirname(__file__), ".."))

from tools.util.platform import get_families

HEADER = """\
# Families

A list of families currently available in this project.

**Note:** the term *family* was chosen over *platform*, in order to reduce possible confusion between LibreTuya supported "platforms" and PlatformIO's "platform", as an entire package. *Family* is also more compatible with the UF2 term.

The following list corresponds to UF2 OTA format family names, and is also [available as JSON](../families.json). The IDs are also present in [ChipType.h](../arduino/libretuya/core/ChipType.h).
"""


def format_row(row: list, lengths: list) -> str:
    row = [col + " " * (lengths[i] - len(col)) for i, col in enumerate(row)]
    return " | ".join(row).rstrip()


if __name__ == "__main__":
    out = join(dirname(__file__), "families.md")

    md = [HEADER]

    lengths = [0, 0, 0, 0, 0, 0, 0]
    header = [
        "Title",
        "Name (parent)",
        "Code",
        "Short name & ID",
        "Supported MCU(s)",
        "Arduino Core",
        "Source SDK",
    ]
    rows = []

    for family in get_families():
        row = [
            # Title
            "[{}]({})".format(
                family.description,
                family.url,
                family.name,
            )
            if family.name
            else family.description,
            # Name (parent)
            f"`{family.name or '-'}`"
            if not family.parent
            else f"`{family.name}` (`{family.parent}`)",
            # Code
            f"`{family.code or '-'}`"
            if not family.parent
            else f"`{family.code}` (`{family.parent_code}`)",
            # Short name & ID
            "`{}` (0x{:X})".format(
                family.short_name,
                family.id,
            ),
            # f"`{family.short_name}` (0x{family.id:X})",
            # Supported MCU(s)
            ", ".join(family.mcus) or "-",
            # Arduino Core
            "✔️" if family.has_arduino_core else "❌",
            # Source SDK
            "`{}` ([{}]({}))".format(
                family.framework,
                family.sdk_name,
                family.sdk,
            )
            if family.name
            else "-",
        ]
        rows.append(row)

    for row in [header] + rows:
        for i, col in enumerate(row):
            lengths[i] = max(lengths[i], len(col))

    md.append(format_row(header, lengths))
    md.append("-|-".join(length * "-" for length in lengths))
    for row in rows:
        md.append(format_row(row, lengths))

    md.append("")

    with open(out, "w", encoding="utf-8") as f:
        f.write("\n".join(md))
