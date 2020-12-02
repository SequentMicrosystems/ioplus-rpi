#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <math.h>

#include "comm.h"
#include "ioplus.h"

int gpioChSet(int dev, u8 channel, OutStateEnumType state)
{
	int resp;
	u8 buff[2];

	if ( (channel < CHANNEL_NR_MIN) || (channel > GPIO_CH_NR_MAX))
	{
		printf("Invalid GPIO nr!\n");
		return ERROR;
	}
	if (FAIL == i2cMem8Read(dev, I2C_MEM_GPIO_VAL_ADD, buff, 1))
	{
		return FAIL;
	}

	switch (state)
	{
	case OFF:
		buff[0] &= ~ (1 << (channel - 1));
		resp = i2cMem8Write(dev, I2C_MEM_GPIO_VAL_ADD, buff, 1);
		break;
	case ON:
		buff[0] |= 1 << (channel - 1);
		resp = i2cMem8Write(dev, I2C_MEM_GPIO_VAL_ADD, buff, 1);
		break;
	default:
		printf("Invalid GPIO state!\n");
		return ERROR;
		break;
	}
	return resp;
}

int gpioChGet(int dev, u8 channel, OutStateEnumType *state)
{
	u8 buff[2];

	if (NULL == state)
	{
		return ERROR;
	}

	if ( (channel < CHANNEL_NR_MIN) || (channel > GPIO_CH_NR_MAX))
	{
		printf("Invalid GPIO nr!\n");
		return ERROR;
	}

	if (FAIL == i2cMem8Read(dev, I2C_MEM_GPIO_VAL_ADD, buff, 1))
	{
		return ERROR;
	}

	if (buff[0] & (1 << (channel - 1)))
	{
		*state = ON;
	}
	else
	{
		*state = OFF;
	}
	return OK;
}

int gpioSet(int dev, int val)
{
	u8 buff[2];

	buff[0] = 0xff & val;

	return i2cMem8Write(dev, I2C_MEM_GPIO_VAL_ADD, buff, 1);
}

int gpioGet(int dev, int *val)
{
	u8 buff[2];

	if (NULL == val)
	{
		return ERROR;
	}
	if (FAIL == i2cMem8Read(dev, I2C_MEM_GPIO_VAL_ADD, buff, 1))
	{
		return ERROR;
	}
	*val = buff[0];
	return OK;
}

int gpioChDirSet(int dev, u8 channel, u8 state)
{
	int resp;
	u8 buff[2];

	if ( (channel < CHANNEL_NR_MIN) || (channel > GPIO_CH_NR_MAX))
	{
		printf("Invalid GPIO nr!\n");
		return ERROR;
	}
	if (FAIL == i2cMem8Read(dev, I2C_MEM_GPIO_DIR_ADD, buff, 1))
	{
		return FAIL;
	}

	switch (state)
	{
	case 0: //output
		buff[0] &= ~ (1 << (channel - 1));
		resp = i2cMem8Write(dev, I2C_MEM_GPIO_DIR_ADD, buff, 1);
		break;
	case 1: //input
		buff[0] |= 1 << (channel - 1);
		resp = i2cMem8Write(dev, I2C_MEM_GPIO_DIR_ADD, buff, 1);
		break;
	default:
		printf("Invalid GPIO state!\n");
		return ERROR;
		break;
	}
	return resp;
}

int gpioDirSet(int dev, int val)
{
	u8 buff[2];

	buff[0] = 0xff & val;

	return i2cMem8Write(dev, I2C_MEM_GPIO_DIR_ADD, buff, 1);
}

int gpioDirGet(int dev, int *val)
{
	u8 buff[2];

	if (NULL == val)
	{
		return ERROR;
	}
	if (FAIL == i2cMem8Read(dev, I2C_MEM_GPIO_DIR_ADD, buff, 1))
	{
		return ERROR;
	}
	*val = buff[0];
	return OK;
}

