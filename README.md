[![ioplus-rpi](res/sequent.jpg)](https://www.sequentmicrosystems.com)

# ioplus-rpi

Command Line, Python, and Node-Red for [Home Automation Stackable Card for Raspberry Pi](https://sequentmicrosystems.com/products/raspberry-pi-home-automation-card)

![IO-PLUS](res/IO-PLUS_V3.jpg)

## Setup

Enable Raspberry Pi I2C communication by opening a terminal and typing:
```bash
~$ sudo raspi-config
```
Go to the *Interface Options* menu then *I2C* and enable the port.

## Usage

```bash
cd
git clone https://github.com/SequentMicrosystems/ioplus-rpi.git
cd ioplus-rpi/
sudo make install
```

Now you can access all the functions of the relays board through the command "ioplus". Use -h option for help:
```bash
ioplus -h
```

If you clone the repository any update can be made with the following commands:

```bash
cd
cd ioplus-rpi/  
git pull
sudo make install
``` 

## [Firmware Update](update/README.md)

## [Python Library](python/README.md)

## [Domoticz Plugin](https://github.com/SequentMicrosystems/ioplus-rpi/tree/master/domoticz)

## [Node-Red](node-red/README.md) example based on exec node.

## [Node-Red nodes](node-red-contrib-sm-ioplus)

## [OpenPLC Usage](OpenPLC.md)
