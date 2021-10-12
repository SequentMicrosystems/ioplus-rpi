import libioplus as iop
import time
import sys
stack = 0
channel = 1
#configure the encoder
iop.cfgOptoEncoder(stack, channel, 1)
count = iop.getOptoEncoderCount(stack, channel)
print("Counter before loop " + str(count))
#reset the counter
iop.resetOptoEncoderCount(stack, channel)
oldCount = -1
try:
    while True:
        count = iop.getOptoEncoderCount(stack, channel)
        if count >= 0 and count != oldCount:
            oldCount = count
            sys.stdout.write("\r%d    " % oldCount)
            sys.stdout.flush()
        time.sleep(0.2)
except KeyboardInterrupt:
	pass