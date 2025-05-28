class Cache:
	def __init__(self):
		self.cache = {}

	def add(self, key: str, value):
		self.cache[key] = value

	def get(self, key: str):
		if key in self.cache:
			return self.cache[key]
		else:
			raise IndexError('Key: ' + key + ' not found in cache!')
