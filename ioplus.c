/*
 * rtd.c:
 *	Command-line interface to the Raspberry
 *	Pi's MEGAS-RTD board.
 *	Copyright (c) 2016-2020 Sequent Microsystem
 *	<http://www.sequentmicrosystem.com>
 ***********************************************************************
 *	Author: Alexandru Burcea
 ***********************************************************************
 */
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <math.h>

#include "ioplus.h"
#include "comm.h"
#include "thread.h"

#define VERSION_BASE	(int)1
#define VERSION_MAJOR	(int)0
#define VERSION_MINOR	(int)0

#define UNUSED(X) (void)X      /* To avoid gcc/g++ warnings */
#define CMD_ARRAY_SIZE	24

CliCmdType gCmdArray[CMD_ARRAY_SIZE];

char *warranty =
	"	       Copyright (c) 2016-2020 Sequent Microsystems\n"
		"                                                             \n"
		"		This program is free software; you can redistribute it and/or modify\n"
		"		it under the terms of the GNU Leser General Public License as published\n"
		"		by the Free Software Foundation, either version 3 of the License, or\n"
		"		(at your option) any later version.\n"
		"                                    \n"
		"		This program is distributed in the hope that it will be useful,\n"
		"		but WITHOUT ANY WARRANTY; without even the implied warranty of\n"
		"		MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the\n"
		"		GNU Lesser General Public License for more details.\n"
		"			\n"
		"		You should have received a copy of the GNU Lesser General Public License\n"
		"		along with this program. If not, see <http://www.gnu.org/licenses/>.";

void usage(void)
{
	int i = 0;
	for (i = 0; i < CMD_ARRAY_SIZE; i++)
	{
		if (gCmdArray[i].name != NULL)
		{
			if (strlen(gCmdArray[i].usage1) > 2)
			{
				printf("%s", gCmdArray[i].usage1);
			}
			if (strlen(gCmdArray[i].usage2) > 2)
			{
				printf("%s", gCmdArray[i].usage2);
			}
		}
	}
	printf("Where: <id> = Board level id = 0..7\n");
	printf("Type ioplus -h <command> for more help\n");
}

int doBoardInit(int stack)
{
	int dev = 0;
	int add = 0;
	uint8_t buff[8];

	if ( (stack < 0) || (stack > 7))
	{
		printf("Invalid stack level [0..7]!");
		return ERROR;
	}
	add = stack + SLAVE_OWN_ADDRESS_BASE;
	dev = i2cSetup(add);
	if (dev == -1)
	{
		return ERROR;
	}
	if (ERROR == i2cMem8Read(dev, I2C_MEM_REVISION_MAJOR_ADD, buff, 1))
	{
		printf("IO-PLUS id %d not detected\n", stack);
		return ERROR;
	}
	return dev;
}

int boardCheck(int stack)
{
	int dev = 0;
	int add = 0;
	uint8_t buff[8];

	if ( (stack < 0) || (stack > 7))
	{
		printf("Invalid stack level [0..7]!");
		return ERROR;
	}
	add = stack + SLAVE_OWN_ADDRESS_BASE;
	dev = i2cSetup(add);
	if (dev == -1)
	{
		return ERROR;
	}
	if (ERROR == i2cMem8Read(dev, I2C_MEM_REVISION_MAJOR_ADD, buff, 1))
	{

		return ERROR;
	}
	return OK;
}
static void doHelp(int argc, char *argv[]);
const CliCmdType CMD_HELP =
{
	"-h",
	1,
	&doHelp,
	"\t-h		Display the list of command options or one command option details\n",
	"\tUsage:		ioplus -h    Display command options list\n",
	"\tUsage:		ioplus -h <param>   Display help for <param> command option\n",
	"\tExample:		ioplus -h rread    Display help for \"rread\" command option\n"};

static void doHelp(int argc, char *argv[])
{
	int i = 0;
	if (argc == 3)
	{
		for (i = 0; i < CMD_ARRAY_SIZE; i++)
		{
			if (gCmdArray[i].name != NULL)
			{
				if (strcasecmp(argv[2], gCmdArray[i].name) == 0)
				{
					printf("%s%s%s%s", gCmdArray[i].help, gCmdArray[i].usage1,
						gCmdArray[i].usage2, gCmdArray[i].example);
					break;
				}
			}
		}
		if (CMD_ARRAY_SIZE == i)
		{
			printf("Option \"%s\" not found\n", argv[2]);
			for (i = 0; i < CMD_ARRAY_SIZE; i++)
			{
				if (gCmdArray[i].name != NULL)
				{
					printf("%s", gCmdArray[i].help);
					break;
				}
			}
		}
	}
	else
	{
		for (i = 0; i < CMD_ARRAY_SIZE; i++)
		{
			if (gCmdArray[i].name != NULL)
			{
				printf("%s", gCmdArray[i].help);
			}
		}
	}
}

