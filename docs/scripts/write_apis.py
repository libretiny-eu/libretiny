# Copyright (c) Kuba Szczodrzyński 2023-06-22.

import re
from glob import glob
from os.path import dirname, join

import colorama
from colorama import Fore, Style
from markdown import Markdown

if __name__ == "__main__":
    colorama.init()

    api_path = join(dirname(__file__), "..", "..", "cores/common/base/api/lt_*.*")
    out_path = join(dirname(__file__), "..", "contrib")

    declaration = ""
    implementation = ""

    for file in glob(api_path):
        with open(file, "r") as f:
            data = f.read()
        if file.endswith(".h"):
            declaration += data
        elif file.endswith(".c"):
            implementation += data

    block_comment_regex = r"\/\*[\d\D]+?\*\/"
    line_comment_regex = r"\/\/.+?$"
    macro_regex = r"#(?:[^\n\\]|\\\n)+$"
    line_regex = r"\n+"
    declaration = re.sub(block_comment_regex, "", declaration)
    declaration = re.sub(line_comment_regex, "", declaration, flags=re.MULTILINE)
    declaration = re.sub(macro_regex, "", declaration, flags=re.MULTILINE)
    declaration = re.sub(line_regex, "\n", declaration)
    implementation = re.sub(block_comment_regex, "", implementation)
    implementation = re.sub(line_comment_regex, "", implementation, flags=re.MULTILINE)
    implementation = re.sub(macro_regex, "", implementation, flags=re.MULTILINE)
    implementation = re.sub(line_regex, "\n", implementation)

    declaration_regex = r"^([^\s][\d\w\s]+ \*?)([\S]+?)\(.*?\);$"
    implementation_regex = r"^(__attribute__\(\(weak\)\) )?([\w\d* ]+?)([\w\d]+)\(.+?{"

    function_types = {}
    decl_functions = set()
    impl_functions = set()
    weak_functions = set()

    for match in re.finditer(
        pattern=declaration_regex,
        string=declaration,
        flags=re.DOTALL | re.MULTILINE,
    ):
        function_type = match[1].strip()
        function_name = match[2].strip()

        if function_types.get(function_name, function_type) != function_type:
            print(
                Fore.YELLOW
                + "WARNING: Wrong return type: "
                + f"'{function_types[function_name]} {function_name}'"
                + f"vs '{function_type} {function_name}'"
                + Style.RESET_ALL
            )

        function_types[function_name] = function_type
        decl_functions.add(function_name)

    for match in re.finditer(
        pattern=implementation_regex,
        string=implementation,
        flags=re.DOTALL | re.MULTILINE,
    ):
        is_weak = match[1]
        function_type = match[2].strip()
        function_name = match[3].strip()
        function_types[function_name] = function_type

        if function_types.get(function_name, function_type) != function_type:
            print(
                Fore.YELLOW
                + "WARNING: Wrong return type: "
                + f"'{function_types[function_name]} {function_name}'"
                + f"vs '{function_type} {function_name}'"
                + Style.RESET_ALL
            )

        function_types[function_name] = function_type
        if is_weak:
            weak_functions.add(function_name)
        else:
            impl_functions.add(function_name)

for function in list(impl_functions):
    if "static" in function_types[function]:
        impl_functions.remove(function)

common_functions = impl_functions.union(weak_functions)
family_functions = decl_functions - common_functions
undecl_functions = common_functions - decl_functions
if undecl_functions:
    print(Fore.RED + "ERROR: Undeclared functions: " + ", ".join(undecl_functions))
    exit(1)

md = Markdown(out_path, "lt-api-functions")
header = [
    "Type",
    "Function",
    "Common",
    "Weak",
    "Family",
]
rows = []

for function in (
    sorted(family_functions) + sorted(weak_functions) + sorted(impl_functions)
):
    rows.append(
        [
            f"`{function_types[function]}`",
            f"{function}()",
            "✔️" if function in impl_functions else "",
            "✔️" if function in weak_functions else "",
            "✔️" if function not in common_functions else "",
        ]
    )

md.add_table(header, *rows)
md.write()
