import os
import re
import string
import traceback
from mkdocs.structure import files
from io import StringIO
from typing import TextIO
from jinja2 import Template
from jinja2.exceptions import TemplateSyntaxError, TemplateError
from jinja2 import StrictUndefined, Undefined
from mkdoxy.node import Node, DummyNode
from mkdoxy.doxygen import Doxygen
from mkdoxy.constants import Kind
from mkdoxy.generatorBase import GeneratorBase
from mkdoxy.utils import recursive_find, recursive_find_with_parent
from pprint import *
from pathlib import Path, PurePath
import logging

log = logging.getLogger("mkdocs")

ADDITIONAL_FILES = {
	'Namespace ListNamespace List': 'namespaces.md',
	'Namespace Members': 'namespace_members.md',
	'Namespace Member Functions': 'namespace_member_functions.md',
	'Namespace Member Variables': 'namespace_member_variables.md',
	'Namespace Member Typedefs': 'namespace_member_typedefs.md',
	'Namespace Member Enumerations': 'namespace_member_enums.md',
	'Class Index': 'classes.md',
	'Class Hierarchy': 'hierarchy.md',
	'Class Members': 'class_members.md',
	'Class Member Functions': 'class_member_functions.md',
	'Class Member Variables': 'class_member_variables.md',
	'Class Member Typedefs': 'class_member_typedefs.md',
	'Class Member Enumerations': 'class_member_enums.md',
}

def generate_link(name, url) -> str:
	return '* [' + name + '](' + url + ')\n'

# def generate_link(name, url) -> str:
# 	return f"\t\t- '{name}': '{url}'\n"

