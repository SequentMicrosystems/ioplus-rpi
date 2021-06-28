#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <math.h>

#include "comm.h"
#include "ioplus.h"

int optoChGet(int dev, u8 channel, OutStateEnumType *state)
{
	u8 buff[2];

	if (NULL == state)
	{
		return ERROR;
	}

	if ( (channel < CHANNEL_NR_MIN) || (channel > OPTO_IN_CH_NR_MAX))
	{
		printf("Invalid opto channel nr!\n");
		return ERROR;
	}

	if (FAIL == i2cReadByteAS(dev, I2C_MEM_OPTO_IN_ADD, buff))
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

int optoGet(int dev, int *val)
{
	u8 buff[2];

	if (NULL == val)
	{
		return ERROR;
	}

	if (FAIL == i2cReadByteAS(dev, I2C_MEM_OPTO_IN_ADD, buff))
	{
		return ERROR;
	}
	*val = buff[0];
	return OK;
}

int optoEdgeGet(int dev, u8 channel, u8 *val)
{
	u8 buff[2];
	u8 rising = 0;
	u8 falling = 0;

	if (NULL == val)
	{
		return ERROR;
	}

	if ( (channel < CHANNEL_NR_MIN) || (channel > OPTO_IN_CH_NR_MAX))
	{
		printf("Invalid opto channel nr!\n");
		return ERROR;
	}
	if (FAIL == i2cMem8Read(dev, I2C_MEM_OPTO_IT_RISING_ADD, buff, 2))
	{
		return ERROR;
	}
	rising = buff[0];
	falling = buff[1];
	*val = ( ( (1 << (channel - 1)) & rising) == 0 ? 0 : 1);
	*val += ( ( (1 << (channel - 1)) & falling) == 0 ? 0 : 2);
	return OK;
}

int optoEdgeSet(int dev, u8 channel, u8 val)
{
	u8 buff[2];
	u8 rising = 0;
	u8 falling = 0;

	if ( (channel < CHANNEL_NR_MIN) || (channel > OPTO_IN_CH_NR_MAX))
	{
		return ERROR;
	}
	if (FAIL == i2cMem8Read(dev, I2C_MEM_OPTO_IT_RISING_ADD, buff, 2))
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
	if (FAIL == i2cMem8Write(dev, I2C_MEM_OPTO_IT_RISING_ADD, buff, 2))
	{
		return ERROR;
	}
	return OK;
}

int optoCountGet(int dev, u8 channel, u32 *val)
{
	if ( (channel < CHANNEL_NR_MIN) || (channel > OPTO_IN_CH_NR_MAX)
		|| (NULL == val))
	{
		return ERROR;
	}

	if (OK
		!= i2cReadDWordAS(dev,
			I2C_MEM_OPTO_EDGE_COUNT_ADD + COUNTER_SIZE * (channel - 1), (uint32_t *)val))
	{
		return ERROR;
	}
	return OK;
}

int optoCountReset(int dev, u8 channel)
{

	if ( (channel < CHANNEL_NR_MIN) || (channel > OPTO_IN_CH_NR_MAX))
	{
		return ERROR;
	}
	if (FAIL == i2cMem8Write(dev, I2C_MEM_OPTO_CNT_RST_ADD, &channel, 1))
	{
		return ERROR;
	}
	return OK;
}

int optoEncStateWrite(int dev, u8 channel, u8 val)
{
	u8 aux = 0;

	if ( (channel < CHANNEL_NR_MIN) || (channel > OPTO_IN_CH_NR_MAX / 2))
	{
		return ERROR;
	}
	if (FAIL == i2cMem8Read(dev, I2C_MEM_OPTO_ENC_ENABLE_ADD, &aux, 1))
	{
		return ERROR;
	}

	if (val != 0)
	{
		aux |= (1 << (channel - 1));
	}
	else
	{
		aux &= ~ (1 << (channel - 1));
	}

	if (FAIL == i2cMem8Write(dev, I2C_MEM_OPTO_ENC_ENABLE_ADD, &aux, 1))
	{
		return ERROR;
	}
	return OK;
}

int optoEncStateRead(int dev, u8 channel, u8 *val)
{
	u8 aux = 0;
	if ( (channel < CHANNEL_NR_MIN) || (channel > OPTO_IN_CH_NR_MAX / 2)
		|| (NULL == val))
	{
		return ERROR;
	}
	if (OK != i2cReadByteAS(dev, I2C_MEM_OPTO_ENC_ENABLE_ADD, &aux))
	{
		return ERROR;
	}
	*val = 0;
	if ( (1 << (channel - 1)) & aux)
	{
		*val = 1;
	}
	return OK;
}

int optoEncGetCnt(int dev, u8 channel, int *val)
{
	if ( (channel < CHANNEL_NR_MIN) || (channel > OPTO_IN_CH_NR_MAX / 2)
		|| (NULL == val))
	{
		return ERROR;
	}

	if (OK
		!= i2cReadIntAS(dev,
			I2C_MEM_OPTO_ENC_COUNT_ADD + COUNTER_SIZE * (channel - 1), val))
	{
		return ERROR;
	}
	return OK;
}

int optoEncRstCnt(int dev, u8 channel)
{
	if ( (channel < CHANNEL_NR_MIN) || (channel > OPTO_IN_CH_NR_MAX / 2))
	{
		return ERROR;
	}
	if (FAIL == i2cMem8Write(dev, I2C_MEM_OPTO_ENC_CNT_RST_ADD, &channel, 1))
	{
		return ERROR;
	}
	return OK;
}

int doOptoRead(int argc, char *argv[])
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
		if ( (pin < CHANNEL_NR_MIN) || (pin > OPTO_IN_CH_NR_MAX))
		{
			printf("Opto input channel number value out of range!\n");
			return ARG_ERR;
		}

		if (OK != optoChGet(dev, pin, &state))
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
		if (OK != optoGet(dev, &val))
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

int doOptoEdgeWrite(int argc, char *argv[])
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
	if ( (pin < CHANNEL_NR_MIN) || (pin > OPTO_IN_CH_NR_MAX))
	{
		printf("Optocoupled channel number value out of range\n");
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
			printf("Invalid edge counting type [0..3]!\n");
			return ARG_ERR;
		}
		state = (u8)atoi(argv[4]);
	}

	if (OK != optoEdgeSet(dev, pin, state))
	{
		printf("Fail to write optocoupled channel edge counting \n");
		return ERROR;
	}
	return OK;
}

