import argparse
import sys
import os
from doxygen_snippets.doxygen import Doxygen
from doxygen_snippets.generatorBase import GeneratorBase
from doxygen_snippets.generatorAuto import GeneratorAuto
from doxygen_snippets.xml_parser import XmlParser
from doxygen_snippets.cache import Cache
from doxygen_snippets.constants import Kind
from doxygen_snippets.runner import run
from doxygen_snippets.node import Node
from doxygen_snippets.finder import Finder
from doxygen_snippets.doxyrun import DoxygenRun
from pprint import *

if __name__ == "__main__":
	doxygenPath = "files/"
	doxygenInput = "src"
	doxygenOutput = "doxy"
	doxygenConfig = "Doxyfile"
	apiOutput = "files/docs/"
	target = "mkdocs"
	hints = True
	ignoreErrors = False
	summary = None
	link_prefix = ""

	# Debug options
	debug = True
	debugFull = True
	fullDoc = True

	os.makedirs(apiOutput, exist_ok=True)

	doxygenRun = DoxygenRun(doxygenPath, doxygenInput, doxygenOutput, doxygenConfig)
	doxygenRun.run()

	options = {
		'target': target,
		'link_prefix': link_prefix
	}

	cache = Cache()
	parser = XmlParser(cache=cache, target=target, hints=hints, debug=debug)
	doxygen = Doxygen(doxygenRun.getDestination(), parser, cache, options=options, debug=debug)

	if debugFull:
		doxygen.print()

	generatorBase = GeneratorBase(ignore_errors=ignoreErrors, options=options)
	generatorAuto = GeneratorAuto(generatorBase=generatorBase, debug=debug)

	if fullDoc:
		generatorAuto.fullDoc(apiOutput, doxygen)

	pp(generatorAuto)
