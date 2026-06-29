import os
import sys
from typing import *
from jinja2 import Template
from mkdocs.config import base
from mkdocs.structure import files, pages
from mkdoxy.doxygen import Doxygen
from mkdoxy.finder import Finder
import re
from ruamel.yaml import YAML, YAMLError
from pprint import *
import pathlib
import string
import traceback
from typing import TextIO
from jinja2 import Template
from jinja2.exceptions import TemplateSyntaxError, TemplateError
from jinja2 import StrictUndefined, Undefined
from mkdoxy.node import Node, DummyNode
from mkdoxy.doxygen import Doxygen
from mkdoxy.constants import Kind
from mkdoxy.generatorBase import GeneratorBase
import logging

log = logging.getLogger("mkdocs")

regexLong= r"(?s)(?<!```yaml\n)(^::: doxy\.(?P<project>[a-zA-Z]+)\.(?P<key>[a-zA-Z.-_]+))\s*\n(?P<yaml>.*?)(?:(?:(?:\r*\n)(?=\n))|(?=:::)|`|\Z)" #https://regex101.com/r/lIgOij/2
regexShort = r"(?s)(?<!```yaml\n)(^::: doxy\.(?P<project>[a-zA-Z]+)\.(?P<key>[a-zA-Z.-_]+))\s*\n(?:(?=\n)|(?=:::)|\Z)" # https://regex101.com/r/QnqxRc/1

