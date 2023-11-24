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
    dataA = 5000
    data = 6000
    retry = 10
    try:
        while (dataA & 0xfffc) != (data & 0xfffc) and retry > 0:
            dataA = data
            retry -= 1
            data = bus.read_word_data(DEVICE_ADDRESS + stack, ADC_VAL_MV_ADD + 2 * (channel - 1))
        val = data / 1000.0
    except Exception as e:
        bus.close()
        raise Exception(e)
    bus.close()
    if retry == 0:
        raise Exception('Spurious read detected')
    return val


def getAdcRaw(stack, channel):
    data = 0
    if stack < 0 or stack > 7:
        raise ValueError('Invalid stack level')
    if channel < 1 or channel > 8:
        raise ValueError('Invalid channel number')
    ADC_VAL_RAW_ADD = 8
    bus = smbus.SMBus(1)
    dataA = 5000
    data = 6000
    retry = 10
    try:
        while (dataA & 0xfffc) != (data & 0xfffc) and retry > 0:
            dataA = data
            retry -= 1
            data = bus.read_word_data(DEVICE_ADDRESS + stack, ADC_VAL_RAW_ADD + 2 * (channel - 1));
        val = data;
    except Exception as e:
        bus.close()
        raise Exception(e)
    bus.close()
    if retry == 0:
        raise Exception('Spurious read detected')
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
    RELAY_VAL_ADD = 0
    valA = 257
    val = 258
    retry = 10
    try:
        while valA != val and retry > 0:
            valA = val
            retry -= 1
            val = bus.read_byte_data(DEVICE_ADDRESS + stack, RELAY_VAL_ADD)
    except Exception as e:
        bus.close()
        raise Exception(e)
    bus.close()
    if retry == 0:
        raise Exception('Spurious read detected')
    return val


def getRelayCh(stack, channel):
    if stack < 0 or stack > 7:
        raise ValueError('Invalid stack level')
    if channel < 1 or channel > 8:
        raise ValueError('Invalid channel number')
    val = getRelays(stack)
    if val < 0:
        return -1
    return (val >> (channel - 1)) & 1


def getOptoCh(stack, channel):
    if stack < 0 or stack > 7:
        raise ValueError('Invalid stack level')
    if channel < 1 or channel > 8:
        raise ValueError('Invalid channel number')
    bus = smbus.SMBus(1)
    OPTO_IN_ADD = 3
    valA = 257
    val = 258
    retry = 10
    try:
      while valA != val and retry > 0:
        valA = val
        retry -= 1
        val = bus.read_byte_data(DEVICE_ADDRESS + stack, OPTO_IN_ADD)
    except Exception as e:
        bus.close()
        raise Exception(e)
    bus.close()
    if retry == 0:
        raise Exception('Spurious read detected')
    if val & (1 << (channel - 1)):
        return 1
    else:
        return 0


def getOpto(stack):
    if stack < 0 or stack > 7:
        raise ValueError('Invalid stack level')
    bus = smbus.SMBus(1)
    OPTO_IN_ADD = 3
    valA = 257
    val = 258
    retry = 10
    try:
        while valA != val and retry > 0:
            valA = val
            retry -= 1
            val = bus.read_byte_data(DEVICE_ADDRESS + stack, OPTO_IN_ADD)
    except Exception as e:
        bus.close()
        raise Exception(e)
    bus.close()
    if retry == 0:
        raise Exception('Spurious read detected')
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
        raise Exception(e)
    bus.close()
    return 1


def getGpio(stack):
    if stack < 0 or stack > 7:
        raise ValueError('Invalid stack level')
    bus = smbus.SMBus(1)
    GPIO_VAL_ADD = 4
    valA = 257
    val = 258
    retry = 10
    try:
        while valA != val and retry > 0:
            valA = val
            retry -= 1
            val = bus.read_byte_data(DEVICE_ADDRESS + stack, GPIO_VAL_ADD)
    except Exception as e:
        bus.close()
        raise Exception(e)
    bus.close()
    if retry == 0:
        raise Exception('Spurious read detected')
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

