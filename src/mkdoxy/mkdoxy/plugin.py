import sys
from os import path, makedirs
from pathlib import Path, PurePath
from mkdocs import utils as mkdocs_utils
from mkdocs.plugins import BasePlugin
from mkdocs.config import base, config_options, Config
from mkdocs.structure import files, pages
from mkdocs.commands import serve
from mkdocs import exceptions

from mkdoxy.doxyrun import DoxygenRun
from mkdoxy.doxygen import Doxygen
from mkdoxy.generatorBase import GeneratorBase
from mkdoxy.generatorAuto import GeneratorAuto
from mkdoxy.xml_parser import XmlParser
from mkdoxy.cache import Cache
from mkdoxy.constants import Kind
from mkdoxy.generatorSnippets import GeneratorSnippets
from mkdoxy.finder import Finder

from pprint import *
import logging

log = logging.getLogger("mkdocs")
pluginName = "MkDoxy"


class MkDoxy(BasePlugin):
	"""
	plugins:
	- search
	- mkdoxy
	"""

	config_scheme = (
		('projects', config_options.Type(dict, default={})),
		('full-doc', config_options.Type(bool, default=True)),
		('debug', config_options.Type(bool, default=False)),
		('ignore-errors', config_options.Type(bool, default=False)),
		('save-api', config_options.Type(str, default="")),
	)

	config_project = (
		('src-dirs', config_options.Type(str)),
		('full-doc', config_options.Type(bool, default=True)),
		('debug', config_options.Type(bool, default=False)),
		# ('ignore-errors', config_options.Type(bool, default=False)),
		('doxy-cfg', config_options.Type(dict, default={}, required=False)),
	)

	def on_files(self, files: files.Files, config):
		def checkConfig(config_project, proData, strict: bool):
			cfg = Config(config_project, '')
			cfg.load_dict(proData)
			errors, warnings = cfg.validate()
			for config_name, warning in warnings:
				log.warning(f"  -> Config value: '{config_name}' in project '{projectName}'. Warning: {warning}")
			for config_name, error in errors:
				log.error(f"  -> Config value: '{config_name}' in project '{projectName}'. Error: {error}")

			if len(errors) > 0:
				raise exceptions.Abort(f"Aborted with {len(errors)} Configuration Errors!")
			elif strict and len(warnings) > 0:
				raise exceptions.Abort(f"Aborted with {len(warnings)} Configuration Warnings in 'strict' mode!")

		def tempDir(siteDir: str, tempDir:str, projectName: str) ->str:
			tempDoxyDir = PurePath.joinpath(Path(siteDir), Path(tempDir), Path(projectName))
			tempDoxyDir.mkdir(parents=True, exist_ok=True)
			return str(tempDoxyDir)

		self.doxygen = {}
		self.generatorBase = {}
		self.projects = self.config["projects"]

		log.info(f"Start plugin {pluginName}")

		for projectName in self.projects:
			self.proData = self.projects.get(projectName)
			log.info(f"-> Start project '{projectName}'")

			# Check project config -> raise exceptions
			checkConfig(self.config_project, self.proData, config['strict'])

			if self.config.get("save-api"):
				tempDirApi = tempDir("", self.config.get("save-api"), "")
			else:
				tempDirApi = tempDir(config['site_dir'], "assets/.doxy/", projectName)

			# Check scr changes -> run Doxygen
			doxygenRun = DoxygenRun(self.proData.get('src-dirs'), tempDirApi, self.proData.get('doxy-cfg', {}))
			if doxygenRun.checkAndRun():
				log.info("  -> generating Doxygen filese")
			else:
				log.info("  -> skip generating Doxygen files (nothing changes)")

			self.debug = self.config.get('debug', False)

			# Parse XML to bacic structure
			cache = Cache()
			parser = XmlParser(cache=cache, debug=self.debug)

			# Parse bacic structure to recursive Nodes
			self.doxygen[projectName] = Doxygen(doxygenRun.path, parser=parser, cache=cache, debug=self.debug)

			# Print parsed files
			if self.debug:
				self.doxygen[projectName].printStructure()

			# Prepare generator for future use (GeneratorAuto, SnippetGenerator)
			self.generatorBase[projectName] = GeneratorBase(ignore_errors=self.config["ignore-errors"])

			if self.config["full-doc"] and self.proData.get("full-doc", True):
				generatorAuto = GeneratorAuto(
					generatorBase=self.generatorBase[projectName],
					tempDoxyDir=tempDirApi,
					siteDir=config['site_dir'],
					apiPath=projectName,
					doxygen=self.doxygen[projectName],
					useDirectoryUrls=config['use_directory_urls'],
					debug=self.debug
				)

				# generate automatic documentation and append files into files
				generatorAuto.fullDoc()

				generatorAuto.summary()

				for file in generatorAuto.fullDocFiles:
					files.append(file)
		return files

	def on_page_markdown(
			self,
			markdown: str,
			page: pages.Page,
			config: base.Config,
			files: files.Files,
	) -> str:
		generatorSnippets = GeneratorSnippets(
			markdown=markdown,
			generatorBase=self.generatorBase,
			doxygen=self.doxygen,
			useDirectoryUrls=config['use_directory_urls'],
			page = page,
		debug=self.debug
		)

		return generatorSnippets.generate()

# def on_pre_build(self, config):

# def on_serve(self, server):
#     return server
#
# def on_files(self, files: files.Files, config):
#     return files

# def on_nav(self, nav, config, files):
#     return nav
#
# def on_env(self, env, config, files):
#     return env
#
# def on_config(self, config):
#     return config
#
# def on_post_build(self, config):
#     return
#
# def on_pre_template(self, template, template_name, config):
#     return template
#
# def on_template_context(self, context, template_name, config):
#     return context
#
# def on_post_template(self, output_content, template_name, config):
#     return output_content
#
# def on_pre_page(self, page: pages.Page, config, files: files.Files):
#     return page
#
# def on_page_read_source(self, page: pages.Page, config):
#     return
#
# def on_page_markdown(self, markdown, page, config, files):
#     return markdown
#
# def on_page_content(self, html, page, config, files):
#     return html
#
# def on_page_context(self, context, page, config, nav):
#     return context
#
# def on_post_page(self, output_content, page, config):
#     return output_content
