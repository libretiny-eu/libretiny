# Copyright (c) Kuba Szczodrzyński 2022-06-18.

from os.path import join
from typing import List


class Markdown:
    items: List[str]
    output: str

    def __init__(self, dir: str, name: str) -> None:
        self.items = ["<!-- This file is auto-generated -->", ""]
        self.output = join(dir, f"{name}.md")

    def write(self):
        with open(self.output, "w", encoding="utf-8") as f:
            f.write("\n".join(self.items))
            f.write("\n")

    def pad(self, s: str, i: int) -> str:
        return s + " " * (i - len(s))

    def add_heading(self, text: str, level: int = 1) -> "Markdown":
        self.items.append(level * "#" + " " + text)
        return self

    def get_link(self, text: str, href: str) -> str:
        return f"[{text}]({href})"

    def get_img(self, alt: str, src: str) -> str:
        return f"![{alt}]({src})"

    def add_link(self, text: str, href: str) -> "Markdown":
        self.items.append(self.get_link(text, href))
        return self

    def add_img(self, alt: str, src: str) -> "Markdown":
        self.items.append(self.get_img(alt, src))
        return self

    def add_text(self, *text: str) -> "Markdown":
        self.items.append(" ".join(text))
        return self

    def add_styled(self, style: str, *text: str) -> "Markdown":
        self.items.append(style + " ".join(text) + style)
        return self

    def add_list(self, *items: str) -> "Markdown":
        self.items.append("- " + "\n- ".join(items))
        return self

    def add_table(self, header: List[str], *rows: List[str]) -> "Markdown":
        maxlen = [len(h) for h in header]
        for row in rows:
            for i, col in enumerate(row):
                maxlen[i] = max(maxlen[i], len(col))
        lines = []
        header = [self.pad(h, maxlen[i]) for i, h in enumerate(header)]
        line = " | ".join(header)
        lines.append(line.rstrip())
        underline = ["-" * i for i in maxlen]
        line = "-|-".join(underline)
        lines.append(line.rstrip())
        for row in rows:
            row += [""] * (len(header) - len(row))
            row = [self.pad(h, maxlen[i]) for i, h in enumerate(row)]
            line = " | ".join(row)
            lines.append(line.rstrip())
        self.items.append("\n".join(lines))
        return self