static void doVersion(int argc, char *argv[]);
const CliCmdType CMD_VERSION =
{
	"-v",
	1,
	&doVersion,
	"\t-v		Display the version number\n",
	"\tUsage:		ioplus -v\n",
	"",
	"\tExample:		ioplus -v  Display the version number\n"};

static void doVersion(int argc, char *argv[])
{
	UNUSED(argc);
	UNUSED(argv);
	printf("ioplus v%d.%d.%d Copyright (c) 2016 - 2020 Sequent Microsystems\n",
	VERSION_BASE, VERSION_MAJOR, VERSION_MINOR);
	printf("\nThis is free software with ABSOLUTELY NO WARRANTY.\n");
	printf("For details type: ioplus -warranty\n");

}

static void doWarranty(int argc, char* argv[]);
const CliCmdType CMD_WAR =
{
	"-warranty",
	1,
	&doWarranty,
	"\t-warranty	Display the warranty\n",
	"\tUsage:		ioplus -warranty\n",
	"",
	"\tExample:		ioplus -warranty  Display the warranty text\n"};

static void doWarranty(int argc UNU, char* argv[] UNU)
{
	printf("%s\n", warranty);
}

static void doList(int argc, char *argv[]);
const CliCmdType CMD_LIST =
	{
		"-list",
		1,
		&doList,
		"\t-list:		List all ioplus boards connected,return the # of boards and stack level for every board\n",
		"\tUsage:		ioplus -list\n",
		"",
		"\tExample:		ioplus -list display: 1,0 \n"};

static void doList(int argc, char *argv[])
{
	int ids[8];
	int i;
	int cnt = 0;

	UNUSED(argc);
	UNUSED(argv);

	for (i = 0; i < 8; i++)
	{
		if (boardCheck(i) == OK)
		{
			ids[cnt] = i;
			cnt++;
		}
	}
	printf("%d board(s) detected\n", cnt);
	if (cnt > 0)
	{
		printf("Id:");
	}
	while (cnt > 0)
	{
		cnt--;
		printf(" %d", ids[cnt]);
	}
	printf("\n");
}

int relayChSet(int dev, u8 channel, OutStateEnumType state)
{
	int resp;
	u8 buff[2];

	if ( (channel < CHANNEL_NR_MIN) || (channel > RELAY_CH_NR_MAX))
	{
		printf("Invalid relay nr!\n");
		return ERROR;
	}
	if (FAIL == i2cMem8Read(dev, I2C_MEM_RELAY_VAL_ADD, buff, 1))
	{
		return FAIL;
	}

	switch (state)
	{
	case OFF:
		buff[0] &= ~ (1 << (channel - 1));
		resp = i2cMem8Write(dev, I2C_MEM_RELAY_VAL_ADD, buff, 1);
		break;
	case ON:
		buff[0] |= 1 << (channel - 1);
		resp = i2cMem8Write(dev, I2C_MEM_RELAY_VAL_ADD, buff, 1);
		break;
	default:
		printf("Invalid relay state!\n");
		return ERROR;
		break;
	}
	return resp;
}

