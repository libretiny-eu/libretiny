from subprocess import Popen, PIPE, STDOUT
import shlex
import logging
from pprint import *
import sys
import hashlib
import glob
import ruamel.yaml as yaml
from pathlib import Path, PurePath
import tempfile

log = logging.getLogger("mkdocs")


class DoxygenRun:
	def __init__(self, doxygenSource: str, tempDoxyFolder: str, doxyCfgNew):
		self.doxygenSource = doxygenSource
		self.tempDoxyFolder = tempDoxyFolder
		self.doxyCfgNew = doxyCfgNew
		self.hashFileName = "hashChanges.yaml"
		self.hashFilePath = PurePath.joinpath(Path(self.tempDoxyFolder), Path(self.hashFileName))

		self.doxyCfg = {
			"INPUT": self.doxygenSource,
			"OUTPUT_DIRECTORY": self.tempDoxyFolder,
			"DOXYFILE_ENCODING": "UTF-8",
			"GENERATE_XML": "YES",
			"RECURSIVE": "YES",
			"EXAMPLE_PATH": "examples",
			"SHOW_NAMESPACES": "YES",
			"GENERATE_HTML": "NO",
			"GENERATE_LATEX": "NO",
		}

		self.doxyCfg.update(self.doxyCfgNew)
		self.doxyCfgStr = self.dox_dict2str(self.doxyCfg)

		new_file, filename = tempfile.mkstemp()

	# Source of dox_dict2str: https://xdress-fabio.readthedocs.io/en/latest/_modules/xdress/doxygen.html#XDressPlugin
	def dox_dict2str(self, dox_dict):
		s = ""
		new_line = '{option} = {value}\n'
		for key, value in dox_dict.items():

			if value is True:
				_value = 'YES'
			elif value is False:
				_value = 'NO'
			else:
				_value = value

			s += new_line.format(option=key.upper(), value=_value)

		# Don't need an empty line at the end
		return s.strip()

	def hasChanged(self):
		def heshWrite(filename: str, hash: str):
			with open(filename, "w") as file:
				file.write(hash)

		def hashRead(filename: str) -> str:
			with open(filename, "r") as file:
				return str(file.read())

		sha1 = hashlib.sha1()
		srcs = self.doxygenSource.split(" ")
		for src in srcs:
			for path in Path(src).rglob('*.*'):
				# # Code from https://stackoverflow.com/a/22058673/15411117
				# # BUF_SIZE is totally arbitrary, change for your app!
				BUF_SIZE = 65536  # lets read stuff in 64kb chunks!
				with open(path, 'rb') as f:
					while True:
						data = f.read(BUF_SIZE)
						if not data:
							break
						sha1.update(data)
				# print(f"{path}: {sha1.hexdigest()}")

		hahsNew = sha1.hexdigest()
		if Path(self.hashFilePath).is_file():
			hashOld = hashRead(self.hashFilePath)
			if hahsNew == hashOld:
				return False

		heshWrite(self.hashFilePath, hahsNew)
		return True

	def run(self):
		doxyBuilder = Popen(['doxygen', '-'], stdout=PIPE, stdin=PIPE, stderr=PIPE)
		stdout_data = doxyBuilder.communicate(self.doxyCfgStr.encode('utf-8'))[0].decode().strip()
		# log.info(self.destinationDir)
		# log.info(stdout_data)

	def checkAndRun(self):
		if self.hasChanged():
			self.run()
			return True
		else:
			return False


	@property
	def path(self):
		return Path.joinpath(Path(self.tempDoxyFolder), Path("xml"))