int gpioEdgeGet(int dev, u8 channel, u8 *val)
{
	u8 buff[2];
	u8 rising = 0;
	u8 falling = 0;

	if ( (NULL == val) || (channel < CHANNEL_NR_MIN)
		|| (channel > GPIO_CH_NR_MAX))
	{
		return ERROR;
	}
	if (FAIL == i2cMem8Read(dev, I2C_MEM_GPIO_EXT_IT_RISING_ADD, buff, 2))
	{
		return ERROR;
	}
	rising = buff[0];
	falling = buff[1];
	*val = ( ( (1 << (channel - 1)) & rising) == 0 ? 0 : 1);
	*val += ( ( (1 << (channel - 1)) & falling) == 0 ? 0 : 2);
	return OK;
}

int gpioEdgeSet(int dev, u8 channel, u8 val)
{
	u8 buff[2];
	u8 rising = 0;
	u8 falling = 0;

	if ( (channel < CHANNEL_NR_MIN) || (channel > GPIO_CH_NR_MAX))
	{
		return ERROR;
	}
	if (FAIL == i2cMem8Read(dev, I2C_MEM_GPIO_EXT_IT_RISING_ADD, buff, 2))
	{
		return ERROR;
	}
	rising = buff[0];
	falling = buff[1];
	if (val & 0x01) //check rising
	{
		rising |= 1 << (channel - 1);
	}
	else
	{
		rising &= ~ (1 << (channel - 1));
	}
	if (val & 0x02) //check falling
	{
		falling |= 1 << (channel - 1);
	}
	else
	{
		falling &= ~ (1 << (channel - 1));
	}
	buff[0] = rising;
	buff[1] = falling;
	if (FAIL == i2cMem8Write(dev, I2C_MEM_GPIO_EXT_IT_RISING_ADD, buff, 2))
	{
		return ERROR;
	}
	return OK;
}

int gpioCountGet(int dev, u8 channel, u32 *val)
{
	u8 buff[4];

	if ( (channel < CHANNEL_NR_MIN) || (channel > GPIO_CH_NR_MAX))
	{
		return ERROR;
	}
	if (FAIL
		== i2cMem8Read(dev,
			I2C_MEM_GPIO_EDGE_COUNT_ADD + COUNTER_SIZE * (channel - 1), buff,
			COUNTER_SIZE))
	{
		return ERROR;
	}
	memcpy(val, buff, COUNTER_SIZE);
	return OK;
}

int gpioCountReset(int dev, u8 channel)
{

	if ( (channel < CHANNEL_NR_MIN) || (channel > OPTO_IN_CH_NR_MAX))
	{
		return ERROR;
	}
	if(FAIL == i2cMem8Write(dev, I2C_MEM_GPIO_CNT_RST_ADD, &channel, 1))
	{
		return ERROR;
	}
	return OK;
}


int doGpioRead(int argc, char *argv[])
{
	int pin = 0;
	int val = 0;
	int dev = 0;
	OutStateEnumType state = STATE_COUNT;

	dev = doBoardInit(atoi(argv[1]));
	if (dev <= 0)
	{
		return ERROR;
	}

	if (argc == 4)
	{
		pin = atoi(argv[3]);
		if ( (pin < CHANNEL_NR_MIN) || (pin > GPIO_CH_NR_MAX))
		{
			printf("Gpio pin number value out of range!\n");
			return ARG_ERR;
		}

		if (OK != gpioChGet(dev, pin, &state))
		{
			printf("Fail to read!\n");
			return ERROR;
		}
		if (state != 0)
		{
			printf("1\n");
		}
		else
		{
			printf("0\n");
		}
	}
	else if (argc == 3)
	{
		if (OK != gpioGet(dev, &val))
		{
			printf("Fail to read!\n");
			return ERROR;
		}
		printf("%d\n", val);
	}
	else
	{
		return ARG_CNT_ERR;
	}
	return OK;
}

