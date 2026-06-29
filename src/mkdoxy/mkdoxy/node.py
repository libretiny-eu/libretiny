import os
import re
import traceback
from xml.etree import ElementTree
from xml.etree.ElementTree import Element as Element
from mkdoxy.constants import Kind, Visibility, OVERLOAD_OPERATORS
from mkdoxy.cache import Cache
from mkdoxy.xml_parser import XmlParser
from mkdoxy.markdown import escape
from mkdoxy.utils import split_safe
from mkdoxy.property import Property
import logging

log = logging.getLogger("mkdocs")


class Node:
	def __init__(self, xml_file: str, xml: Element, cache: Cache, parser: XmlParser, parent: 'Node', refid: str = None, debug: bool = False):
		self._children: ['Node'] = []
		self._cache: Cache = cache
		self._parser: XmlParser = parser
		self._parent = parent
		self.debug = debug
		self.linkPrefix = ""

		if xml_file == 'root':
			self._refid = 'root'
			self._kind = Kind.from_str('root')
			self._name = 'root'
			self._xml = None

		elif xml is None:
			if self.debug:
				log.info('Loading XML from: ' + xml_file)
			self._dirname = os.path.dirname(xml_file)
			self._xml = ElementTree.parse(xml_file).getroot().find('compounddef')
			if self._xml is None:
				raise Exception('File ' + xml_file + ' has no <compounddef>')
			self._kind = Kind.from_str(self._xml.get('kind'))
			self._refid = self._xml.get('id')
			self._language = self._xml.get('language')
			self._name = self._xml.find('compoundname').text
			self._cache.add(self._refid, self)
			self._static = False

			if self.debug:
				log.info('Parsing: ' + self._refid)
			self._check_for_children()

			title = self._xml.find('title')
			self._title = title.text if title is not None else self._name
		else:
			self._xml = xml
			self._kind = Kind.from_str(self._xml.get('kind'))
			self._language = parent.code_language
			self._refid = refid if refid is not None else self._xml.get('id')
			self._cache.add(self._refid, self)

			if self.debug:
				log.info('Parsing: ' + self._refid)
			self._check_attrs()
			self._title = self._name

		self._details = Property.Details(self._xml, parser, self._kind)
		self._brief = Property.Brief(self._xml, parser, self._kind)
		self._includes = Property.Includes(self._xml, parser, self._kind)
		self._type = Property.Type(self._xml, parser, self._kind)
		self._location = Property.Location(self._xml, parser, self._kind)
		self._params = Property.Params(self._xml, parser, self._kind)
		self._templateparams = Property.TemplateParams(self._xml, parser, self._kind)
		self._specifiers = Property.Specifiers(self._xml, parser, self._kind)
		self._values = Property.Values(self._xml, parser, self._kind)
		self._initializer = Property.Initializer(self._xml, parser, self._kind)
		self._definition = Property.Definition(self._xml, parser, self._kind)
		self._programlisting = Property.Programlisting(self._xml, parser, self._kind)

	def setLinkPrefix(self, linkPrefix: str):
		self.linkPrefix = linkPrefix

	def add_child(self, child: 'Node'):
		self._children.append(child)

	def sort_children(self):
		self._children.sort(key=lambda x: x._name, reverse=False)

	def _check_for_children(self):
		for innergroup in self._xml.findall('innergroup'):
			refid = innergroup.get('refid')
			if self._kind in [Kind.GROUP, Kind.DIR, Kind.FILE]:
				try:
					child = self._cache.get(refid)
					self.add_child(child)
					continue
				except:
					pass
			child = Node(os.path.join(self._dirname, refid + '.xml'), None, self._cache, self._parser, self)
			child._visibility = Visibility.PUBLIC
			self.add_child(child)

		for innerclass in self._xml.findall('innerclass'):
			refid = innerclass.get('refid')
			prot = Visibility(innerclass.get('prot'))
			if prot == Visibility.PRIVATE:
				continue

			if self._kind in [Kind.GROUP, Kind.DIR, Kind.FILE]:
				try:
					child = self._cache.get(refid)
					self.add_child(child)
					continue
				except:
					pass

			try:
				child = Node(os.path.join(self._dirname, refid + '.xml'), None, self._cache, self._parser, self)
			except FileNotFoundError as e:
				child = Node(os.path.join(self._dirname, refid + '.xml'), Element('compounddef'), self._cache,
				             self._parser, self, refid=refid)
				child._name = innerclass.text
			child._visibility = prot
			self.add_child(child)

		for innerfile in self._xml.findall('innerfile'):
			refid = innerfile.get('refid')
			if self._kind == Kind.DIR:
				try:
					child = self._cache.get(refid)
					self.add_child(child)
					continue
				except:
					pass

			child = Node(os.path.join(self._dirname, refid + '.xml'), None, self._cache, self._parser, self)
			child._visibility = Visibility.PUBLIC
			self.add_child(child)

		for innerdir in self._xml.findall('innerdir'):
			refid = innerdir.get('refid')
			if self._kind == Kind.DIR:
				try:
					child = self._cache.get(refid)
					self.add_child(child)
					continue
				except:
					pass

			child = Node(os.path.join(self._dirname, refid + '.xml'), None, self._cache, self._parser, self)
			child._visibility = Visibility.PUBLIC
			self.add_child(child)

		for innernamespace in self._xml.findall('innernamespace'):
			refid = innernamespace.get('refid')

			if self._kind in [Kind.GROUP, Kind.DIR, Kind.FILE]:
				try:
					child = self._cache.get(refid)
					self.add_child(child)
					continue
				except:
					pass

			child = Node(os.path.join(self._dirname, refid + '.xml'), None, self._cache, self._parser, self)
			child._visibility = Visibility.PUBLIC
			self.add_child(child)

		for sectiondef in self._xml.findall('sectiondef'):
			for memberdef in sectiondef.findall('memberdef'):
				kind = Kind.from_str(memberdef.get('kind'))
				if kind.is_language():
					if self._kind in [Kind.GROUP, Kind.DIR, Kind.FILE]:
						refid = memberdef.get('id')
						try:
							child = self._cache.get(refid)
							self.add_child(child)
							continue
						except:
							pass
					child = Node(None, memberdef, self._cache, self._parser, self)
					self.add_child(child)

	def _check_attrs(self):
		prot = self._xml.get('prot')
		self._visibility = Visibility(prot) if prot is not None else Visibility.PUBLIC

		static = self._xml.get('static')
		self._static = static == 'yes'

		explicit = self._xml.get('explicit')
		self._explicit = explicit == 'yes'

		mutable = self._xml.get('mutable')
		self._mutable = mutable == 'yes'

		inline = self._xml.get('inline')
		self._inline = inline == 'yes'

		const = self._xml.get('inline')
		self._const = const == 'yes'

		name = self._xml.find('name')
		self._name = name.text if name is not None else ''
		virt = self._xml.get('virt')
		if virt:
			self._virtual = virt in ['virtual', 'pure-virtual']
			self._pure = virt == 'pure-virtual'
		else:
			self._virtual = False
			self._pure = False

	def has(self, visibility: str, kinds: [str], static: bool) -> bool:
		return len(self.query(visibility, kinds, static)) > 0

	def query(self, visibility: str, kinds: [str], static: bool) -> ['Node']:
		visibility = Visibility(visibility)
		kinds = list(map(lambda kind: Kind.from_str(kind), kinds))
		return [
		    child for child in self._children if child._visibility == visibility
		    and child._kind in kinds and child._static == static
		]

	@property
	def is_static(self) -> bool:
		return self._static

	@property
	def is_explicit(self) -> bool:
		return self._explicit

	@property
	def is_const(self) -> bool:
		return self._const

	@property
	def is_inline(self) -> bool:
		return self._inline

	@property
	def is_mutable(self) -> bool:
		return self._mutable

	@property
	def is_virtual(self) -> bool:
		return self._virtual

	@property
	def is_pure(self) -> bool:
		return self._pure

	@property
	def has_children(self) -> bool:
		return len(self._children) > 0

	@property
	def children(self) -> ['Node']:
		return self._children

	@property
	def parent(self) -> 'Node':
		return self._parent

	@property
	def is_function(self) -> bool:
		return self._kind.is_function()

	@property
	def is_variable(self) -> bool:
		return self._kind.is_variable()

	@property
	def is_namespace(self) -> bool:
		return self._kind.is_namespace()

	@property
	def is_class(self) -> bool:
		return self._kind.is_class()

	@property
	def is_struct(self) -> bool:
		return self._kind.is_struct()

	@property
	def is_enum(self) -> bool:
		return self._kind.is_enum()

	@property
	def is_class_or_struct(self) -> bool:
		return self._kind.is_class_or_struct()

	@property
	def is_interface(self) -> bool:
		return self._kind.is_interface()

	@property
	def is_typedef(self) -> bool:
		return self._kind.is_typedef()

	@property
	def is_define(self) -> bool:
		return self._kind.is_define()

	@property
	def is_union(self) -> bool:
		return self._kind.is_union()

	@property
	def is_group(self) -> bool:
		return self._kind.is_group()

	@property
	def is_language(self) -> bool:
		return self._kind.is_language()

	@property
	def is_root(self) -> bool:
		return self._kind.is_root()

	@property
	def is_parent(self) -> bool:
		return self._kind.is_parent()

	@property
	def is_friend(self) -> bool:
		return self._kind.is_friend()

	@property
	def is_file(self) -> bool:
		return self._kind.is_file()

	@property
	def is_dir(self) -> bool:
		return self._kind.is_dir()

	@property
	def is_page(self) -> bool:
		return self._kind.is_page()

	@property
	def name(self) -> str:
		return self._name

	@property
	def name_params(self) -> str:
		name = self._name
		type = self._type.plain()
		params = self._specifiers.plain()
		if params:
			return type + " " + name + params
		return self.name_long

	@property
	def title(self) -> str:
		return self._title

	@property
	def refid(self) -> str:
		return self._refid

	@property
	def kind(self) -> str:
		return self._kind

	@property
	def is_operator(self) -> bool:
		return self._name in OVERLOAD_OPERATORS

	@property
	def operators_total(self) -> int:
		return sum(child.name in OVERLOAD_OPERATORS for child in self.children)

	@property
	def operator_num(self) -> int:
		total = 0
		for child in self.parent.children:
			if child.is_function and child.name.replace(' ', '') in OVERLOAD_OPERATORS:
				total += 1
			if child.refid == self._refid:
				break
		return total

	@property
	def name_url_safe(self) -> str:
		name = self.name_tokens[-1]
		name = name.replace(' ', '-').replace('=', '').replace('~', '').lower()
		return name

	@property
	def anchor(self) -> str:
		name = ''
		if self._name.replace(' ', '') in OVERLOAD_OPERATORS:
			num = self.operator_num
			name = 'operator_' + str(self.operator_num - 1) if num > 1 else 'operator'
		elif self.is_overloaded:
			name = self.name_url_safe + '-' + str(self.overload_num) + str(self.overload_total)
		else:
			name = self.name_url_safe

		if name.startswith('-'):
			name = name[1:]
		return self._kind.value + '-' + name

	@property
	def url(self) -> str:
		if self.is_parent or self.is_group or self.is_file or self.is_dir or self.is_page:
			return self.linkPrefix + self._refid + '.md'
		else:
			return self._parent.url + '#' + self.anchor

	@property
	def base_url(self) -> str:
		def prefix(page: str):
			return self.linkPrefix + page

		if self.is_group:
			return prefix("modules.md")
		elif self.is_file or self.is_dir:
			return prefix("files.md")
		elif self.is_namespace:
			return prefix("namespaces.md")
		else:
			return prefix("annotated.md")

	@property
	def base_name(self) -> str:
		if self.is_group:
			return "Modules"
		elif self.is_file or self.is_dir:
			return "FileList"
		elif self.is_namespace:
			return "Namespace List"
		else:
			return "ClassList"

	@property
	def url_source(self) -> str:
		if self.is_parent or self.is_group or self.is_file or self.is_dir:
			return self.linkPrefix + self._refid + '_source.md'
		else:
			return self.linkPrefix + self._refid + '.md'

	@property
	def filename(self) -> str:
		return self.linkPrefix + self._refid + '.md'

	@property
	def root(self) -> 'Node':
		if self._kind == Kind.ROOT:
			return self
		else:
			return self._parent.root

	@property
	def name_tokens(self) -> [str]:
		if self.is_dir or self.is_file:
			return self._name.split('/')
		return split_safe(self._name, '::')

	@property
	def name_short(self) -> str:
		return escape(self.name_tokens[-1])

	@property
	def name_long(self) -> str:
		try:
			if self._parent.is_parent:
				return self._parent.name_long + '::' + escape(self.name_tokens[-1])
			else:
				return escape(self._name)
		except Exception as e:
			print(e)
			raise e

	@property
	def name_full_unescaped(self) -> str:
		if self._parent is not None and not self._parent.is_root and self._parent.is_parent:
			return self._parent.name_full_unescaped + '::' + self.name_tokens[-1]
		else:
			return self.name_tokens[-1]

	@property
	def overload_total(self) -> int:
		if self._parent is not None and self._parent.is_class_or_struct:
			return sum(neighbour.name == self.name for neighbour in self._parent.children)
		return 0

	@property
	def overload_num(self) -> int:
		if self._parent is not None and self._parent.is_class_or_struct:
			count = 0
			for neighbour in self._parent.children:
				if neighbour.name == self.name:
					count += 1
				if neighbour.refid == self.refid:
					break
			return count
		return 0

	@property
	def is_overloaded(self) -> bool:
		return self.overload_total > 1

	@property
	def overload_suffix(self) -> str:
		if self.is_operator:
			return ''

		total = self.overload_total
		if total > 1:
			return '[' + str(self.overload_num) + '/' + str(total) + ']'
		else:
			return ''

	@property
	def parents(self) -> ['Node']:
		ret = []
		if self._parent is not None and (self._parent.is_language
		                                 or self._parent.is_dir):
			ret.extend(self.parent.parents)
		ret.append(self)
		return ret

	@property
	def suffix(self) -> str:
		if self.is_parent:
			if self._templateparams.has():
				return '&lt;' + ', '.join(self._templateparams.array(notype=True)) + '&gt;'
			else:
				return ''
		elif self.is_function:
			return self._specifiers.md()
		elif self.is_variable:
			if self._initializer.has():
				return ' = ' + self._initializer.md()
			else:
				return ''
		elif self.is_define:
			test = self._initializer.md()
			if '\n' in test:
				return ''
			return test
		else:
			return ''

	@property
	def prefix(self) -> str:
		if self.is_function:
			ret = []
			if self.is_virtual:
				ret.append('virtual')
			return ' '.join(ret)
		elif self.kind is Kind.VARIABLE:
			return ''
		else:
			return self.kind.value

	@property
	def code_language(self) -> str:
		return self._language

	@property
	def codeblock(self) -> str:
		code = []
		if self.is_function or self.is_friend:
			if self._templateparams.has():
				code.append('template<' + self._templateparams.plain() + '>')

			typ = self._type.plain()
			if typ:
				typ += ' '
			if self.is_virtual:
				typ = 'virtual ' + typ
			if self.is_explicit:
				typ = 'explicit ' + typ
			if self.is_inline:
				typ = 'inline ' + typ
			if self.is_static:
				typ = 'static ' + typ

			if self._params.has():
				code.append(typ + self.name_full_unescaped + ' (')
				params = self._params.array(plain=True)
				for i, param in enumerate(params):
					if i + 1 >= len(params):
						code.append('    ' + param)
					else:
						code.append('    ' + param + ',')
				code.append(') ' + self._specifiers.parsed())
			else:
				code.append(typ + self.name_full_unescaped + ' () ' + self._specifiers.parsed())

		elif self.is_enum:
			if self._values.has():
				code.append('enum ' + self.name_full_unescaped + ' {')

				values = []
				for enumvalue in self._xml.findall('enumvalue'):
					p = enumvalue.find('name').text
					initializer = enumvalue.find('initializer')
					if initializer is not None:
						p += ' ' + self._parser.paras_as_str(initializer, plain=True)
					values.append(p)

				for i, value in enumerate(values):
					if i + 1 >= len(values):
						code.append('    ' + value)
					else:
						code.append('    ' + value + ',')
				code.append('};')
			else:
				code.append('enum ' + self.name_full_unescaped + ';')

		elif self.is_define:
			if self._params.has():
				code.append('#define ' + self.name_full_unescaped + ' (')
				params = self._params.array(plain=True)
				for i, param in enumerate(params):
					if i + 1 >= len(params):
						code.append('    ' + param)
					else:
						code.append('    ' + param + ',')
				code.append(') ' + self._initializer.plain())
			else:
				code.append('#define ' + self.name_full_unescaped + ' ' + self._initializer.plain())

		else:
			code.append(self._definition.plain())
		return '\n'.join(code)

	@property
	def has_base_classes(self) -> bool:
		return len(self._xml.findall('basecompoundref')) > 0

	@property
	def has_derived_classes(self) -> bool:
		return len(self._xml.findall('derivedcompoundref')) > 0

	@property
	def base_classes(self) -> ['Node']:
		ret = []
		for basecompoundref in self._xml.findall('basecompoundref'):
			refid = basecompoundref.get('refid')
			if refid is None:
				ret.append(basecompoundref.text)
			else:
				ret.append(self._cache.get(refid))
		return ret

	@property
	def derived_classes(self) -> ['Node']:
		ret = []
		for derivedcompoundref in self._xml.findall('derivedcompoundref'):
			refid = derivedcompoundref.get('refid')
			if refid is None:
				ret.append(derivedcompoundref.text)
			else:
				ret.append(self._cache.get(refid))
		return ret

	@property
	def has_details(self) -> bool:
		return self._details.has()

	@property
	def details(self) -> str:
		return self._details.md()

	@property
	def has_brief(self) -> bool:
		return self._brief.has()

	@property
	def brief(self) -> str:
		return self._brief.md()

	@property
	def has_includes(self) -> bool:
		return self._includes.has()

	@property
	def includes(self) -> str:
		return self._includes.plain()

	@property
	def has_type(self) -> bool:
		return self._type.has()

	@property
	def type(self) -> str:
		return self._type.md()

	@property
	def has_location(self) -> bool:
		return self._location.has()

	@property
	def location(self) -> str:
		return self._location.plain()

	@property
	def has_params(self) -> bool:
		return self._params.has()

	@property
	def params(self) -> str:
		if self._params.has():
			return '(' + self._params.md() + ')'
		elif self.is_function:
			return '()'
		else:
			return ''

	@property
	def has_templateparams(self) -> bool:
		return self._templateparams.has()

	@property
	def templateparams(self) -> str:
		return self._templateparams.md()

	@property
	def has_specifiers(self) -> bool:
		return self._specifiers.has()

	@property
	def specifiders(self) -> str:
		return self._specifiers.parsed()

	@property
	def has_values(self) -> bool:
		return self._values.has()

	@property
	def values(self) -> str:
		return self._values.md()

	@property
	def has_initializer(self) -> bool:
		return self._initializer.has()

	@property
	def initializer(self) -> str:
		return self._initializer.md()

	@property
	def has_definition(self) -> bool:
		return self._definition.has()

	@property
	def definition(self) -> str:
		return self._definition.plain()

	@property
	def has_programlisting(self) -> bool:
		return self._programlisting.has()

	@property
	def programlisting(self) -> str:
		return self._programlisting.md()

	@property
	def is_resolved(self) -> bool:
		return True

	@property
	def reimplements(self) -> 'Node':
		reimp = self._xml.find('reimplements')
		if reimp is not None:
			return self._cache.get(reimp.get('refid'))
		else:
			return None


class DummyNode:
	def __init__(self, name_long: str, derived_classes: [Node], kind: Kind):
		self.name_long = name_long
		self.derived_classes = derived_classes
		self.kind = kind

	@property
	def is_resolved(self) -> bool:
		return False
