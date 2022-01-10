import libioplus as home
import RPi.GPIO as GPIO
import time
import sys

GPIO.setmode(GPIO.BCM)
GPIO.setwarnings(False)
pbPin = 26
GPIO.setup(pbPin, GPIO.IN, pull_up_down=GPIO.PUD_UP)


if __name__ == "__main__": 
    ch = 1
    if len(sys.argv) > 1:
        ch = int(sys.argv[1])
        if ch > 8:
            ch = 8
        if ch < 1:
            ch = 1
    state = 0
    try:
        while 1:
            switch = GPIO.input(pbPin)
            if switch != state:
                home.setRelayCh(0, ch, 1 - switch)
                state = switch
            time.sleep(0.1)

    except KeyboardInterrupt:
        home.setRelayCh(0, ch, 0)
        GPIO.cleanup()
        pass
