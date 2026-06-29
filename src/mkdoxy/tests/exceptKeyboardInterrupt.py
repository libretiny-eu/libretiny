import time

if __name__ == '__main__':
	try:
		while True:
			time.sleep(1)
			print("Hi")
	except KeyboardInterrupt:
		print("End KeyboardInterrupt")

	print("After KeyboardInterrupt")