int doGpioWrite(int argc, char *argv[])
{
	int pin = 0;
	OutStateEnumType state = STATE_COUNT;
	int val = 0;
	int dev = 0;
	OutStateEnumType stateR = STATE_COUNT;
	int retry = 0;
	int direction = 0x0f;

	if ( (argc != 5) && (argc != 4))
	{
		return ARG_CNT_ERR;
	}

	dev = doBoardInit(atoi(argv[1]));
	if (dev <= 0)
	{
		return ERROR;
	}
	if (argc == 5)
	{
		pin = atoi(argv[3]);
		if ( (pin < CHANNEL_NR_MIN) || (pin > GPIO_CH_NR_MAX))
		{
			printf("Gpio pin number value out of range\n");
			return ARG_ERR;
		}

		/**/if ( (strcasecmp(argv[4], "up") == 0)
			|| (strcasecmp(argv[4], "on") == 0))
			state = ON;
		else if ( (strcasecmp(argv[4], "down") == 0)
			|| (strcasecmp(argv[4], "off") == 0))
			state = OFF;
		else
		{
			if ( (atoi(argv[4]) >= STATE_COUNT) || (atoi(argv[4]) < 0))
			{
				printf("Invalid gpio state!\n");
				return ARG_ERR;;
			}
			state = (OutStateEnumType)atoi(argv[4]);
		}

		if (OK != gpioDirGet(dev, &direction))
		{
			printf("Fail to read gpio direction \n");
			return ERROR;
		}

		if ( (1 << (pin - 1)) & direction) //pin is input
		{
			printf("Fail to write gpio pin, is input\n");
			return ERROR;
		}
		retry = RETRY_TIMES;

		while ( (retry > 0) && (stateR != state))
		{
			if (OK != gpioChSet(dev, pin, state))
			{
				printf("Fail to write gpio\n");
				return ERROR;
			}
			if (OK != gpioChGet(dev, pin, &stateR))
			{
				printf("Fail to read gpio\n");
				return ERROR;
			}
			retry--;
		}
#ifdef DEBUG_I
		if(retry < RETRY_TIMES)
		{
			printf("retry %d times\n", 3-retry);
		}
#endif
		if (retry == 0)
		{
			printf("Fail to write gpio pin\n");
			return ERROR;
		}
	}
	else
	{
		val = atoi(argv[3]);
		if (val < 0 || val > 0x0f)
		{
			printf("Invalid gpio value\n");
			return ARG_ERR;
		}

		if (OK != gpioSet(dev, val))
		{
			printf("Fail to write GPIO\n");
			return ERROR;
		}
	}
	return OK;
}


int doGpioDirRead(int argc, char *argv[])
{
	int pin = 0;
	int val = 0;
	int dev = 0;

	dev = doBoardInit(atoi(argv[1]));
	if (dev <= 0)
	{
		return ERROR;
	}

	if (argc == 4)
	{
		pin = atoi(argv[3]);
		if ( (pin < CHANNEL_NR_MIN) || (pin > GPIO_CH_NR_MAX))
		{
			printf("Gpio pin number value out of range!\n");
			return ARG_ERR;
		}

		if (OK != gpioDirGet(dev, &val))
		{
			printf("Fail to read!\n");
			return ERROR;
		}
		if ( (val & (1 << (pin - 1))) != 0)
		{
			printf("1\n");
		}
		else
		{
			printf("0\n");
		}
	}
	else if (argc == 3)
	{
		if (OK != gpioDirGet(dev, &val))
		{
			printf("Fail to read!\n");
			return ERROR;
		}
		printf("%d\n", val);
	}
	else
	{
		return ARG_CNT_ERR;
	}
	return OK;
}

int doGpioDirWrite(int argc, char *argv[])
{
	int pin = 0;
	OutStateEnumType state = STATE_COUNT;
	int val = 0;
	int dev = 0;

	if ( (argc != 5) && (argc != 4))
	{
		return ARG_CNT_ERR;
	}

	dev = doBoardInit(atoi(argv[1]));
	if (dev <= 0)
	{
		return ERROR;
	}
	if (argc == 5)
	{
		pin = atoi(argv[3]);
		if ( (pin < CHANNEL_NR_MIN) || (pin > GPIO_CH_NR_MAX))
		{
			printf("Gpio pin number value out of range\n");
			return ARG_ERR;
		}

		/**/if ( (strcasecmp(argv[4], "in") == 0)
			|| (strcasecmp(argv[4], "input") == 0))
			state = ON;
		else if ( (strcasecmp(argv[4], "out") == 0)
			|| (strcasecmp(argv[4], "output") == 0))
			state = OFF;
		else
		{
			if ( (atoi(argv[4]) >= STATE_COUNT) || (atoi(argv[4]) < 0))
			{
				printf("Invalid gpio direction!\n");
				return ARG_ERR;
			}
			state = (OutStateEnumType)atoi(argv[4]);
		}

		if (OK != gpioChDirSet(dev, pin, state))
		{
			printf("Fail to write gpio direction \n");
			return ERROR;
		}

	}
	else if (argc == 4)
	{
		val = atoi(argv[3]);
		if (val < 0 || val > 0x0f)
		{
			printf("Invalid gpio direction value\n");
			return ARG_ERR;
		}

		if (OK != gpioDirSet(dev, val))
		{
			printf("Fail to write gpio direction \n");
			return ERROR;
		}
	}
	else
	{
		return ARG_CNT_ERR;
	}
	return OK;
}

