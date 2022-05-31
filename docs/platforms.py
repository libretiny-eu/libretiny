# Copyright (c) Kuba Szczodrzyński 2022-05-31.

import json
from os.path import dirname, isdir, join

HEADER = """\
# Platforms

A list of platforms currently available in this project.

The following list corresponds to UF2 OTA format family names, and is also [available as JSON](../uf2families.json). The IDs are also present in [uf2ota/models.py](../tools/uf2ota/models.py) and [ChipType.h](../arduino/libretuya/core/ChipType.h).
"""


def format_row(row: list, lengths: list) -> str:
    row = [col + " " * (lengths[i] - len(col)) for i, col in enumerate(row)]
    return " | ".join(row).rstrip()


if __name__ == "__main__":
    data = join(dirname(__file__), "..", "uf2families.json")
    out = join(dirname(__file__), "platforms.md")
    with open(data, "r") as f:
        data = json.load(f)

    md = [HEADER]

    lengths = [0, 0, 0, 0, 0, 0]
    header = [
        "Platform name",
        "Platform code",
        "Family name & ID",
        "Supported MCU(s)",
        "Arduino Core",
        "Source SDK",
    ]
    rows = []

    for family in data:
        id = family["id"]
        short_name = family["short_name"]
        description = family["description"]
        platform = family.get("platform", "")
        platform_code = family.get("platform_code", "-")
        platform_url = family.get("platform_url", "-")
        platform_sdk = family.get("platform_sdk", "-")
        platform_framework = family.get("platform_framework", "-")
        mcus = family.get("mcus", "-")
        platform_sdk_name = platform_sdk.rpartition("/")[2]
        arduino = (
            isdir(join(dirname(__file__), "..", "arduino", platform))
            if platform
            else False
        )
        row = [
            f"[{description}]({platform_url}) (`{platform}`)"
            if platform
            else description,
            f"`{platform_code}`",
            f"`{short_name}` ({id})",
            ", ".join(mcus),
            "✔️" if arduino else "❌",
            f"`{platform_framework}` ([{platform_sdk_name}]({platform_sdk}))"
            if platform
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