int relayChGet(int dev, u8 channel, OutStateEnumType* state)
{
	u8 buff[2];

	if (NULL == state)
	{
		return ERROR;
	}

	if ( (channel < CHANNEL_NR_MIN) || (channel > RELAY_CH_NR_MAX))
	{
		printf("Invalid relay nr!\n");
		return ERROR;
	}

	if (FAIL == i2cMem8Read(dev, I2C_MEM_RELAY_VAL_ADD, buff, 1))
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

int relaySet(int dev, int val)
{
	u8 buff[2];

	buff[0] = 0xff & val;

	return i2cMem8Write(dev, I2C_MEM_RELAY_VAL_ADD, buff, 1);
}

int relayGet(int dev, int* val)
{
	u8 buff[2];

	if (NULL == val)
	{
		return ERROR;
	}
	if (FAIL == i2cMem8Read(dev, I2C_MEM_RELAY_VAL_ADD, buff, 1))
	{
		return ERROR;
	}
	*val = buff[0];
	return OK;
}

static void doRelayWrite(int argc, char *argv[]);
const CliCmdType CMD_RELAY_WRITE =
{
	"relwr",
	2,
	&doRelayWrite,
	"\trelwr:		Set relays On/Off\n",
	"\tUsage:		ioplus <id> relwr <channel> <on/off>\n",
	"\tUsage:		ioplus <id> relwr <value>\n",
	"\tExample:		ioplus 0 relwr 2 1; Set Relay #2 on Board #0 On\n"};

static void doRelayWrite(int argc, char *argv[])
{
	int pin = 0;
	OutStateEnumType state = STATE_COUNT;
	int val = 0;
	int dev = 0;
	OutStateEnumType stateR = STATE_COUNT;
	int valR = 0;
	int retry = 0;

	if ( (argc != 5) && (argc != 4))
	{
		printf("%s", CMD_RELAY_WRITE.usage1);
		printf("%s", CMD_RELAY_WRITE.usage2);
		exit(1);
	}

	dev = doBoardInit(atoi(argv[1]));
	if (dev <= 0)
	{
		exit(1);
	}
	if (argc == 5)
	{
		pin = atoi(argv[3]);
		if ( (pin < CHANNEL_NR_MIN) || (pin > RELAY_CH_NR_MAX))
		{
			printf("Relay number value out of range\n");
			exit(1);
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
				printf("Invalid relay state!\n");
				exit(1);
			}
			state = (OutStateEnumType)atoi(argv[4]);
		}

		retry = RETRY_TIMES;

		while ( (retry > 0) && (stateR != state))
		{
			if (OK != relayChSet(dev, pin, state))
			{
				printf("Fail to write relay\n");
				exit(1);
			}
			if (OK != relayChGet(dev, pin, &stateR))
			{
				printf("Fail to read relay\n");
				exit(1);
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
			printf("Fail to write relay\n");
			exit(1);
		}
	}
	else
	{
		val = atoi(argv[3]);
		if (val < 0 || val > 255)
		{
			printf("Invalid relay value\n");
			exit(1);
		}

		retry = RETRY_TIMES;
		valR = -1;
		while ( (retry > 0) && (valR != val))
		{

			if (OK != relaySet(dev, val))
			{
				printf("Fail to write relay!\n");
				exit(1);
			}
			if (OK != relayGet(dev, &valR))
			{
				printf("Fail to read relay!\n");
				exit(1);
			}
		}
		if (retry == 0)
		{
			printf("Fail to write relay!\n");
			exit(1);
		}
	}
}

static void doRelayRead(int argc, char *argv[]);
const CliCmdType CMD_RELAY_READ =
{
	"relrd",
	2,
	&doRelayRead,
	"\trelrd:		Read relays status\n",
	"\tUsage:		ioplus <id> relrd <channel>\n",
	"\tUsage:		ioplus <id> relrd\n",
	"\tExample:		ioplus 0 relrd 2; Read Status of Relay #2 on Board #0\n"};

static void doRelayRead(int argc, char *argv[])
{
	int pin = 0;
	int val = 0;
	int dev = 0;
	OutStateEnumType state = STATE_COUNT;

	dev = doBoardInit(atoi(argv[1]));
	if (dev <= 0)
	{
		exit(1);
	}

	if (argc == 4)
	{
		pin = atoi(argv[3]);
		if ( (pin < CHANNEL_NR_MIN) || (pin > RELAY_CH_NR_MAX))
		{
			printf("Relay number value out of range!\n");
			exit(1);
		}

		if (OK != relayChGet(dev, pin, &state))
		{
			printf("Fail to read!\n");
			exit(1);
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
		if (OK != relayGet(dev, &val))
		{
			printf("Fail to read!\n");
			exit(1);
		}
		printf("%d\n", val);
	}
	else
	{
		printf("%s", CMD_RELAY_READ.usage1);
		printf("%s", CMD_RELAY_READ.usage2);
		exit(1);
	}
}

static void doRelayTest(int argc, char* argv[]);
const CliCmdType CMD_TEST =
{
	"reltest",
	2,
	&doRelayTest,
	"\treltest:	Turn ON and OFF the relays until press a key\n",
	"\tUsage:		ioplus <id> reltest\n",
	"",
	"\tExample:		ioplus 0 reltest\n"};

static void doRelayTest(int argc, char* argv[])
{
	int dev = 0;
	int i = 0;
	int retry = 0;
	int relVal;
	int valR;
	int relayResult = 0;
	FILE* file = NULL;
	const u8 relayOrder[8] =
	{
		1,
		2,
		3,
		4,
		5,
		6,
		7,
		8};

	dev = doBoardInit(atoi(argv[1]));
	if (dev <= 0)
	{
		exit(1);
	}
	if (argc == 4)
	{
		file = fopen(argv[3], "w");
		if (!file)
		{
			printf("Fail to open result file\n");
			//return -1;
		}
	}
//relay test****************************
	if (strcasecmp(argv[2], "reltest") == 0)
	{
		relVal = 0;
		printf(
			"Are all relays and LEDs turning on and off in sequence?\nPress y for Yes or any key for No....");
		startThread();
		while (relayResult == 0)
		{
			for (i = 0; i < 8; i++)
			{
				relayResult = checkThreadResult();
				if (relayResult != 0)
				{
					break;
				}
				valR = 0;
				relVal = (u8)1 << (relayOrder[i] - 1);

				retry = RETRY_TIMES;
				while ( (retry > 0) && ( (valR & relVal) == 0))
				{
					if (OK != relayChSet(dev, relayOrder[i], ON))
					{
						retry = 0;
						break;
					}

					if (OK != relayGet(dev, &valR))
					{
						retry = 0;
					}
				}
				if (retry == 0)
				{
					printf("Fail to write relay\n");
					if (file)
						fclose(file);
					exit(1);
				}
				busyWait(150);
			}

			for (i = 0; i < 8; i++)
			{
				relayResult = checkThreadResult();
				if (relayResult != 0)
				{
					break;
				}
				valR = 0xff;
				relVal = (u8)1 << (relayOrder[i] - 1);
				retry = RETRY_TIMES;
				while ( (retry > 0) && ( (valR & relVal) != 0))
				{
					if (OK != relayChSet(dev, relayOrder[i], OFF))
					{
						retry = 0;
					}
					if (OK != relayGet(dev, &valR))
					{
						retry = 0;
					}
				}
				if (retry == 0)
				{
					printf("Fail to write relay!\n");
					if (file)
						fclose(file);
					exit(1);
				}
				busyWait(150);
			}
		}
	}
	else
	{
		usage();
		exit(1);
	}
	if (relayResult == YES)
	{
		if (file)
		{
			fprintf(file, "Relay Test ............................ PASS\n");
		}
		else
		{
			printf("Relay Test ............................ PASS\n");
		}
	}
	else
	{
		if (file)
		{
			fprintf(file, "Relay Test ............................ FAIL!\n");
		}
		else
		{
			printf("Relay Test ............................ FAIL!\n");
		}
	}
	if (file)
	{
		fclose(file);
	}
	relaySet(dev, 0);
}

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

int gpioChGet(int dev, u8 channel, OutStateEnumType* state)
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

int gpioGet(int dev, int* val)
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

int gpioDirGet(int dev, int* val)
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

static void doGpioWrite(int argc, char *argv[]);
const CliCmdType CMD_GPIO_WRITE =
{
	"gpiowr",
	2,
	&doGpioWrite,
	"\tgpiowr:		Set gpio pins On/Off\n",
	"\tUsage:		ioplus <id> gpiowr <channel> <on/off>\n",
	"\tUsage:		ioplus <id> gpiowr <value>\n",
	"\tExample:		ioplus 0 gpiowr 2 1; Set GPIO pin #2 on Board #0 to 1 logic\n"};

static void doGpioWrite(int argc, char *argv[])
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
		printf("%s", CMD_GPIO_WRITE.usage1);
		printf("%s", CMD_GPIO_WRITE.usage2);
		exit(1);
	}

	dev = doBoardInit(atoi(argv[1]));
	if (dev <= 0)
	{
		exit(1);
	}
	if (argc == 5)
	{
		pin = atoi(argv[3]);
		if ( (pin < CHANNEL_NR_MIN) || (pin > GPIO_CH_NR_MAX))
		{
			printf("Gpio pin number value out of range\n");
			exit(1);
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
				printf("Invalid relay state!\n");
				exit(1);
			}
			state = (OutStateEnumType)atoi(argv[4]);
		}

		if (OK != gpioDirGet(dev, &direction))
		{
			printf("Fail to read gpio direction \n");
			exit(1);
		}

		if ( (1 << (pin - 1)) & direction) //pin is input
		{
			printf("Fail to write gpio pin, is input\n");
			exit(1);
		}
		retry = RETRY_TIMES;

		while ( (retry > 0) && (stateR != state))
		{
			if (OK != gpioChSet(dev, pin, state))
			{
				printf("Fail to write gpio\n");
				exit(1);
			}
			if (OK != gpioChGet(dev, pin, &stateR))
			{
				printf("Fail to read gpio\n");
				exit(1);
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
			exit(1);
		}
	}
	else
	{
		val = atoi(argv[3]);
		if (val < 0 || val > 0x0f)
		{
			printf("Invalid gpio value\n");
			exit(1);
		}

		if (OK != gpioSet(dev, val))
		{
			printf("Fail to write GPIO\n");
			exit(1);
		}
	}
}