class GeneratorSnippets:
	def __init__(self,
	             markdown,
	             generatorBase, #: GeneratorBase,
	             doxygen, # Dict[Doxygen],
				 useDirectoryUrls: bool,
				 page: pages.Page,
	             debug: bool = False):

		self.markdown = markdown
		self.generatorBase = generatorBase
		self.doxygen = doxygen
		self.useDirectoryUrls = useDirectoryUrls
		self.page = page
		self.debug = debug
		self.finder = Finder(doxygen, debug)

		self.DOXY_CALL = {
			"code": self.doxyCode,
			"function": self.doxyFunction,
			"class": self.doxyClass,
			"class.method": self.doxyClassMethod,
			"class.list":self.doxyClassList,
			"class.index":self.doxyClassIndex,
			"class.hierarchy":self.doxyClassHierarchy,
			"namespace.list":self.doxyNamespaceList,
			"file.list":self.doxyFileList,
		}

		# fix absolute path
		path = pathlib.PurePath(self.page.url).parts
		self.pageUrlPrefix = ''.join("../" for i in range(len(path)-1))

	def generate(self):

		matches = re.finditer(regexShort, self.markdown, re.MULTILINE)
		for match in reversed(list(matches)):
			snippet = match.group()
			key = match.group('key')
			project = match.group('project')

			keyLow = key.lower()
			log.debug(f"\nKey: {keyLow}")

			replaceStr = self.callDoxyByName(snippet, project, keyLow, {})
			self.replaceMarkdown(match.start(), match.end(), replaceStr)

		matches = re.finditer(regexLong, self.markdown, re.MULTILINE)
		for match in reversed(list(matches)):
			if match:
				snippet = match.group()
				key = match.group('key')
				project = match.group('project')
				keyLow = key.lower()
				log.debug(f"\nKey: {keyLow}")
				yamlRaw = match.group('yaml')
				if yamlRaw:
					try:
						yaml = YAML()
						config = yaml.load(yamlRaw)
						# yaml.dump(config, sys.stdout)
						log.debug(pformat(config))
					except YAMLError as e:
						print(e)

				replaceStr = self.callDoxyByName(snippet, project, keyLow, config)
				self.replaceMarkdown(match.start(), match.end(), replaceStr)


		return self.markdown

	def replaceMarkdown(self, start: int, end: int, newString: string):
		self.markdown = self.markdown[:start] + newString + "\n" + self.markdown[end:]

	def _recurs_setLinkPrefixNode(self, node: Node, linkPrefix: str):
		node.setLinkPrefix(linkPrefix)
		if node.kind.is_parent():
			self._recurs_setLinkPrefixNodes(node.children, linkPrefix)

	def _recurs_setLinkPrefixNodes(self, nodes: [Node], linkPrefix: str):
		for node in nodes:
			self._recurs_setLinkPrefixNode(node, linkPrefix)

	def callDoxyByName(self, snippet, project: str, key: str, config):
		if key not in self.DOXY_CALL:
			return self.generatorBase[project].error(f"Did not exist key with name: {key}", snippet, "yaml")

		funcName = self.DOXY_CALL[key]
		return funcName(snippet, project, config)

	def checkConfig(self, snippet, project: str, config, params):
		"""
		returns false if config is correct
		return error message if project not exist or find problem in config
		"""
		# Project is exist
		if project not in self.generatorBase:
			return self.generatorBase[list(self.generatorBase)[0]].error(f"Did not exist project with name: {project}", snippet, "yaml")
		# Project has got parameters
		for param in params:
			if not config.get(param):
				return self.doxyError(project, f"The requid parameter `{param}` is not configured!", snippet, "yaml")
		return False

	### Create documentation generator callbacks

	def doxyError(self, project, title: str = "", message: str = "", language: str = ""):
		log.error(f"  -> {title} -> page: {self.page.canonical_url}")
		return self.generatorBase[project].error(title, message, language)
	
	def doxyCode(self, snippet, project: str, config):
		errorMsg = self.checkConfig(snippet, project, config, ["file"])
		if errorMsg:
			return errorMsg
		node = self.finder.doxyCode(project, config.get("file"))
		if isinstance(node, Node):

			progCode = self.codeStrip(node.programlisting, node.code_language, config.get("start", 1), config.get("end", 0))
			if progCode == False:
				return self.doxyError(project, f"Parameter start: {config.get('start')} is greater than end: {config.get('end')}",f"{snippet}", "yaml")
			self._recurs_setLinkPrefixNode(node, self.pageUrlPrefix + project + "/")
			return self.generatorBase[project].code(node, config, progCode)
		return self.doxyError(project, f"Did not find File: `{config.get('file')}`", f"{snippet}\nAvailable:\n{pformat(node)}", "yaml")

	def codeStrip(self, codeRaw, codeLanguage: str, start: int = 1, end: int = None):
		lines = codeRaw.split("\n")

		if end and start > end:
			return False

		out = "".join(line + "\n" for num, line in enumerate(lines)
		              if num >= start and (num <= end or end == 0))
		return f"```{codeLanguage} linenums='{start}'\n{out}```"


	def doxyFunction(self, snippet, project: str, config):
		errorMsg = self.checkConfig(snippet, project, config, ["name"])
		if errorMsg:
			return errorMsg

		node = self.finder.doxyFunction(project, config.get("name"))
		if isinstance(node, Node):
			self._recurs_setLinkPrefixNode(node, self.pageUrlPrefix + project + "/")
			return self.generatorBase[project].function(node, config)
		return self.doxyError(project, f"Did not find Function with name: `{config.get('name')}`", f"{snippet}\nAvailable:\n{pformat(node)}", "yaml")

	def doxyClass(self, snippet, project: str, config):
		errorMsg = self.checkConfig(snippet, project, config, ["name"])
		if errorMsg:
			return errorMsg

		node = self.finder.doxyClass(project, config.get("name"))
		if isinstance(node, Node):
			self._recurs_setLinkPrefixNode(node, self.pageUrlPrefix + project + "/")
			return self.generatorBase[project].member(node, config)
		return self.doxyError(project, f"Did not find Class with name: `{config.get('name')}`", f"{snippet}\nAvailable:\n{pformat(node)}", "yaml")

	def doxyClassMethod(self, snippet, project: str, config):
		errorMsg = self.checkConfig(snippet, project, config, ["name", "method"])
		if errorMsg:
			return errorMsg

		node = self.finder.doxyClassMethod(project, config.get("name"), config.get("method"))
		if isinstance(node, Node):
			self._recurs_setLinkPrefixNode(node, self.pageUrlPrefix + project + "/")
			return self.generatorBase[project].function(node, config)
		return self.doxyError(project, f"Did not find Class with name: `{config.get('name')}` and method: `{config.get('method')}`", f"{snippet}\nAvailable:\n{pformat(node)}", "yaml")


	def doxyClassList(self, snippet, project: str, config):
		errorMsg = self.checkConfig(snippet, project, config, [])
		if errorMsg:
			return errorMsg
		nodes = self.doxygen[project].root.children
		self._recurs_setLinkPrefixNodes(nodes, self.pageUrlPrefix + project + "/")
		return self.generatorBase[project].annotated(nodes)

	def doxyClassIndex(self, snippet, project: str, config):
		errorMsg = self.checkConfig(snippet, project, config, [])
		if errorMsg:
			return errorMsg
		nodes = self.doxygen[project].root.children
		self._recurs_setLinkPrefixNodes(nodes, self.pageUrlPrefix + project + "/")
		return self.generatorBase[project].classes(nodes)

	def doxyClassHierarchy(self, snippet, project: str, config):
		errorMsg = self.checkConfig(snippet, project, config, [])
		if errorMsg:
			return errorMsg
		nodes = self.doxygen[project].root.children
		self._recurs_setLinkPrefixNodes(nodes, self.pageUrlPrefix + project + "/")
		return self.generatorBase[project].hierarchy(nodes)

	def doxyNamespaceList(self, snippet, project: str, config):
		errorMsg = self.checkConfig(snippet, project, config, [])
		if errorMsg:
			return errorMsg
		nodes = self.doxygen[project].root.children
		self._recurs_setLinkPrefixNodes(nodes, self.pageUrlPrefix + project + "/")
		return self.generatorBase[project].namespaces(nodes)

	def doxyFileList(self, snippet, project: str, config):
		errorMsg = self.checkConfig(snippet, project, config, [])
		if errorMsg:
			return errorMsg
		nodes = self.doxygen[project].files.children
		self._recurs_setLinkPrefixNodes(nodes, self.pageUrlPrefix + project + "/")
		return self.generatorBase[project].fileindex(nodes)

### Create documentation generator callbacks END


class SnippetClass:
	def __init__(self, config):
		self.config = config

	def default(self):
		return ""