from subprocess import Popen, PIPE, STDOUT

p = Popen(['doxygen', '-'], stdout=PIPE, stdin=PIPE, stderr=PIPE)

input = f"""
INPUT = files/src
OUTPUT_DIRECTORY = files/doxy2
DOXYFILE_ENCODING = UTF-8
GENERATE_XML = YES
RECURSIVE = YES
EXAMPLE_PATH = examples
SHOW_NAMESPACES = YES
GENERATE_HTML = NO
GENERATE_LATEX = NO
"""

stdout_data = p.communicate(input.encode('utf-8'))[0].decode().strip()
print(stdout_data)