int doGpioEdgeWrite(int argc, char *argv[])
{
	int pin = 0;
	u8 state = 0;
	int dev = 0;

	if ( (argc != 5))
	{
		return ARG_CNT_ERR;
	}

	dev = doBoardInit(atoi(argv[1]));
	if (dev <= 0)
	{
		return ERROR;
	}

	pin = atoi(argv[3]);
	if ( (pin < CHANNEL_NR_MIN) || (pin > GPIO_CH_NR_MAX))
	{
		printf("Gpio pin number value out of range\n");
		return ARG_ERR;
	}

	/**/
	if ( (strcasecmp(argv[4], "none") == 0))
		state = 0;

	else if ( (strcasecmp(argv[4], "up") == 0)
		|| (strcasecmp(argv[4], "rising") == 0))
		state = 1;
	else if ( (strcasecmp(argv[4], "down") == 0)
		|| (strcasecmp(argv[4], "falling") == 0))
		state = 2;
	else if ( (strcasecmp(argv[4], "both") == 0))
		state = 3;
	else
	{
		if ( (atoi(argv[4]) > 3) || (atoi(argv[4]) < 0))
		{
			printf("Invalid gpio edge counting!\n");
			return ARG_ERR;
		}
		state = (u8)atoi(argv[4]);
	}

	if (OK != gpioEdgeSet(dev, pin, state))
	{
		printf("Fail to write gpio edge counting \n");
		return ERROR;
	}
	return OK;
}

int doGpioCntRead(int argc, char *argv[])
{
	int pin = 0;
	u32 val = 0;
	int dev = 0;

	dev = doBoardInit(atoi(argv[1]));
	if (dev <= 0)
	{
		return ERROR;
	}

	if (argc == 4)
	{
		pin = atoi(argv[3]);
		if ( (pin < CHANNEL_NR_MIN) || (pin > GPIO_CH_NR_MAX))
		{
			printf("Gpio pin number value out of range!\n");
			return ARG_ERR;
		}

		if (OK != gpioCountGet(dev, (u8)pin, &val))
		{
			printf("Fail to read!\n");
			return ERROR;
		}
		printf("%u\n", val);
	}
	else
	{
		return ARG_CNT_ERR;
	}
	return OK;
}


int doGpioEdgeRead(int argc, char *argv[])
{
	u8 pin = 0;
	u8 val = 0;
	int dev = 0;

	dev = doBoardInit(atoi(argv[1]));
	if (dev <= 0)
	{
		return ERROR;
	}

	if (argc == 4)
	{
		pin = (u8)atoi(argv[3]);
		if ( (pin < CHANNEL_NR_MIN) || (pin > GPIO_CH_NR_MAX))
		{
			printf("Gpio pin number value out of range!\n");
			return ARG_ERR;
		}

		if (OK != gpioEdgeGet(dev, pin, &val))
		{
			printf("Fail to read!\n");
			return ERROR;
		}
		printf("%d\n", val);
	}
	else
	{
		return ARG_CNT_ERR;
	}
	return OK;
}

int doGpioCntRst(int argc, char *argv[])
{
	int pin = 0;
	int dev = 0;

	dev = doBoardInit(atoi(argv[1]));
	if (dev <= 0)
	{
		return ERROR;
	}

	if (argc == 4)
	{
		pin = atoi(argv[3]);
		if ( (pin < CHANNEL_NR_MIN) || (pin > GPIO_CH_NR_MAX))
		{
			printf("Gpio pin number value out of range!\n");
			return ARG_ERR;
		}

		if (OK != gpioCountReset(dev, (u8)pin))
		{
			printf("Fail to reset!\n");
			return ERROR;
		}
	}
	else
	{
		return ARG_CNT_ERR;
	}
	return OK;
}