def cfgOptoEdgeCount(stack, channel, state):
    if stack < 0 or stack > 7:
        raise ValueError('Invalid stack level')
    if channel < 1 or channel > 8:
        raise ValueError('Invalid channel number')
    EDGE_NONE = 0
    EDGE_FALLING = 2
    EDGE_RISING = 1
    if state < EDGE_NONE or state > EDGE_FALLING + EDGE_RISING:
        raise ValueError('Invalid edge type 0-none, 1-rising, 2-falling, 3-both')
    I2C_MEM_OPTO_IT_RISING_ADD = 56
    I2C_MEM_OPTO_IT_FALLING_ADD = 57
    bus = smbus.SMBus(1)
    try:
        rising = bus.read_byte_data(DEVICE_ADDRESS + stack, I2C_MEM_OPTO_IT_RISING_ADD)
        falling = bus.read_byte_data(DEVICE_ADDRESS + stack, I2C_MEM_OPTO_IT_FALLING_ADD)
        if state & EDGE_FALLING:
            falling |= 1 << (channel - 1)
        else:
            falling &= ~(1 << (channel - 1))
        if state & EDGE_RISING:
            rising |= 1 << (channel - 1)
        else:
            rising &= ~(1 << (channel -1))
        bus.write_byte_data(DEVICE_ADDRESS + stack,I2C_MEM_OPTO_IT_RISING_ADD, rising)
        bus.write_byte_data(DEVICE_ADDRESS + stack, I2C_MEM_OPTO_IT_FALLING_ADD, falling);

    except Exception as e:
        bus.close()
        return -1
    bus.close()
    return 1

def getOptoCount(stack, channel):
    if stack < 0 or stack > 7:
        raise ValueError('Invalid stack level')
    if channel < 1 or channel > 8:
        raise ValueError('Invalid channel number')
    I2C_MEM_OPTO_EDGE_COUNT_ADD = 128
    bus = smbus.SMBus(1)
    try:
        buff = bus.read_i2c_block_data(DEVICE_ADDRESS + stack, I2C_MEM_OPTO_EDGE_COUNT_ADD + 4 * (channel - 1), 4)
        count = buff[0] + buff[1] * 0x100 + buff[2] * 0x10000 + buff[3] * 0x1000000
    except Exception as e:
        bus.close()
        return -1
    bus.close()
    return count

def rstOptoCount(stack, channel):
    if stack < 0 or stack > 7:
        raise ValueError('Invalid stack level')
    if channel < 1 or channel > 8:
        raise ValueError('Invalid channel number')
    I2C_MEM_OPTO_CNT_RST_ADD = 60
    bus = smbus.SMBus(1)
    try:
        bus.write_byte_data(DEVICE_ADDRESS + stack, I2C_MEM_OPTO_CNT_RST_ADD, channel)
    except Exception as e:
        bus.close()
        return -1
    bus.close()
    return 1

def cfgOptoEncoder(stack, channel, state):
    if stack < 0 or stack > 7:
        raise ValueError('Invalid stack level')
    if channel < 1 or channel > 4:
        raise ValueError('Invalid channel number')
    if state < 0 or state > 1:
        raise ValueError('Invalid state value 0-off, 1-on')
    I2C_MEM_OPTO_ENC_ENABLE_ADD = 70
    bus = smbus.SMBus(1)
    try:
        encoders = bus.read_byte_data(DEVICE_ADDRESS + stack, I2C_MEM_OPTO_ENC_ENABLE_ADD)
        if state == 1:
            encoders |= 1 << (channel - 1)
        else:
            encoders &= ~(1 << (channel - 1))
        bus.write_byte_data(DEVICE_ADDRESS + stack, I2C_MEM_OPTO_ENC_ENABLE_ADD, encoders)
    except Exception as e:
        bus.close()
        return -1
    bus.close()
    return 1

