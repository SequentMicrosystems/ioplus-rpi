[![ioplus-rpi](res/sequent.jpg)](https://www.sequentmicrosystems.com)

# libioplus

This is the python library to control the [Home Automation Stackable Card for Raspberry Pi](https://sequentmicrosystems.com/products/raspberry-pi-home-automation-card).

## Install

```bash
sudo pip install libioplus
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

## [libioplus documentation](ioplus/README.md)





# libioplus

## Functions

### getAdcV(stack, channel)

Return voltage readings on ADC channel (converted to volts)

**stack** - stack level, set with jumpers [0..7]

**channel** - ADC channel [1..8]


### getAdcRaw(stack, channel)

Return the raw readings on ADC channel (12bits, [0..4095]

**stack** - stack level, set with jumpers [0..7]

**channel** - ADC channel [1..8]


### setDacV(stack, channel, value)

Set output voltage on one DAC channel 

**stack** - stack level, set with jumpers [0..7]

**channel** - DAC channel [1..4]

**value** - DAC voltage [0..10]


### setOdPwm(stack, channel, value)

Set PWM value for one Open-Drain channel

**stack** - stack level, set with jumpers [0..7]

**channel** - OD channel [1..4]

**value** - PWM [0..10000]


### setRelayCh(stack, channel, value)

Set the output state of one relay

**stack** - stack level, set with jumpers [0..7]

**channel** - Relay nr [1..8]

**value** - 1 = relay on, 0 = relay off


### getRelayCh(stack, channel)

Return state of one relay

**stack** - stack level, set with jumpers [0..7]

**channel** - Relay nr [1..8]


### setRelays(stack, value)

Set all relays 8 bits value

**stack** - stack level, set with jumpers [0..7]

**value**  [0..255], 0 - all relays OFF; 255 - all relays ON


### getRelays(stack)

Return state of all relays, 8 bits value

**stack** - stack level, set with jumpers [0..7]


### getOptoCh(stack, channel)

Return one opto input channel state

**stack** - stack level, set with jumpers [0..7]

**channel** - Opto channel nr [1..8]


### getOpto(stack)

Return all opto input channels states as 8 bits value

**stack** - stack level, set with jumpers [0..7]


### setGpioDir(stack, dir)

Set all 4 GPIO direction 

**stack** - stack level, set with jumpers [0..7]

**dir** - 4 bits value : 0 - all output, 15 - all input


### getGpio(stack)

Return the GPIO state, 4 bits value

**stack** - stack level, set with jumpers [0..7]


### setGpioPin(stack, pin, val)

Set one GPIO pin state if his direction is output

**stack** - stack level, set with jumpers [0..7]

**pin** - pin number [1..4]

**val** - 0 = low; 1 - high

### cfgOptoEdgeCount(stack, channel, state)
Configure the edge counting on a opto coupled input channel

**stack** - stack level, set with jumpers [0..7]

**channel** - channel number [1..8]

**state** - 0 = disble; 1 - count rising edges; 2 - count falling edges; 3 - count both

### getOptoCount(stack, channel)
Read the opto edge counter on a channel

**stack** - stack level, set with jumpers [0..7]

**channel** - channel number [1..8]

Returns the counter value

### rstOptoCount(stack, channel)
Reset the edge counter.

**stack** - stack level, set with jumpers [0..7]

**channel** - channel number [1..8]

### cfgOptoEncoder(stack, channel, state)
Config the encoder inputs on optocoupled channels. The encoder can be connected on opto 1/2, 3/4, 5/6, 7/8 this means 4 encoders.

**stack** - stack level, set with jumpers [0..7]

**channel** - channel number [1..4]

**state** - 0- disable, 1 - enable

### getOptoEncoderCount(stack, channel)
Read the opto encoder counter

**stack** - stack level, set with jumpers [0..7]

**channel** - channel number [1..4]

Return 32 bits signed integer represents the counter

### resetOptoEncoderCount(stack, channel)
Reset the conter

**stack** - stack level, set with jumpers [0..7]

**channel** - channel number [1..4]

### owgGetTemp(stack, channel)
Read the temperature af one sensor

**stack** - stack level, set with jumpers [0..7]

**channel** - channel number [1..number of connected sensors]

Return temperature in degree Celsius readed from the selected sensor 

### owbGetSnsNo(stack)
Return the nomber of sensors connected to the one wire port

**stack** - stack level, set with jumpers [0..7]

### owbGetSnsId(stack, channel)
Return the 8 bytes ID of the selected sensor

**stack** - stack level, set with jumpers [0..7]

**channel** - channel number [1..number of connected sensors]

### owbScam(stack)
Start scaning procedure

**stack** - stack level, set with jumpers [0..7]