static void doGpioRead(int argc, char *argv[]);
const CliCmdType CMD_GPIO_READ =
{
	"gpiord",
	2,
	&doGpioRead,
	"\tgpiord:		Read gpio status\n",
	"\tUsage:		ioplus <id> gpiord <channel>\n",
	"\tUsage:		ioplus <id> gpiord\n",
	"\tExample:		ioplus 0 gpiord 2; Read Status of Gpio pin #2 on Board #0\n"};

static void doGpioRead(int argc, char *argv[])
{
	int pin = 0;
	int val = 0;
	int dev = 0;
	OutStateEnumType state = STATE_COUNT;

	dev = doBoardInit(atoi(argv[1]));
	if (dev <= 0)
	{
		exit(1);
	}

	if (argc == 4)
	{
		pin = atoi(argv[3]);
		if ( (pin < CHANNEL_NR_MIN) || (pin > GPIO_CH_NR_MAX))
		{
			printf("Gpio pin number value out of range!\n");
			exit(1);
		}

		if (OK != gpioChGet(dev, pin, &state))
		{
			printf("Fail to read!\n");
			exit(1);
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
			exit(1);
		}
		printf("%d\n", val);
	}
	else
	{
		printf("%s", CMD_GPIO_READ.usage1);
		printf("%s", CMD_GPIO_READ.usage2);
		exit(1);
	}
}

