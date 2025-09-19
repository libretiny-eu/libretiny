import os
import re
import string
import traceback
from dataclasses import dataclass, field
from typing import Tuple
from typing import TextIO
from jinja2.exceptions import TemplateSyntaxError, TemplateError
from jinja2 import StrictUndefined, Undefined
from jinja2 import Environment, FileSystemLoader, Template, select_autoescape
import mkdoxy
from mkdoxy.node import Node, DummyNode
from mkdoxy.doxygen import Doxygen
from mkdoxy.constants import Kind
from mkdoxy.utils import parseTemplateFile, merge_two_dicts, recursive_find_with_parent, recursive_find
from mkdocs import exceptions
from markdown import extensions, preprocessors
import logging

log = logging.getLogger("mkdocs")


LETTERS = string.ascii_lowercase + '~_@\\'

class GeneratorBase:
	def __init__(self, ignore_errors: bool = False, debug: bool = False):
		self.debug = debug

		on_undefined_class = Undefined
		if not ignore_errors:
			on_undefined_class = StrictUndefined

		self.templates: Dict[str, Template] = {}
		self.metaData: Dict[str, list[str]] = {}

		# code from https://github.com/daizutabi/mkapi/blob/master/mkapi/core/renderer.py#L29-L38
		path = os.path.join(os.path.dirname(mkdoxy.__file__), "templates")
		for fileName in os.listdir(path):
			filePath = os.path.join(path, fileName)
			if fileName.endswith(".jinja2"):
				with open(filePath, "r") as file:
					name = os.path.splitext(fileName)[0]
					fileTemplate, metaData = parseTemplateFile(file.read())
					self.templates[name] = Template(fileTemplate)
					self.metaData[name] = metaData
			else:
				log.error(f"Trying to load unsupported file '{filePath}'. Supported file ends with '.jinja2'.")
				
	def loadConfigAndTemplate(self, name: str) -> [Template, dict]:
		template = self.templates.get(name)
		if not template:
			raise exceptions.Abort(f"Trying to load unexisting template '{name}'. Please create a new template file with name '{name}.jinja2'")
		metaData = self.metaData.get(name, {})
		return template, metaData

	def render(self, tmpl: Template, data: dict) -> str:
		try:
			if self.debug:
				print('Generating', path)
			return tmpl.render(data)
		except TemplateError as e:
			raise Exception(str(e))

	def error(self, title: str = "", message: str = "", language: str = ""):
		template, metaConfig = self.loadConfigAndTemplate("error")

		data = {
			'title': title,
			'message': message,
			'language': language,

		}
		return self.render(template, data)

	def annotated(self, nodes: [Node], config: dict = {}):
		template, metaConfig = self.loadConfigAndTemplate("annotated")
		data = {
			'nodes': nodes,
			'config': merge_two_dicts(config, metaConfig),
		}
		return self.render(template, data)

	def programlisting(self, node: [Node], config: dict = {}):
		template, metaConfig = self.loadConfigAndTemplate("programlisting")
		data = {
			'node': node,
			'config': merge_two_dicts(config, metaConfig),
		}
		return self.render(template, data)

	def code(self, node: [Node], config: dict = {}, code: str = ""):
		template, metaConfig = self.loadConfigAndTemplate("code")
		newConfig = config
		# newConfig = merge_two_dicts(CODE_CONFIG, config)

		data = {
			'node': node,
			'config': merge_two_dicts(config, metaConfig),
			'code': code
		}

		return self.render(template, data)

	def fileindex(self, nodes: [Node], config: dict = {}):
		template, metaConfig = self.loadConfigAndTemplate("files")
		data = {
			'nodes': nodes,
			'config': merge_two_dicts(config, metaConfig),
		}
		return self.render(template, data)

	def namespaces(self, nodes: [Node], config: dict = {}):
		template, metaConfig = self.loadConfigAndTemplate("namespaces")
		data = {
			'nodes': nodes,
			'config': merge_two_dicts(config, metaConfig),
		}
		return self.render(template, data)

	def page(self, node: Node, config: dict = {}):
		template, metaConfig = self.loadConfigAndTemplate("page")
		data = {
			'node': node,
			'config': merge_two_dicts(config, metaConfig),
		}
		return self.render(template, data)

	def relatedpages(self, nodes: [Node], config: dict = {}):
		template, metaConfig = self.loadConfigAndTemplate("page")
		data = {
			'nodes': nodes,
			'config': merge_two_dicts(config, metaConfig),
		}
		return self.render(template, data)

	def classes(self, nodes: [Node], config: dict = {}):
		template, metaConfig = self.loadConfigAndTemplate("classes")

		classes = recursive_find(nodes, Kind.CLASS)
		classes.extend(recursive_find(nodes, Kind.STRUCT))
		classes.extend(recursive_find(nodes, Kind.INTERFACE))
		dictionary = {letter: [] for letter in LETTERS}

		for klass in classes:
			asd = klass.name_short[0].lower()
			dictionary[asd].append(klass)

		for letter in list(dictionary):
			if len(dictionary[letter]) == 0:
				del dictionary[letter]

		data = {
			'dictionary': dictionary,
			'config': merge_two_dicts(config, metaConfig),
		}
		return self.render(template, data)

	def _find_base_classes(self, nodes: [Node], derived: Node):
		ret = []
		for node in nodes:
			if isinstance(node, str):
				ret.append({
					'refid': node,
					'derived': derived
				})
			elif node.kind.is_parent() and not node.kind.is_namespace():
				bases = node.base_classes
				if len(bases) == 0:
					ret.append(node)
				else:
					ret.extend(self._find_base_classes(bases, node))
		return ret

	def modules(self, nodes: [Node], config: dict = {}):
		template, metaConfig = self.loadConfigAndTemplate("modules")
		data = {
			'nodes': nodes,
			'config': merge_two_dicts(config, metaConfig),
		}
		return self.render(template, data)

	def hierarchy(self, nodes: [Node], config: dict = {}):
		template, metaConfig = self.loadConfigAndTemplate("hierarchy")

		classes = recursive_find(nodes, Kind.CLASS)
		classes.extend(recursive_find(nodes, Kind.STRUCT))
		classes.extend(recursive_find(nodes, Kind.INTERFACE))

		bases = self._find_base_classes(classes, None)
		deduplicated = {
		    base.refid: base
		    for base in bases if not isinstance(base, dict)
		}

		for base in bases:
			if isinstance(base, dict):
				if base['refid'] not in deduplicated:
					deduplicated[base['refid']] = []
				deduplicated[base['refid']].append(base)

		deduplicated_arr = []
		for key, children in deduplicated.items():
			if isinstance(children, list):
				deduplicated_arr.append(DummyNode(
					key,
					list(map(lambda x: x['derived'], children)),
					Kind.CLASS
				))
			else:
				found: Node = None
				for klass in classes:
					if klass.refid == key:
						found = klass
						break
				if found:
					deduplicated_arr.append(found)

		data = {
			'classes': deduplicated_arr,
			'config': merge_two_dicts(config, metaConfig),
		}
		return self.render(template, data)

	def function(self, node: Node, config: dict = {}):
		templateMemDef, metaConfigMemDef = self.loadConfigAndTemplate("memDef")

		data = {
			'node': node,
			'configMemDef': merge_two_dicts(config, metaConfigMemDef)
		}
		return self.render(templateMemDef, data)

	def member(self, node: Node, config: dict = {}):
		template, metaConfig = self.loadConfigAndTemplate("member")
		templateMemDef, metaConfigMemDef = self.loadConfigAndTemplate("memDef")
		templateMemTab, metaConfigMemTab = self.loadConfigAndTemplate("memTab")

		data = {
			'node': node,
			'templateMemDef': templateMemDef,
			'configMemDef': metaConfigMemDef,
			'templateMemTab': templateMemTab,
			'configMemTab': metaConfigMemTab,
			'config': merge_two_dicts(config, metaConfig)
		}
		return self.render(template, data)

	def file(self, node: Node, config: dict = {}):
		template, metaConfig = self.loadConfigAndTemplate("member")
		templateMemDef, metaConfigMemDef = self.loadConfigAndTemplate("memDef")
		templateMemTab, metaConfigMemTab = self.loadConfigAndTemplate("memTab")

		data = {
			'node': node,
			'templateMemDef': templateMemDef,
			'configMemDef': metaConfigMemDef,
			'templateMemTab': templateMemTab,
			'configMemTab': metaConfigMemTab,
			'config': merge_two_dicts(config, metaConfig)
		}
		return self.render(template, data)

	def index(self, nodes: [Node], kind_filters: Kind, kind_parents: [Kind], title: str, config: dict = {}):
		template, metaConfig = self.loadConfigAndTemplate("index")

		found_nodes = recursive_find_with_parent(nodes, kind_filters, kind_parents)
		dictionary = {letter: [] for letter in LETTERS}

		# Sort items into the dictionary
		for found in found_nodes:
			dictionary[found.name_tokens[-1][0].lower()].append(found)

		# Delete unused letters
		for letter in list(dictionary):
			if len(dictionary[letter]) == 0:
				del dictionary[letter]

		# Sort items if they have the same name
		sorted_dictionary = {}
		for letter, items in dictionary.items():
			d = {}
			for item in items:
				# The name of the item is not yet in the dictionary
				if item.name_short not in d:
					d[item.name_short] = [item.parent]

				else:
					found = any(test.refid == item.parent.refid for test in d[item.name_short])
					if not found:
						d[item.name_short].append(item.parent)

			sorted_dictionary[letter] = d

		data = {
			'title': title,
			'dictionary': sorted_dictionary,
			'config': merge_two_dicts(config, metaConfig),
		}
		return self.render(template, data)