int doOptoEdgeRead(int argc, char *argv[])
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
		if ( (pin < CHANNEL_NR_MIN) || (pin > OPTO_IN_CH_NR_MAX))
		{
			printf("Optocoupled channel number value out of range!\n");
			return ARG_ERR;
		}

		if (OK != optoEdgeGet(dev, pin, &val))
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

int doOptoCntRead(int argc, char *argv[])
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
		if ( (pin < CHANNEL_NR_MIN) || (pin > OPTO_IN_CH_NR_MAX))
		{
			printf("Optocoupled channel number value out of range!\n");
			return ARG_ERR;
		}

		if (OK != optoCountGet(dev, (u8)pin, &val))
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

int doOptoCntReset(int argc, char *argv[])
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
		if ( (pin < CHANNEL_NR_MIN) || (pin > OPTO_IN_CH_NR_MAX))
		{
			printf("Optocoupled channel number value out of range!\n");
			return ARG_ERR;
		}

		if (OK != optoCountReset(dev, (u8)pin))
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

int doOptoEncoderWrite(int argc, char *argv[])
{
	int pin = 0;
	int dev = 0;
	u8 state = 0;

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
	if ( (pin < CHANNEL_NR_MIN) || (pin > OPTO_IN_CH_NR_MAX / 2))
	{
		printf("Optocoupled encoder number value out of range [1..4]\n");
		return ARG_ERR;
	}
	state = (u8)atoi(argv[4]);
	if (OK != optoEncStateWrite(dev, pin, state))
	{
		printf("Fail to write encoder State\n");
		return ERROR;
	}

	return OK;
}

int doOptoEncoderRead(int argc, char *argv[])
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
		if ( (pin < CHANNEL_NR_MIN) || (pin > OPTO_IN_CH_NR_MAX / 2))
		{
			printf("Optocoupled encoder number value out of range!\n");
			return ARG_ERR;
		}

		if (OK != optoEncStateRead(dev, pin, &val))
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

int doOptoEncoderCntRead(int argc, char *argv[])
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
		if ( (pin < CHANNEL_NR_MIN) || (pin > OPTO_IN_CH_NR_MAX / 2))
		{
			printf("Optocoupled encoder number value out of range!\n");
			return ARG_ERR;
		}

		if (OK != optoEncGetCnt(dev, (u8)pin, &val))
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

int doOptoEncoderCntReset(int argc, char *argv[])
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
		if ( (pin < CHANNEL_NR_MIN) || (pin > OPTO_IN_CH_NR_MAX / 2))
		{
			printf("Optocoupled encoder number value out of range!\n");
			return ARG_ERR;
		}

		if (OK != optoEncRstCnt(dev, (u8)pin))
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