static void doGpioDirWrite(int argc, char *argv[]);
const CliCmdType CMD_GPIO_DIR_WRITE =
{
	"gpiodirwr",
	2,
	&doGpioDirWrite,
	"\tgpiodirwr:	Set gpio pins direction I/O  0- output; 1-input\n",
	"\tUsage:		ioplus <id> gpiodirwr <channel> <out/in> \n",
	"\tUsage:		ioplus <id> gpiodirwr <value>\n",
	"\tExample:	ioplus 0 gpiodirwr 2 1; Set GPIO pin #2 on Board #0 as input\n"};

static void doGpioDirWrite(int argc, char *argv[])
{
	int pin = 0;
	OutStateEnumType state = STATE_COUNT;
	int val = 0;
	int dev = 0;

	if ( (argc != 5) && (argc != 4))
	{
		printf("%s", CMD_GPIO_WRITE.usage1);
		printf("%s", CMD_GPIO_WRITE.usage2);
		exit(1);
	}

	dev = doBoardInit(atoi(argv[1]));
	if (dev <= 0)
	{
		exit(1);
	}
	if (argc == 5)
	{
		pin = atoi(argv[3]);
		if ( (pin < CHANNEL_NR_MIN) || (pin > GPIO_CH_NR_MAX))
		{
			printf("Gpio pin number value out of range\n");
			exit(1);
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
				exit(1);
			}
			state = (OutStateEnumType)atoi(argv[4]);
		}

		if (OK != gpioChDirSet(dev, pin, state))
		{
			printf("Fail to write gpio direction \n");
			exit(1);
		}

	}
	else if (argc == 4)
	{
		val = atoi(argv[3]);
		if (val < 0 || val > 0x0f)
		{
			printf("Invalid gpio direction value\n");
			exit(1);
		}

		if (OK != gpioDirSet(dev, val))
		{
			printf("Fail to write gpio direction \n");
			exit(1);
		}
	}
	else
	{
		printf("%s", CMD_GPIO_DIR_WRITE.usage1);
		printf("%s", CMD_GPIO_DIR_WRITE.usage2);
		exit(1);
	}
}

static void doGpioDirRead(int argc, char *argv[]);
const CliCmdType CMD_GPIO_DIR_READ =
	{
		"gpiodirrd",
		2,
		&doGpioDirRead,
		"\tgpiodirrd:	Read gpio direction 0 - output; 1 - input\n",
		"\tUsage:		ioplus <id> gpiodirrd <pin>\n",
		"\tUsage:		ioplus <id> gpiodirrd\n",
		"\tExample:		ioplus 0 gpiodirrd 2; Read direction of Gpio pin #2 on Board #0\n"};

static void doGpioDirRead(int argc, char *argv[])
{
	int pin = 0;
	int val = 0;
	int dev = 0;

	dev = doBoardInit(atoi(argv[1]));
	if (dev <= 0)
	{
		exit(1);
	}

	if (argc == 4)
	{
		pin = atoi(argv[3]);
		if ( (pin < CHANNEL_NR_MIN) || (pin > GPIO_CH_NR_MAX))
		{
			printf("Gpio pin number value out of range!\n");
			exit(1);
		}

		if (OK != gpioDirGet(dev, &val))
		{
			printf("Fail to read!\n");
			exit(1);
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
			exit(1);
		}
		printf("%d\n", val);
	}
	else
	{
		printf("%s", CMD_GPIO_DIR_READ.usage1);
		printf("%s", CMD_GPIO_DIR_READ.usage2);
		exit(1);
	}
}
int optoChGet(int dev, u8 channel, OutStateEnumType* state)
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

	if (FAIL == i2cMem8Read(dev, I2C_MEM_OPTO_IN_ADD, buff, 1))
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

int optoGet(int dev, int* val)
{
	u8 buff[2];

	if (NULL == val)
	{
		return ERROR;
	}
	if (FAIL == i2cMem8Read(dev, I2C_MEM_OPTO_IN_ADD, buff, 1))
	{
		return ERROR;
	}
	*val = buff[0];
	return OK;
}

