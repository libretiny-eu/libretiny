import argparse
import sys
import os
from doxygen_snippets.doxygen import Doxygen
from doxygen_snippets.generatorBase import GeneratorBase
from doxygen_snippets.generatorAuto import GeneratorAuto
from doxygen_snippets.generatorSnippets import GeneratorSnippets
from doxygen_snippets.xml_parser import XmlParser
from doxygen_snippets.cache import Cache
from doxygen_snippets.constants import Kind
from doxygen_snippets.runner import run
from doxygen_snippets.node import Node
from doxygen_snippets.finder import Finder
from doxygen_snippets.doxyrun import DoxygenRun
from pprint import *

if __name__ == "__main__":
	doxygenSource = "files/src-stm32"
	# doxygenSource = "files/src"
	tempDoxyDir="files/doxy"
	siteDir = "files/doxy"
	apiPath="api"
	target = "mkdocs"
	useDirectoryUrls=True
	hints = True
	ignoreErrors = False
	summary = None
	link_prefix = ""

	# Debug options
	debug = True
	debugFull = False
	fullDoc = True

	doxygenRun = DoxygenRun(doxygenSource, siteDir)
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
	
	generatorAuto = GeneratorAuto(generatorBase=generatorBase,
	                              tempDoxyDir=tempDoxyDir,
	                              siteDir=siteDir,
	                              apiPath=apiPath,
	                              useDirectoryUrls=useDirectoryUrls,
	                              fullDocFiles=[],
	                              debug=debug)
	if fullDoc:
		generatorAuto.fullDoc(doxygen)

	# find = Finder(doxygen, debug)
	# fc = find.doxyClass("example::Bird", "Bird (const Bird & other)= delete")

	generatorSnippets = GeneratorSnippets(markdown="", generatorBase=generatorBase, doxygen=doxygen, debug=debug)
	# func = generatorSnippets.doxyFunction("", {"name":"getRandomNumber()"})

	# func = generatorSnippets.doxyCode("", {"file":"shape.cppa"})
	func = generatorSnippets.doxyClassMethod("", {"name":"asd", "method":"as"})

	pp(func)
