[![ioplus-rpi](res/sequent.jpg)](https://www.sequentmicrosystems.com)

# ioplus-rpi

Command Line, Python, and Node-Red for [Home Automation Stackable Card for Raspberry Pi](https://sequentmicrosystems.com/product/raspberry-pi-stackable-card-for-home-automation/)

![IO-PLUS](res/IO-PLUS.jpg)

## Setup

Enable Raspberry Pi I2C communication:
```bash
~$ sudo raspi-config
```

## Usage

```bash
~$ git clone https://github.com/SequentMicrosystems/ioplus-rpi.git
~$ cd ioplus-rpi/
~/ioplus-rpi$ sudo make install
```

Now you can access all the functions of the relays board through the command "ioplus". Use -h option for help:
```bash
~$ ioplus -h
```

If you clone the repository any update can be made with the following commands:

```bash
~$ cd ioplus-rpi/  
~/ioplus-rpi$ git pull
~/ioplus-rpi$ sudo make install
``` 

## [Firmware Update](update/README.md)

## [Python Library](python/README.md)

## [Node-Red](node-red/README.md)