static void doOptoRead(int argc, char *argv[]);
const CliCmdType CMD_OPTO_READ =
	{
		"optrd",
		2,
		&doOptoRead,
		"\toptrd:		Read optocoupled inputs status\n",
		"\tUsage:		ioplus <id> optrd <channel>\n",
		"\tUsage:		ioplus <id> optrd\n",
		"\tExample:		ioplus 0 optrd 2; Read Status of Optocoupled input ch #2 on Board #0\n"};

static void doOptoRead(int argc, char *argv[])
{
	int pin = 0;
	int val = 0;
	int dev = 0;
	OutStateEnumType state = STATE_COUNT;

	dev = doBoardInit(atoi(argv[1]));
	if (dev <= 0)
	{
		exit(1);
	}

	if (argc == 4)
	{
		pin = atoi(argv[3]);
		if ( (pin < CHANNEL_NR_MIN) || (pin > OPTO_IN_CH_NR_MAX))
		{
			printf("Opto input channel number value out of range!\n");
			exit(1);
		}

		if (OK != optoChGet(dev, pin, &state))
		{
			printf("Fail to read!\n");
			exit(1);
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
			exit(1);
		}
		printf("%d\n", val);
	}
	else
	{
		printf("%s", CMD_OPTO_READ.usage1);
		printf("%s", CMD_OPTO_READ.usage2);
		exit(1);
	}
}

int odGet(int dev, int ch, float* val)
{
	u8 buff[2] =
	{
		0,
		0};
	u16 raw = 0;

	if ( (ch < CHANNEL_NR_MIN) || (ch > OD_CH_NR_MAX))
	{
		printf("Open drain channel out of range!\n");
		return ERROR;
	}
	if (OK
		!= i2cMem8Read(dev, I2C_MEM_OD_PWM_VAL_RAW_ADD + 2 * (ch - 1), buff, 2))
	{
		printf("Fail to read!\n");
		return ERROR;
	}
	memcpy(&raw, buff, 2);
	*val = 100 * (float)raw / OD_PWM_VAL_MAX;
	return OK;
}

int odSet(int dev, int ch, float val)
{
	u8 buff[2] =
	{
		0,
		0};
	u16 raw = 0;

	if ( (ch < CHANNEL_NR_MIN) || (ch > OD_CH_NR_MAX))
	{
		printf("Open drain channel out of range!\n");
		return ERROR;
	}
	if (val < 0)
	{
		val = 0;
	}
	if (val > 100)
	{
		val = 100;
	}
	raw = (u16)ceil(OD_PWM_VAL_MAX * val / 100);
	memcpy(buff, &raw, 2);
	if (OK
		!= i2cMem8Write(dev, I2C_MEM_OD_PWM_VAL_RAW_ADD + 2 * (ch - 1), buff, 2))
	{
		printf("Fail to write!\n");
		return ERROR;
	}
	return OK;
}

static void doOdRead(int argc, char *argv[]);
const CliCmdType CMD_OD_READ =
	{
		"odrd",
		2,
		&doOdRead,
		"\todrd:		Read open drain output pwm value (0% - 100%)\n",
		"\tUsage:		ioplus <id> odrd <channel>\n",
		"",
		"\tExample:		ioplus 0 odrd 2; Read pwm value of open drain channel #2 on Board #0\n"};

static void doOdRead(int argc, char *argv[])
{
	int ch = 0;
	float val = 0;
	int dev = 0;

	dev = doBoardInit(atoi(argv[1]));
	if (dev <= 0)
	{
		exit(1);
	}

	if (argc == 4)
	{
		ch = atoi(argv[3]);
		if ( (ch < CHANNEL_NR_MIN) || (ch > OD_CH_NR_MAX))
		{
			printf("Open drain channel out of range!\n");
			exit(1);
		}

		if (OK != odGet(dev, ch, &val))
		{
			printf("Fail to read!\n");
			exit(1);
		}

		printf("%0.2f\n", val);
	}
	else
	{
		printf("Invalid params number:\n %s", CMD_OD_READ.usage1);
		exit(1);
	}
}

static void doOdWrite(int argc, char *argv[]);
const CliCmdType CMD_OD_WRITE =
	{
		"odwr",
		2,
		&doOdWrite,
		"\todwr:		Write open drain output pwm value (0% - 100%), Warning: This function change the output of the coresponded DAC channel\n",
		"\tUsage:		ioplus <id> odwr <channel> <value>\n",
		"",
		"\tExample:		ioplus 0 odwr 2 12.5; Write pwm 12.5% to open drain channel #2 on Board #0\n"};

