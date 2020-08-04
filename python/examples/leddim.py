import libioplus as io
import time

try:
	while True:
		for i in range(100):
			io.setOdPwm(0, 1, i * 100)
			time.sleep(0.02)
		for i in range(100):
			io.setOdPwm(0, 1, (99 - i) * 100)
			time.sleep(0.02)
		time.sleep(1)		
except KeyboardInterrupt:
	pass
io.setOdPwm(0, 1, 0)
