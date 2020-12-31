import smbus
import struct

# bus = smbus.SMBus(1)    # 0 = /dev/i2c-0 (port I2C0), 1 = /dev/i2c-1 (port I2C1)

DEVICE_ADDRESS = 0x28  # 7 bit address (will be left shifted to add the read write bit)


def getAdcV(stack, channel):
    data = 0
    if stack < 0 or stack > 7:
        raise ValueError('Invalid stack level')
    if channel < 1 or channel > 8:
        raise ValueError('Invalid channel number')
    ADC_VAL_MV_ADD = 24
    bus = smbus.SMBus(1)
    try:
        data = bus.read_word_data(DEVICE_ADDRESS + stack, ADC_VAL_MV_ADD + 2 * (channel - 1));
        val = data / 1000.0;
    except Exception as e:
        val = -1
    bus.close()
    return val


def getAdcRaw(stack, channel):
    data = 0
    if stack < 0 or stack > 7:
        raise ValueError('Invalid stack level')
    if channel < 1 or channel > 8:
        raise ValueError('Invalid channel number')
    ADC_VAL_RAW_ADD = 8
    bus = smbus.SMBus(1)
    try:
        data = bus.read_word_data(DEVICE_ADDRESS + stack, ADC_VAL_RAW_ADD + 2 * (channel - 1));
        val = data;
    except Exception as e:
        val = -1
    bus.close()
    return val


def setDacV(stack, channel, value):
    if stack < 0 or stack > 7:
        raise ValueError('Invalid stack level')
    if channel < 1 or channel > 4:
        raise ValueError('Invalid channel number')
    if value < 0:
        value = 0;

    if value > 10:
        value = 10;

    raw = int(value * 1000)
    bus = smbus.SMBus(1)
    DAC_VAL_MV_ADD = 40

    try:
        bus.write_word_data(DEVICE_ADDRESS + stack, DAC_VAL_MV_ADD + 2 * (channel - 1), raw)
    except Exception as e:
        print(e)
        bus.close()
        return -1
    bus.close()
    return 1


def setOdPwm(stack, channel, value):
    if stack < 0 or stack > 7:
        raise ValueError('Invalid stack level')
    if channel < 1 or channel > 4:
        raise ValueError('Invalid channel number')
    bus = smbus.SMBus(1)
    if value < 0:
        value = 0
    if value > 10000:
        value = 10000
    OD_PWM_VAL_RAW_ADD = 48
    try:
        bus.write_word_data(DEVICE_ADDRESS + stack, OD_PWM_VAL_RAW_ADD + 2 * (channel - 1), value)
    except Exception as e:
        bus.close()
        return -1
    bus.close()
    return 1


def setRelayCh(stack, channel, value):
    if stack < 0 or stack > 7:
        raise ValueError('Invalid stack level')
    if channel < 1 or channel > 8:
        raise ValueError('Invalid channel number')
    bus = smbus.SMBus(1)
    RELAY_SET_ADD = 1
    RELAY_CLR_ADD = 2
    if value == 0:
        try:
            bus.write_byte_data(DEVICE_ADDRESS + stack, RELAY_CLR_ADD, channel)
        except Exception as e:
            bus.close()
            return -1
    else:
        try:
            bus.write_byte_data(DEVICE_ADDRESS + stack, RELAY_SET_ADD, channel)
        except Exception as e:
            bus.close()
            return -1
    bus.close()
    return 1


def setRelays(stack, value):
    if stack < 0 or stack > 7:
        raise ValueError('Invalid stack level')
    if value < 0 or value > 255:
        raise ValueError('Invalid relays value')
    bus = smbus.SMBus(1)
    RELAY_VAL_ADD = 0;
    try:
        bus.write_byte_data(DEVICE_ADDRESS + stack, RELAY_VAL_ADD, value)
    except Exception as e:
        bus.close()
        return -1
    bus.close()

def getRelays(stack):
    if stack < 0 or stack > 7:
        raise ValueError('Invalid stack level')
    bus = smbus.SMBus(1)
    RELAY_VAL_ADD = 0;
    try:
        val = bus.read_word_data(DEVICE_ADDRESS + stack, RELAY_VAL_ADD)
    except Exception as e:
        bus.close()
        return -1
    bus.close()
    return val

def getRelayCh(stack, channel):
    if stack < 0 or stack > 7:
        raise ValueError('Invalid stack level')
    if channel < 1 or channel > 8:
        raise ValueError('Invalid channel number')
    val=getRelays(stack)
    if val<0:
        return -1
    return (val>>(channel-1)) & 1

def getOptoCh(stack, channel):
    if stack < 0 or stack > 7:
        raise ValueError('Invalid stack level')
    if channel < 1 or channel > 8:
        raise ValueError('Invalid channel number')
    bus = smbus.SMBus(1)
    OPTO_IN_ADD = 3
    try:
        val = bus.read_byte_data(DEVICE_ADDRESS + stack, OPTO_IN_ADD)
    except Exception as e:
        bus.close()
        return -1
    bus.close()
    if val & (1 << channel):
        return 1
    else:
        return 0


def getOpto(stack):
    if stack < 0 or stack > 7:
        raise ValueError('Invalid stack level')
    bus = smbus.SMBus(1)
    OPTO_IN_ADD = 3
    try:
        val = bus.read_byte_data(DEVICE_ADDRESS + stack, OPTO_IN_ADD)
    except Exception as e:
        bus.close()
        return -1
    bus.close()
    return val


def setGpioDir(stack, dir):
    if stack < 0 or stack > 7:
        raise ValueError('Invalid stack level')
    if dir < 0 or dir > 15:
        raise ValueError('Invalid channel direction register value (allow 0..15)')
    bus = smbus.SMBus(1)
    GPIO_DIR_ADD = 7
    try:
        bus.write_byte_data(DEVICE_ADDRESS + stack, GPIO_DIR_ADD, dir)
    except Exception as e:
        bus.close()
        return -1
    bus.close()
    return 1


def getGpio(stack):
    if stack < 0 or stack > 7:
        raise ValueError('Invalid stack level')
    bus = smbus.SMBus(1)
    GPIO_VAL_ADD = 4
    try:
        val = bus.read_byte_data(DEVICE_ADDRESS + stack, GPIO_VAL_ADD)
    except Exception as e:
        bus.close()
        return -1
    bus.close()
    return val


def setGpioPin(stack, pin, val):
    if stack < 0 or stack > 7:
        raise ValueError('Invalid stack level')
    if pin < 1 or pin > 4:
        raise ValueError('Invalid pin number')
    bus = smbus.SMBus(1)
    GPIO_SET_ADD = 5
    GPIO_CLR_ADD = 6
    try:
        if val == 0:
            bus.write_byte_data(DEVICE_ADDRESS + stack, GPIO_CLR_ADD, pin)
        else:
            bus.write_byte_data(DEVICE_ADDRESS + stack, GPIO_SET_ADD, pin)
    except Exception as e:
        bus.close()
        return -1
    bus.close()
    return 1