static void doOdWrite(int argc, char *argv[])
{
	int ch = 0;
	int dev = 0;
	float proc = 0;

	dev = doBoardInit(atoi(argv[1]));
	if (dev <= 0)
	{
		exit(1);
	}

	if (argc == 5)
	{
		ch = atoi(argv[3]);
		if ( (ch < CHANNEL_NR_MIN) || (ch > OD_CH_NR_MAX))
		{
			printf("Open drain channel out of range!\n");
			exit(1);
		}
		proc = atof(argv[4]);
		if (proc < 0 || proc > 100)
		{
			printf("Invalid open drain pwm value, must be 0..100 \n");
			exit(1);
		}

		if (OK != odSet(dev, ch, proc))
		{
			printf("Fail to write!\n");
			exit(1);
		}
		printf("done\n");
	}
	else
	{
		printf("Invalid params number:\n %s", CMD_OD_WRITE.usage1);
		exit(1);
	}
}

int dacGet(int dev, int ch, float* val)
{
	u8 buff[2] =
	{
		0,
		0};
	u16 raw = 0;

	if ( (ch < CHANNEL_NR_MIN) || (ch > DAC_CH_NR_MAX))
	{
		printf("DAC channel out of range!\n");
		return ERROR;
	}
	if (OK != i2cMem8Read(dev, I2C_MEM_DAC_VAL_MV_ADD + 2 * (ch - 1), buff, 2))
	{
		printf("Fail to read!\n");
		return ERROR;
	}
	memcpy(&raw, buff, 2);
	*val = (float)raw / 1000;
	return OK;
}

int dacSet(int dev, int ch, float val)
{
	u8 buff[2] =
	{
		0,
		0};
	u16 raw = 0;

	if ( (ch < CHANNEL_NR_MIN) || (ch > DAC_CH_NR_MAX))
	{
		printf("DAC channel out of range!\n");
		return ERROR;
	}
	if (val < 0)
	{
		val = 0;
	}
	if (val > 100)
	{
		val = 100;
	}
	raw = (u16)ceil(val * 1000); //transform to milivolts
	memcpy(buff, &raw, 2);
	if (OK != i2cMem8Write(dev, I2C_MEM_DAC_VAL_MV_ADD + 2 * (ch - 1), buff, 2))
	{
		printf("Fail to write!\n");
		return ERROR;
	}
	return OK;
}

static void doDacRead(int argc, char *argv[]);
const CliCmdType CMD_DAC_READ =
	{
		"dacrd",
		2,
		&doDacRead,
		"\tdacrd:		Read DAC voltage value (0 - 10V)\n",
		"\tUsage:		ioplus <id> dacrd <channel>\n",
		"",
		"\tExample:		ioplus 0 dacrd 2; Read the voltage on DAC channel #2 on Board #0\n"};

static void doDacRead(int argc, char *argv[])
{
	int ch = 0;
	float val = 0;
	int dev = 0;

	dev = doBoardInit(atoi(argv[1]));
	if (dev <= 0)
	{
		exit(1);
	}

	if (argc == 4)
	{
		ch = atoi(argv[3]);
		if ( (ch < CHANNEL_NR_MIN) || (ch > DAC_CH_NR_MAX))
		{
			printf("DAC channel out of range!\n");
			exit(1);
		}

		if (OK != dacGet(dev, ch, &val))
		{
			printf("Fail to read!\n");
			exit(1);
		}

		printf("%0.3f\n", val);
	}
	else
	{
		printf("Invalid params number:\n %s", CMD_DAC_READ.usage1);
		exit(1);
	}
}

static void doDacWrite(int argc, char *argv[]);
const CliCmdType CMD_DAC_WRITE =
	{
		"dacwr",
		2,
		&doDacWrite,
		"\tdacwr:		Write DAC output voltage value (0..10V), Warning: This function change the output of the coresponded open-drain channel\n",
		"\tUsage:		ioplus <id> dacwr <channel> <value>\n",
		"",
		"\tExample:		ioplus 0 dacwr 2 2.5; Write 2.5V to DAC channel #2 on Board #0\n"};

static void doDacWrite(int argc, char *argv[])
{
	int ch = 0;
	int dev = 0;
	float volt = 0;

	dev = doBoardInit(atoi(argv[1]));
	if (dev <= 0)
	{
		exit(1);
	}

	if (argc == 5)
	{
		ch = atoi(argv[3]);
		if ( (ch < CHANNEL_NR_MIN) || (ch > DAC_CH_NR_MAX))
		{
			printf("DAC channel out of range!\n");
			exit(1);
		}
		volt = atof(argv[4]);
		if (volt < 0 || volt > 10)
		{
			printf("Invalid DAC voltage value, must be 0..10 \n");
			exit(1);
		}

		if (OK != dacSet(dev, ch, volt))
		{
			printf("Fail to write!\n");
			exit(1);
		}
		printf("done\n");
	}
	else
	{
		printf("Invalid params number:\n %s", CMD_DAC_WRITE.usage1);
		exit(1);
	}
}

