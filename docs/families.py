# Copyright (c) Kuba Szczodrzyński 2022-05-31.

import json
from os.path import dirname, isdir, join

HEADER = """\
# Families

A list of families currently available in this project.

**Note:** the term *family* was chosen over *platform*, in order to reduce possible confusion between LibreTuya supported "platforms" and PlatformIO's "platform", as an entire package. *Family* is also more compatible with the UF2 term.

The following list corresponds to UF2 OTA format family names, and is also [available as JSON](../uf2families.json). The IDs are also present in [ChipType.h](../arduino/libretuya/core/ChipType.h).
"""


def format_row(row: list, lengths: list) -> str:
    row = [col + " " * (lengths[i] - len(col)) for i, col in enumerate(row)]
    return " | ".join(row).rstrip()


if __name__ == "__main__":
    data = join(dirname(__file__), "..", "families.json")
    out = join(dirname(__file__), "families.md")
    with open(data, "r") as f:
        data = json.load(f)

    md = [HEADER]

    lengths = [0, 0, 0, 0, 0, 0]
    header = [
        "Full name",
        "Code",
        "Short name & ID",
        "Supported MCU(s)",
        "Arduino Core",
        "Source SDK",
    ]
    rows = []

    for family in data:
        id = family["id"]
        short_name = family["short_name"]
        description = family["description"]
        name = family.get("name", "")
        code = family.get("code", "-")
        url = family.get("url", "-")
        sdk = family.get("sdk", "-")
        framework = family.get("framework", "-")
        mcus = family.get("mcus", "-")
        sdk_name = sdk.rpartition("/")[2]
        arduino = (
            isdir(join(dirname(__file__), "..", "arduino", name)) if name else False
        )
        row = [
            f"[{description}]({url}) (`{name}`)" if name else description,
            f"`{code}`",
            f"`{short_name}` ({id})",
            ", ".join(mcus),
            "✔️" if arduino else "❌",
            f"`{framework}` ([{sdk_name}]({sdk}))" if name else "-",
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
