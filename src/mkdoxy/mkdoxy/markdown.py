from typing import List

def escape(s: str) -> str:
	ret = s.replace('*', '\\*')
	ret = ret.replace('_', '\\_')
	ret = ret.replace('<', '&lt;')
	ret = ret.replace('>', '&gt;')
	ret = ret.replace('|', '\|')
	return ret



class MdRenderer:
	def __init__(self):
		self.output = ''
		self.eol_flag = True

	def write(self, s: str):
		self.output += s
		self.eol_flag = False

	def eol(self):
		if not self.eol_flag:
			self.output += '\n'
			self.eol_flag = True


class Md:
	def __init__(self, children: List['Md']):
		self.children = children

	def append(self, child: 'Md'):
		self.children.append(child)

	def extend(self, child: List['Md']):
		self.children.extend(child)


class Text:
	def __init__(self, text: str):
		self.text = text

	def render(self, f: MdRenderer, indent: str):
		if self.text:
			f.write(escape(self.text))


class Br:
	def __init__(self):
		pass

	def render(self, f: MdRenderer, indent: str):
		f.write('\n\n')


class MdHint(Md):
	def __init__(self, children: List[Md], typ: str, title: str):
		Md.__init__(self, children)
		self.title = title
		self.typ = typ

	def render(self, f: MdRenderer, indent: str):
		f.write('::: ' + self.typ + ' ' + self.title + '\n')
		for child in self.children:
			child.render(f, '')
		f.write(':::\n')


class MdBold(Md):
	def __init__(self, children: List[Md]):
		Md.__init__(self, children)

	def render(self, f: MdRenderer, indent: str):
		f.write('**')
		for child in self.children:
			child.render(f, '')
		f.write('**')


class MdImage:
	def __init__(self, url: str):
		self.url = url

	def render(self, f: MdRenderer, indent: str):
		f.write('![Image](' + self.url + ')')


class Code():
	def __init__(self, text: str):
		self.text = text

	def render(self, f: MdRenderer, indent: str):
		f.write('`' + self.text + '`')


class MdCodeBlock:
	def __init__(self, lines: List[str]):
		self.lines = lines

	def append(self, line: str):
		self.lines.append(line)

	def render(self, f: MdRenderer, indent: str):
		for line in self.lines:
			f.write(line)
			f.write('\n')


class MdBlockQuote(Md):
	def __init__(self, children: List[Md]):
		Md.__init__(self, children)

	def render(self, f: MdRenderer, indent: str):
		f.write('\n')
		for child in self.children:
			f.write('> ')
			child.render(f, '')
			f.write('\n')


class MdItalic(Md):
	def __init__(self, children: List[Md]):
		Md.__init__(self, children)

	def render(self, f: MdRenderer, indent: str):
		f.write('_')
		for child in self.children:
			child.render(f, '')
		f.write('_')


class MdParagraph(Md):
	def __init__(self, children: List[Md]):
		Md.__init__(self, children)

	def render(self, f: MdRenderer, indent: str):
		for child in self.children:
			child.render(f, indent)
		f.eol()


class MdLink(Md):
	def __init__(self, children: List[Md], url: str):
		Md.__init__(self, children)
		self.url = url

	def render(self, f: MdRenderer, indent: str):
		f.write('[')
		for child in self.children:
			child.render(f, '')
		f.write('](' + self.url + ')') # TODO add {{link_prefix}}


class MdList(Md):
	def __init__(self, children: List[Md]):
		Md.__init__(self, children)

	def render(self, f: MdRenderer, indent: str):
		f.eol()
		for child in self.children:
			if not isinstance(child, MdList):
				f.write(indent + '* ')
			child.render(f, indent + '  ')


class MdLine:
	def __init__(self):
		pass

	def render(self, f: MdRenderer, indent: str):
		f.eol()
		f.write('----------------------------------------')
		f.eol()


class MdHeader(Md):
	def __init__(self, level: int, children: List[Md]):
		Md.__init__(self, children)
		self.level = level

	def render(self, f: MdRenderer, indent: str):
		f.write('#' * self.level + ' ')
		for child in self.children:
			child.render(f, indent + '')
		f.write('\n')
		f.eol()


class MdTableCell(Md):
	def __init__(self, children: List[Md]):
		Md.__init__(self, children)

	def render(self, f: MdRenderer, indent: str):
		for child in self.children:
			child.render(f, indent)


class MdTableRow(Md):
	def __init__(self, children: List[Md]):
		Md.__init__(self, children)

	def render(self, f: MdRenderer, indent: str):
		f.eol()
		f.write('|')
		for child in self.children:
			child.render(f, '')
			f.write('|')
		f.eol()


class MdTable(Md):
	def __init__(self):
		Md.__init__(self, [])

	def render(self, f: MdRenderer, indent: str):
		is_first = True
		f.eol()
		for child in self.children:
			child.render(f, '')
			if is_first:
				for _ in range(len(child.children)):
					f.write('|-----')
				f.write('|')
			is_first = False
		f.write('\n\n')
