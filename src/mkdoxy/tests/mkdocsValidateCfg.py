from mkdocs.config import base, config_options, Config


if __name__ == '__main__':
	config_scheme = (
		('doxygen-source', config_options.Type(str, default='')),
		('api-path', config_options.Type(str, default='api')),
		('target', config_options.Type(str, default='mkdocs')),
		('full-doc', config_options.Type(bool, default=False)),
		('hints', config_options.Type(bool, default=False)),
		('debug', config_options.Type(bool, default=False)),
		('ignore-errors', config_options.Type(bool, default=False)),
		('link-prefix', config_options.Type(str, default='')),
	)
	
	config_ok = {'doxygen-source': 'src-esp', 'api-path': 'api', 'target': 'mkdocs', 'full-doc': True, 'hints': True, 'debug': False, 'ignore-errors': False}
	config_false = {'doxygen-source': False, 'api-path': 'api', 'target': 'mkdocs', 'full-doc': 'asd', 'hints': True, 'debug': False, 'ignore-errors': False}

	cfg = Config(config_scheme, '')
	cfg.load_dict(config_false)
	# cfg.load_dict(config_ok)
	print(cfg.validate())