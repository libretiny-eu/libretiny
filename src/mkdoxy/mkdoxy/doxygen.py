import os
from xml.etree import ElementTree
from mkdoxy.node import Node
from mkdoxy.constants import Kind, Visibility
from mkdoxy.cache import Cache
from mkdoxy.xml_parser import XmlParser
import logging

log = logging.getLogger("mkdocs")



class Doxygen:
	def __init__(self, index_path: str, parser: XmlParser, cache: Cache, debug: bool = False):
		self.debug = debug
		path = os.path.join(index_path, 'index.xml')
		if self.debug:
			log.info('Loading XML from: ' + path)
		xml = ElementTree.parse(path).getroot()

		self.parser = parser
		self.cache = cache

		self.root = Node('root', None, self.cache, self.parser, None)
		self.groups = Node('root', None, self.cache, self.parser, None)
		self.files = Node('root', None, self.cache, self.parser, None)
		self.pages = Node('root', None, self.cache, self.parser, None)

		for compound in xml.findall('compound'):
			kind = Kind.from_str(compound.get('kind'))
			refid = compound.get('refid')
			if kind.is_language():
				node = Node(os.path.join(index_path, refid + '.xml'), None, self.cache, self.parser, self.root)
				node._visibility = Visibility.PUBLIC
				self.root.add_child(node)
			if kind == Kind.GROUP:
				node = Node(os.path.join(index_path, refid + '.xml'), None, self.cache, self.parser, self.root)
				node._visibility = Visibility.PUBLIC
				self.groups.add_child(node)
			if kind in [Kind.FILE, Kind.DIR]:
				node = Node(os.path.join(index_path, refid + '.xml'), None, self.cache, self.parser, self.root)
				node._visibility = Visibility.PUBLIC
				self.files.add_child(node)
			if kind == Kind.PAGE:
				node = Node(os.path.join(index_path, refid + '.xml'), None, self.cache, self.parser, self.root)
				node._visibility = Visibility.PUBLIC
				self.pages.add_child(node)

		if self.debug:
			log.info('Deduplicating data... (may take a minute!)')
		for i, child in enumerate(self.root.children.copy()):
			self._fix_duplicates(child, self.root, [])

		for i, child in enumerate(self.groups.children.copy()):
			self._fix_duplicates(child, self.groups, [Kind.GROUP])

		for i, child in enumerate(self.files.children.copy()):
			self._fix_duplicates(child, self.files, [Kind.FILE, Kind.DIR])

		self._fix_parents(self.files)

		if self.debug:
			log.info('Sorting...')
		self._recursive_sort(self.root)
		self._recursive_sort(self.groups)
		self._recursive_sort(self.files)
		self._recursive_sort(self.pages)

	def _fix_parents(self, node: Node):
		if node.is_dir or node.is_root:
			for child in node.children:
				if child.is_file:
					child._parent = node
				if child.is_dir:
					self._fix_parents(child)

	def _recursive_sort(self, node: Node):
		node.sort_children()
		for child in node.children:
			self._recursive_sort(child)

	def _is_in_root(self, node: Node, root: Node):
		return any(node.refid == child.refid for child in root.children)

	def _remove_from_root(self, refid: str, root: Node):
		for i, child in enumerate(root.children):
			if child.refid == refid:
				root.children.pop(i)
				return

	def _fix_duplicates(self, node: Node, root: Node, filter: [Kind]):
		for child in node.children:
			if len(filter) > 0 and child.kind not in filter:
				continue
			if self._is_in_root(child, root):
				self._remove_from_root(child.refid, root)
			self._fix_duplicates(child, root, filter)

	def printStructure(self):
		if not self.debug:
			return
		print('\n')
		log.info("Print root")
		for node in self.root.children:
			self.print_node(node, '')
		print('\n')

		log.info("Print groups")
		for node in self.groups.children:
			self.print_node(node, '')
		print('\n')

		log.info("Print files")
		for node in self.files.children:
			self.print_node(node, '')

	def print_node(self, node: Node, indent: str):
		if self.debug:
			log.info(f"{indent} {node.kind} {node.name}")
		for child in node.children:
			self.print_node(child, indent + '  ')