def getOptoEncoderCount(stack, channel):
    if stack < 0 or stack > 7:
        raise ValueError('Invalid stack level')
    if channel < 1 or channel > 4:
        raise ValueError('Invalid channel number')
    I2C_MEM_OPTO_ENC_COUNT_ADD = 187
    bus = smbus.SMBus(1)
    try:
        buff = bus.read_i2c_block_data(DEVICE_ADDRESS + stack, I2C_MEM_OPTO_ENC_COUNT_ADD + 4 * (channel - 1), 4)
        count = struct.unpack('i', bytearray(buff)) #int.from_bytes(buff, byteorder='big', signed=True)
    except Exception as e:
        bus.close()
        raise e
    bus.close()
    return count

def resetOptoEncoderCount(stack, channel):
    if stack < 0 or stack > 7:
        raise ValueError('Invalid stack level')
    if channel < 1 or channel > 4:
        raise ValueError('Invalid channel number')
    I2C_MEM_OPTO_ENC_CNT_RST_ADD = 72
    bus = smbus.SMBus(1)
    try:
        bus.write_byte_data(DEVICE_ADDRESS + stack, I2C_MEM_OPTO_ENC_CNT_RST_ADD, channel)
    except Exception as e:
        bus.close()
        raise e
    bus.close()
    return 1


def owbGetTemp(stack, channel):
    I2C_MEM_1WB_DEV = 211
    I2C_MEM_1WB_T1 = 222
    if stack < 0 or stack > 7:
        raise ValueError('Invalid stack level')
    bus = smbus.SMBus(1)
    temp = 0
    try:
        nr = bus.read_byte_data(DEVICE_ADDRESS + stack, I2C_MEM_1WB_DEV)
        if channel > nr or channel < 1:
            bus.close()
            raise ValueError('Invalid channel number')
        data = bus.read_word_data(DEVICE_ADDRESS + stack, I2C_MEM_1WB_T1 + 2 * (channel - 1))
        temp = data / 100
    except Exception as e:
        bus.close()
        raise e
    bus.close()
    return temp

def owbGetSnsNo(stack):
    I2C_MEM_1WB_DEV = 211
    if stack < 0 or stack > 7:
        raise ValueError('Invalid stack level')
    bus = smbus.SMBus(1)
    try:
         nr = bus.read_byte_data(DEVICE_ADDRESS + stack, I2C_MEM_1WB_DEV)
    except Exception as e:
        bus.close()
        raise e
    bus.close()
    return nr

def owbScan(stack):
    I2C_MEM_1WB_START_SEARCH = 221
    if stack < 0 or stack > 7:
        raise ValueError('Invalid stack level')
    bus = smbus.SMBus(1)
    try:
        bus.write_byte_data(DEVICE_ADDRESS + stack, I2C_MEM_1WB_START_SEARCH,1 )
    except Exception as e:
        bus.close()
        raise e
    bus.close()   
    return 1

def owbGetSnsId(stack, channel):
    I2C_MEM_1WB_ROM_CODE_IDX =212
    I2C_MEM_1WB_ROM_CODE = 213
    ROM_CODE_LEN = 8
    I2C_MEM_1WB_T1 = 222
    I2C_MEM_1WB_START_SEARCH = 221
    I2C_MEM_1WB_DEV = 211
    if stack < 0 or stack > 7:
        raise ValueError('Invalid stack level')
    bus = smbus.SMBus(1)
    temp = 0
    try:
        nr = bus.read_byte_data(DEVICE_ADDRESS + stack, I2C_MEM_1WB_DEV)
        if channel > nr or channel < 1:
            bus.close()
            raise ValueError('Invalid channel number') 
        bus.write_byte_data(DEVICE_ADDRESS + stack, I2C_MEM_1WB_ROM_CODE_IDX, channel - 1)
        buff = bus.read_i2c_block_data(DEVICE_ADDRESS + stack, I2C_MEM_1WB_ROM_CODE, 8)
    except Exception as e:
        bus.close()
        raise e
    bus.close()
    return buff


