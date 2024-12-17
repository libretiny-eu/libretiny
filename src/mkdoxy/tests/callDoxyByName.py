class DoxyCall:
	def __init__(self):
		self.DOXY_CALL = {
			"class": self.doxyClass,
			"class.list":self.doxyClassList,
		}

	def callDoxyByName(self, name, config):
		if name in self.DOXY_CALL:
			funcName = self.DOXY_CALL[name]
			funcName(config)

	def doxyClass(self, config):
		print("doxyClassasd", config)

	def doxyClassList(self, config):
		print("doxyClassList", config)



if __name__ == "__main__":
	doxyCall = DoxyCall()
	doxyCall.callDoxyByName("class", ["asd"])
	doxyCall.callDoxyByName("class.list", ["list"])