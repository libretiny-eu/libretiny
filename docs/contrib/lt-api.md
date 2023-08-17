# API functions guide

The [LibreTiny C API](../dev/lt-api.md) functions are split between three types: common, weak and family.

- Common functions are implemented in the base, common core and are the same between all families.
- Weak functions are provided in the common core, but can (and sometimes should) be overridden by family cores. They sometimes provide usable default implementations (which *can* be overriden to provide e.g. a better way to do something), otherwise they're empty (e.g. if a family doesn't support such a feature).
- Family functions are not provided in the common core and have to be implemented in the family core.

A quick outline of all available functions and their types:

{%
	include-markdown "./lt-api-functions.md"
%}
