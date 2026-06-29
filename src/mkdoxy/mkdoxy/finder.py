import os
import re
import string
import traceback
from typing import TextIO
from jinja2 import Template
from jinja2.exceptions import TemplateSyntaxError, TemplateError
from jinja2 import StrictUndefined, Undefined
from mkdoxy.node import Node, DummyNode
from mkdoxy.doxygen import Doxygen
from mkdoxy.constants import Kind
from mkdoxy.utils import recursive_find, recursive_find_with_parent
from pprint import pprint


class Finder:
	def __init__(self, doxygen: Doxygen, debug: bool = False):
		self.doxygen = doxygen
		self.debug = debug

	def _normalize(self, name: str) -> str:
		return name.replace(" ", "")

	def listToNames(self, list):
		return [part.name_params for part in list]

	def doxyClass(self, project, className: str):
		classes = recursive_find(self.doxygen[project].root.children, Kind.CLASS)
		if classes:
			for findClass in classes:
				if findClass.name_long == className:
					return findClass
			return self.listToNames(classes)
		return None

	def doxyClassMethod(self, project, className: str, methodName: str):
		findClass = self.doxyClass(project, className)
		if findClass:
			if isinstance(findClass, list):
				for member in findClass:
					if self._normalize(methodName) in self._normalize(member):
						return member
				return findClass
			else:
				members = recursive_find(findClass.children, Kind.FUNCTION)
				if members:
					for member in members:
						if self._normalize(methodName) in self._normalize(member.name_params):
							return member
					return self.listToNames(members)
				return None
		return None

	def doxyFunction(self, project, functionName: str):
		functions = recursive_find_with_parent(self.doxygen[project].files.children, [Kind.FUNCTION], [Kind.FILE])
		if functions:
			for function in functions:
				if self._normalize(functionName) == self._normalize(function.name_params):
					return function
			return self.listToNames(functions)
		return None

	def doxyCode(self, project, fileName):
		files = recursive_find_with_parent(self.doxygen[project].files.children, [Kind.FILE], [Kind.DIR])
		if files:
			for file in files:
				if self._normalize(fileName) == self._normalize(file.name_long):
					return file
			return self.listToNames(files)
		return None
