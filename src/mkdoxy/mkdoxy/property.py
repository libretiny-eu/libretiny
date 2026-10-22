import re
from xml.etree.ElementTree import Element as Element
from mkdoxy.xml_parser import XmlParser
from mkdoxy.constants import Kind
from mkdoxy.markdown import escape


class Property:
	class Details:
		def __init__(self, xml: Element, parser: XmlParser, kind: Kind):
			self.xml = xml
			self.parser = parser
			self.kind = kind

		def md(self, plain: bool = False) -> str:
			detaileddescription = self.xml.find('detaileddescription')
			if len(list(detaileddescription)) > 0:
				return self.parser.paras_as_str(detaileddescription, plain=plain)
			else:
				return ''

		def plain(self) -> str:
			return self.md(plain=True)

		def has(self) -> bool:
			detaileddescription = self.xml.find('detaileddescription')
			return len(list(detaileddescription)) > 0

	class Brief:
		def __init__(self, xml: Element, parser: XmlParser, kind: Kind):
			self.xml = xml
			self.parser = parser
			self.kind = kind

		def md(self, plain: bool = False) -> str:
			briefdescription = self.xml.find('briefdescription')
			if not briefdescription:
				return ''

			paras = briefdescription.findall('para')
			if len(paras) > 0:
				text = []
				for para in paras:
					text.append(self.parser.paras_as_str(para, italic=True, plain=plain))
				return ' '.join(text)
			else:
				return ''

		def plain(self) -> str:
			return self.md(plain=True)

		def has(self) -> bool:
			detaileddescription = self.xml.find('detaileddescription')
			return len(list(detaileddescription)) > 0

	class Includes:
		def __init__(self, xml: Element, parser: XmlParser, kind: Kind):
			self.xml = xml
			self.parser = parser
			self.kind = kind

		def md(self, plain: bool = False) -> [str]:
			return self.array(plain=False)

		def plain(self) -> [str]:
			return self.array(plain=True)

		def array(self, plain: bool = False) -> [str]:
			ret = []
			for includes in self.xml.findall('includes'):
				if plain:
					incl = includes.text
				else:
					incl = self.parser.reference_as_str(includes)

				if includes.get('local') == 'yes':
					ret.append('"' + incl + '"')
				else:
					if plain:
						ret.append('<' + incl + '>')
					else:
						ret.append('&lt;' + incl + '&gt;')
			return ret

		def has(self) -> bool:
			return len(self.xml.findall('includes')) > 0

	class Type:
		def __init__(self, xml: Element, parser: XmlParser, kind: Kind):
			self.xml = xml
			self.parser = parser
			self.kind = kind

		def md(self, plain: bool = False) -> str:
			para = self.xml.find('type')
			if para is not None:
				return self.parser.paras_as_str(para, plain=plain)
			else:
				return ''

		def plain(self) -> str:
			return self.md(plain=True)

		def has(self) -> bool:
			return self.xml.find('type') is not None

	class Location:
		def __init__(self, xml: Element, parser: XmlParser, kind: Kind):
			self.xml = xml
			self.parser = parser
			self.kind = kind

		def md(self, plain: bool = False) -> str:
			return self.plain()

		def plain(self) -> str:
			loc = self.xml.find('location')
			if loc is not None:
				return loc.get('file')
			else:
				return ''

		def has(self) -> bool:
			return self.xml.find('location') is not None

	class Params:
		def __init__(self, xml: Element, parser: XmlParser, kind: Kind):
			self.xml = xml
			self.parser = parser
			self.kind = kind

		def md(self, plain: bool = False) -> str:
			return ', '.join(self.array(plain=plain))

		def plain(self) -> str:
			return ', '.join(self.array(plain=True))

		def array(self, plain: bool = False) -> [str]:
			ret = []
			for param in self.xml.findall('param'):
				p = ''
				type = param.find('type')
				p = self.parser.paras_as_str(type, plain=plain)

				declname = param.find('declname')
				if declname is not None:
					p += ' ' + self.parser.paras_as_str(declname, plain=plain)
				else:
					defname = param.find('defname')
					if defname is not None:
						p += ' ' + self.parser.paras_as_str(defname, plain=plain)

				defval = param.find('defval')
				if defval is not None:
					p += '=' + self.parser.paras_as_str(defval, plain=plain)

				ret.append(p.strip())
			return ret

		def has(self) -> bool:
			return len(self.xml.findall('param')) > 0

	class TemplateParams:
		def __init__(self, xml: Element, parser: XmlParser, kind: Kind):
			self.xml = xml
			self.parser = parser
			self.kind = kind

		def md(self, plain: bool = False) -> str:
			return ', '.join(self.array(plain=plain))

		def plain(self) -> str:
			return ', '.join(self.array(plain=True))

		def array(self, plain: bool = False, notype: bool = False) -> [str]:
			ret = []
			templateparamlist = self.xml.find('templateparamlist')
			if templateparamlist is not None:
				for param in templateparamlist.findall('param'):
					if notype:
						declname = param.find('declname')
						if declname is None:
							declname = param.find('type')
						ret.append(self.parser.paras_as_str(declname, plain=plain))
					else:
						type = param.find('type')
						declname = param.find('declname')
						if declname is None:
							declname = param.find('type')
						ret.append(
							self.parser.paras_as_str(type, plain=plain) + ' ' + self.parser.paras_as_str(declname,
							                                                                             plain=plain))
			return ret

		def has(self) -> bool:
			return self.xml.find('templateparamlist') is not None

	class CodeBlock:
		def __init__(self, xml: Element, parser: XmlParser, kind: Kind):
			self.xml = xml
			self.parser = parser
			self.kind = kind

		def md(self, plain: bool = False) -> str:
			return self.plain()

		def plain(self) -> str:
			return ''

		def has(self) -> bool:
			return True

	class Specifiers:
		def __init__(self, xml: Element, parser: XmlParser, kind: Kind):
			self.xml = xml
			self.parser = parser
			self.kind = kind

		def md(self, plain: bool = False) -> str:
			return self.parsed()

		def plain(self) -> str:
			argss = self.xml.find('argsstring')
			if argss is None or argss.text is None:
				return ''
			return argss.text

		def parsed(self) -> str:
			argss = self.xml.find('argsstring')
			if argss is None or argss.text is None:
				return ''

			argsstring = argss.text
			ret = []

			# Is deleted?
			if re.search('\\)\\s*=\\s*delete', argsstring):
				ret.append('= delete')

			# Is default?
			if re.search('\\)\\s*=\\s*default', argsstring):
				ret.append('= default')

			# Is noexcept
			if re.search('\\).*noexcept', argsstring):
				ret.append('noexcept')

			# Is override
			if re.search('\\).*override', argsstring):
				ret.append('override')

			# Is const?
			if self.xml.get('const') == 'yes':
				ret.append('const')

			# Is pure?
			virt = self.xml.get('virt')
			if virt == 'pure-virtual':
				ret.append('= 0')

			return ' '.join(ret)

		def has(self) -> bool:
			return self.xml.find('argsstring') is not None

	class Values:
		def __init__(self, xml: Element, parser: XmlParser, kind: Kind):
			self.xml = xml
			self.parser = parser
			self.kind = kind

		def md(self, plain: bool = False) -> str:
			return ', '.join(self.array(plain=plain))

		def plain(self) -> str:
			return ', '.join(self.array(plain=False))

		def array(self, plain: bool = False) -> [str]:
			ret = []
			if self.kind.is_enum():
				for enumvalue in self.xml.findall('enumvalue'):
					p = '**' + escape(enumvalue.find('name').text) + '**'
					initializer = enumvalue.find('initializer')
					if initializer is not None:
						p += ' ' + self.parser.paras_as_str(initializer, plain=plain)
					ret.append(p)
			return ret

		def has(self) -> bool:
			if self.kind.is_enum():
				return self.xml.find('enumvalue') is not None
			else:
				return False

	class Initializer:
		def __init__(self, xml: Element, parser: XmlParser, kind: Kind):
			self.xml = xml
			self.parser = parser
			self.kind = kind

		def md(self, plain: bool = False) -> str:
			initializer = self.xml.find('initializer')
			if initializer is not None:
				return self.parser.paras_as_str(initializer, plain=plain)
			else:
				return ''

		def plain(self) -> str:
			return self.md(plain=True)

		def has(self) -> bool:
			return self.xml.find('initializer') is not None

	class Definition:
		def __init__(self, xml: Element, parser: XmlParser, kind: Kind):
			self.xml = xml
			self.parser = parser
			self.kind = kind

		def md(self, plain: bool = False) -> str:
			return self.plain()

		def plain(self) -> str:
			definition = self.xml.find('definition')
			if definition is not None and definition.text:
				return definition.text + ';'
			else:
				return ''

		def has(self) -> bool:
			return self.xml.find('definition') is not None

	class Programlisting:
		def __init__(self, xml: Element, parser: XmlParser, kind: Kind):
			self.xml = xml
			self.parser = parser
			self.kind = kind

		def md(self, plain: bool = False) -> str:
			programlisting = self.xml.find('programlisting')
			if programlisting is None:
				return ''

			return self.parser.programlisting_as_str(programlisting)

		def has(self) -> bool:
			return self.xml.find('programlisting') is not None