class GeneratorAuto:
	def __init__(self,
	             generatorBase: GeneratorBase,
	             tempDoxyDir: str,
	             siteDir: str,
	             apiPath: str,
				 doxygen: Doxygen,
	             useDirectoryUrls: bool,
	             debug: bool = False):
		self.generatorBase = generatorBase
		self.tempDoxyDir = tempDoxyDir
		self.siteDir = siteDir
		self.apiPath = apiPath
		self.doxygen = doxygen
		self.useDirectoryUrls = useDirectoryUrls,
		self.fullDocFiles = []
		self.debug = debug
		self.outputSumm = ""
		os.makedirs(os.path.join(self.tempDoxyDir, self.apiPath), exist_ok=True)

	def save(self, path: str, output: str):
		pathRel = os.path.join(self.apiPath, path)
		self.fullDocFiles.append(files.File(pathRel, self.tempDoxyDir, self.siteDir, self.useDirectoryUrls))
		with open(os.path.join(self.tempDoxyDir, pathRel), 'w', encoding='utf-8') as file:
			file.write(output)

	def fullDoc(self):
		self.annotated(self.doxygen.root.children)
		self.fileindex(self.doxygen.files.children)
		self.members(self.doxygen.root.children)
		self.members(self.doxygen.groups.children)
		self.files(self.doxygen.files.children)
		self.namespaces(self.doxygen.root.children)
		self.classes(self.doxygen.root.children)
		self.hierarchy(self.doxygen.root.children)
		self.modules(self.doxygen.groups.children)
		self.pages(self.doxygen.pages.children)
		self.relatedpages(self.doxygen.pages.children)
		self.index(self.doxygen.root.children, [Kind.FUNCTION, Kind.VARIABLE, Kind.TYPEDEF, Kind.ENUM],
		           [Kind.CLASS, Kind.STRUCT, Kind.INTERFACE], 'Class Members')
		self.index(self.doxygen.root.children, [Kind.FUNCTION], [Kind.CLASS, Kind.STRUCT, Kind.INTERFACE],
		           'Class Member Functions')
		self.index(self.doxygen.root.children, [Kind.VARIABLE], [Kind.CLASS, Kind.STRUCT, Kind.INTERFACE],
		           'Class Member Variables')
		self.index(self.doxygen.root.children, [Kind.TYPEDEF], [Kind.CLASS, Kind.STRUCT, Kind.INTERFACE],
		           'Class Member Typedefs')
		self.index(self.doxygen.root.children, [Kind.ENUM], [Kind.CLASS, Kind.STRUCT, Kind.INTERFACE],
		           'Class Member Enums')
		self.index(self.doxygen.root.children, [Kind.FUNCTION, Kind.VARIABLE, Kind.TYPEDEF, Kind.ENUM],
		           [Kind.NAMESPACE], 'Namespace Members')
		self.index(self.doxygen.root.children, [Kind.FUNCTION], [Kind.NAMESPACE], 'Namespace Member Functions')
		self.index(self.doxygen.root.children, [Kind.VARIABLE], [Kind.NAMESPACE], 'Namespace Member Variables')
		self.index(self.doxygen.root.children, [Kind.TYPEDEF], [Kind.NAMESPACE], 'Namespace Member Typedefs')
		self.index(self.doxygen.root.children, [Kind.ENUM], [Kind.NAMESPACE], 'Namespace Member Enums')
		self.index(self.doxygen.files.children, [Kind.FUNCTION], [Kind.FILE], 'Functions')
		self.index(self.doxygen.files.children, [Kind.DEFINE], [Kind.FILE], 'Macros')
		self.index(self.doxygen.files.children, [Kind.VARIABLE, Kind.UNION, Kind.TYPEDEF, Kind.ENUM], [Kind.FILE],
		           'Variables')

	def annotated(self, nodes: [Node]):
		path = 'annotated.md'
		output = self.generatorBase.annotated(nodes)
		self.save(path, output)

	def programlisting(self, node: [Node]):
		path = node.refid + '_source.md'

		output = self.generatorBase.programlisting(node)
		self.save(path, output)

	def fileindex(self, nodes: [Node]):
		path = 'files.md'

		output = self.generatorBase.fileindex(nodes)
		self.save(path, output)

	def namespaces(self, nodes: [Node]):
		path = 'namespaces.md'

		output = self.generatorBase.namespaces(nodes)
		self.save(path, output)

	def page(self, node: Node):
		path = node.name + '.md'

		output = self.generatorBase.page(node)
		self.save(path, output)

	def pages(self, nodes: [Node]):
		for node in nodes:
			self.page(node)

	def relatedpages(self, nodes: [Node]):
		path = 'pages.md'

		output = self.generatorBase.annotated(nodes)
		self.save(path, output)

	def classes(self, nodes: [Node]):
		path = 'classes.md'

		output = self.generatorBase.classes(nodes)
		self.save(path, output)

	def modules(self, nodes: [Node]):
		path = 'modules.md'

		output = self.generatorBase.modules(nodes)
		self.save(path, output)

	def hierarchy(self, nodes: [Node]):
		path = 'hierarchy.md'

		output = self.generatorBase.hierarchy(nodes)
		self.save(path, output)

	def member(self, node: Node):
		path = node.filename

		output = self.generatorBase.member(node)
		self.save(path, output)

		if node.is_language or node.is_group or node.is_file or node.is_dir:
			self.members(node.children)

	def file(self, node: Node):
		path = node.filename

		output = self.generatorBase.file(node)
		self.save(path, output)

		if node.is_file and node.has_programlisting:
			self.programlisting(node)

		if node.is_file or node.is_dir:
			self.files(node.children)

	def members(self, nodes: [Node]):
		for node in nodes:
			if node.is_parent or node.is_group or node.is_file or node.is_dir:
				self.member(node)

	def files(self, nodes: [Node]):
		for node in nodes:
			if node.is_file or node.is_dir:
				self.file(node)

	def index(self, nodes: [Node], kind_filters: Kind, kind_parents: [Kind], title: str):
		path = title.lower().replace(' ', '_') + '.md'

		output = self.generatorBase.index(nodes, kind_filters, kind_parents, title)
		self.save(path, output)

	def _generate_recursive(self, node: Node, level: int):
		if node.kind.is_parent():
			self.outputSumm += str(' ' * level + generate_link(node.kind.value + ' ' + node.name, node.refid + '.md'))
			for child in node.children:
				self._generate_recursive(child, level + 2)

	def _generate_recursive_files(self, node: Node, level: int):
		if node.kind.is_file() or node.kind.is_dir():
			self.outputSumm += str(' ' * level + generate_link(node.name, node.refid + '.md'))
			if node.kind.is_file():
				self.outputSumm += str(' ' * level + generate_link(node.name + ' source', node.refid + '_source.md'))
			for child in node.children:
				self._generate_recursive_files(child, level + 2)

	def _generate_recursive_groups(self, node: Node, level: int):
		if node.kind.is_group():
			self.outputSumm += str(' ' * level + generate_link(node.title, node.refid + '.md'))
			for child in node.children:
				self._generate_recursive_groups(child, level + 2)

	def _generate_recursive_pages(self, node: Node, level: int):
		if node.kind.is_page():
			self.outputSumm += str(' ' * level + generate_link(node.title, node.refid + '.md'))
			for child in node.children:
				self._generate_recursive_pages(child, level + 2)

	def summary(self):
		offset = 0
		self.outputSumm += str(' ' * (offset + 2) + generate_link('Related Pages',  'pages.md'))
		for node in self.doxygen.pages.children:
			self._generate_recursive_pages(node, offset + 4)

		self.outputSumm += str(' ' * (offset + 2) + generate_link('Modules', 'modules.md'))
		for node in self.doxygen.groups.children:
			self._generate_recursive_groups(node, offset + 4)

		self.outputSumm += str(' ' * (offset + 2) + generate_link('Class List', 'annotated.md'))
		for node in self.doxygen.root.children:
			self._generate_recursive(node, offset + 4)

		for key, val in ADDITIONAL_FILES.items():
			self.outputSumm += str(' ' * (offset + 2) + generate_link(key, val))

		self.outputSumm += str(' ' * (offset + 2) + generate_link('Files', 'files.md'))
		for node in self.doxygen.files.children:
			self._generate_recursive_files(node, offset + 4)

		self.outputSumm += str(' ' * (offset + 2) + generate_link('File Variables', 'variables.md'))
		self.outputSumm += str(' ' * (offset + 2) + generate_link('File Functions', 'functions.md'))
		self.outputSumm += str(' ' * (offset + 2) + generate_link('File Macros', 'macros.md'))

		self.save("links.md", self.outputSumm)