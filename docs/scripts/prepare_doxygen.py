# Copyright (c) Kuba Szczodrzyński 2023-03-11.

import re
from glob import glob
from os.path import dirname, join

inputs_path = join(dirname(__file__), "..", "..", "cores/common/base/**/*.*")
outputs_path = join(
    dirname(__file__), "..", "..", "cores/common/arduino/libraries/**/*.*"
)
inputs = glob(inputs_path, recursive=True)
outputs = glob(outputs_path, recursive=True)

functions = {}

for input in inputs:
    with open(input, "r") as f:
        code = f.read()
    regex = r"\/\*\*(.+?)\*\/\n.+?\s\*?(\w+)\(.*?\);"
    for match in re.finditer(regex, code, re.DOTALL):
        functions[match[2]] = match[1]

for output in outputs:
    with open(output, "r") as f:
        code = f.read()
    regex = r"(\t*)\/\*\*.+?@copydoc (\w+)\(\)"

    def transform(match: re.Match):
        docs: str = functions.get(match[2], None)
        if not docs:
            return match[0]
        indent = "\n" + match[1]
        docs = indent.join(docs.split("\n"))
        return match[1] + "/** " + docs

    code = re.sub(regex, transform, code, re.DOTALL)
    with open(output, "w") as f:
        f.write(code)
