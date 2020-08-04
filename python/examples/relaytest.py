import libioplus as io
import time
import sys

if __name__ == "__main__":
	if len(sys.argv) != 2 :
		print("Invalid params number, Usage: python relaytest.py <stack level>")
		sys.exit(-1)
	stackLevel = int(sys.argv[1])	
	try:
		while True:
			for i in range(8):
				io.setRelayCh(stackLevel, i +1, 1)
				time.sleep(0.1)
			for i in range(8):
				io.setRelayCh(stackLevel, i +1, 0)
				time.sleep(0.1)
			time.sleep(1)		
	except KeyboardInterrupt:
		pass