int adcGet(int dev, int ch, float* val)
{
	u8 buff[2] =
	{
		0,
		0};
	u16 raw = 0;

	if ( (ch < CHANNEL_NR_MIN) || (ch > ADC_CH_NR_MAX))
	{
		printf("ADC channel out of range!\n");
		return ERROR;
	}
	if (OK != i2cMem8Read(dev, I2C_MEM_ADC_VAL_MV_ADD + 2 * (ch - 1), buff, 2))
	{
		printf("Fail to read!\n");
		return ERROR;
	}
	memcpy(&raw, buff, 2);
	*val = (float)raw / 1000;
	return OK;
}

static void doAdcRead(int argc, char *argv[]);
const CliCmdType CMD_ADC_READ =
	{
		"adcrd",
		2,
		&doAdcRead,
		"\tadcrd:		Read ADC input voltage value (0 - 3.3V)\n",
		"\tUsage:		ioplus <id> adcrd <channel>\n",
		"",
		"\tExample:		ioplus 0 adcrd 2; Read the voltage input on ADC channel #2 on Board #0\n"};

static void doAdcRead(int argc, char *argv[])
{
	int ch = 0;
	float val = 0;
	int dev = 0;

	dev = doBoardInit(atoi(argv[1]));
	if (dev <= 0)
	{
		exit(1);
	}

	if (argc == 4)
	{
		ch = atoi(argv[3]);
		if ( (ch < CHANNEL_NR_MIN) || (ch > ADC_CH_NR_MAX))
		{
			printf("ADC channel out of range!\n");
			exit(1);
		}

		if (OK != adcGet(dev, ch, &val))
		{
			printf("Fail to read!\n");
			exit(1);
		}

		printf("%0.3f\n", val);
	}
	else
	{
		printf("Invalid params number:\n %s", CMD_ADC_READ.usage1);
		exit(1);
	}
}

static void cliInit(void)
{
	int i = 0;

	memset(gCmdArray, 0, sizeof(CliCmdType) * CMD_ARRAY_SIZE);
	memcpy(&gCmdArray[i], &CMD_VERSION, sizeof(CliCmdType));
	i++;
	memcpy(&gCmdArray[i], &CMD_HELP, sizeof(CliCmdType));
	i++;
	memcpy(&gCmdArray[i], &CMD_WAR, sizeof(CliCmdType));
	i++;
	memcpy(&gCmdArray[i], &CMD_LIST, sizeof(CliCmdType));
	i++;
	memcpy(&gCmdArray[i], &CMD_RELAY_WRITE, sizeof(CliCmdType));
	i++;
	memcpy(&gCmdArray[i], &CMD_RELAY_READ, sizeof(CliCmdType));
	i++;
	memcpy(&gCmdArray[i], &CMD_TEST, sizeof(CliCmdType));
	i++;
	memcpy(&gCmdArray[i], &CMD_GPIO_WRITE, sizeof(CliCmdType));
	i++;
	memcpy(&gCmdArray[i], &CMD_GPIO_READ, sizeof(CliCmdType));
	i++;
	memcpy(&gCmdArray[i], &CMD_GPIO_DIR_WRITE, sizeof(CliCmdType));
	i++;
	memcpy(&gCmdArray[i], &CMD_GPIO_DIR_READ, sizeof(CliCmdType));
	i++;
	memcpy(&gCmdArray[i], &CMD_OPTO_READ, sizeof(CliCmdType));
	i++;
	memcpy(&gCmdArray[i], &CMD_OD_READ, sizeof(CliCmdType));
	i++;
	memcpy(&gCmdArray[i], &CMD_OD_WRITE, sizeof(CliCmdType));
	i++;
	memcpy(&gCmdArray[i], &CMD_DAC_READ, sizeof(CliCmdType));
	i++;
	memcpy(&gCmdArray[i], &CMD_DAC_WRITE, sizeof(CliCmdType));
	i++;
	memcpy(&gCmdArray[i], &CMD_ADC_READ, sizeof(CliCmdType));
}

int main(int argc, char *argv[])
{
	int i = 0;

	cliInit();

	if (argc == 1)
	{
		usage();
		return 1;
	}
	for (i = 0; i < CMD_ARRAY_SIZE; i++)
	{
		if ( (gCmdArray[i].name != NULL) && (gCmdArray[i].namePos < argc))
		{
			if (strcasecmp(argv[gCmdArray[i].namePos], gCmdArray[i].name) == 0)
			{
				gCmdArray[i].pFunc(argc, argv);
				return 0;
			}
		}
	}
	printf("Invalid command option\n");
	usage();

	return 0;
}
