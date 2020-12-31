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

**value** - 0 = relay on, 1 = relay off


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


