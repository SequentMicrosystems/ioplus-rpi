# libioplus

This is the python library to control the [io-plus](https://sequentmicrosystems.com/index.php?route=product/product&path=33&product_id=42) Raspberry Pi Stackable Card for Home Automation.

## Install

```bash
~$ sudo apt-get update
~$ sudo apt-get install build-essential python-pip python-dev python-smbus git
~$ git clone https://github.com/SequentMicrosystems/ioplus-rpi.git
~$ cd ioplus-rpi/python/ioplus/
~/ioplus-rpi/python/ioplus$ sudo python setup.py install
```
If you use python3.x repace the last line with:
```
~/ioplus-rpi/python/ioplus$ sudo python3 setup.py install
```
## Update

```bash
~$ cd ioplus-rpi/
~/ioplus-rpi$ git pull
~$ cd ioplus-rpi/python/ioplus/
~/ioplus-rpi/python/ioplus$ sudo python setup.py install
```
If you use python3.x repace the last line with:
```
~/ioplus-rpi/python/ioplus$ sudo python3 setup.py install
```
## Usage 

Now you can import the megaio library and use its functions. To test, read relays status from the board with stack level 0:

```bash
~$ python
Python 2.7.9 (default, Sep 17 2016, 20:26:04)
[GCC 4.9.2] on linux2
Type "help", "copyright", "credits" or "license" for more information.
>>> import libioplus
>>> libioplus.getAdcV(0, 1)
2.654391
>>>
```

## Functions


