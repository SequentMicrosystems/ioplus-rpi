/*
 * ioplus.c:
 *	Command-line interface to the Raspberry
 *	Pi's IOPLUS card.
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
#include "cli.h"

#define VERSION_BASE	(int)1
#define VERSION_MAJOR	(int)2
#define VERSION_MINOR	(int)3

#define UNUSED(X) (void)X      /* To avoid gcc/g++ warnings */

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

char *cn = " SIGNAL  CONNECTOR    SIGNAL\n"
	"           |---|\n"
	" 3.3V  -- 1|O O| 2--  +5V\n"
	" OPTO1 -- 3|O O| 4--  OPTO VEXT\n"
	" OPTO2 -- 5|O O| 6--  GND\n"
	" OPTO4 -- 7|O O| 8--  OPTO3\n"
	" GND   -- 9|O O|10--  ADC7\n"
	" ADC6  --11|O O|12--  ADC8\n"
	" ADC5  --13|O O|14--  GND\n"
	" ADC3  --15|O O|16--  ADC4\n"
	" 3.3V  --17|O O|18--  ADC2\n"
	" ADC1  --19|O O|20--  GND\n"
	" GPIO3 --21|O O|22--  IO4\n"
	" GPIO1 --23|O O|24--  IO2\n"
	" GND   --25|O O|26--  OC3\n"
	" DAC3  --27|O O|28--  OC4\n"
	" OPTO5 --29|O O|30--  GND\n"
	" OPTO6 --31|O O|32--  OC1\n"
	" DAC2  --33|O O|34--  GND\n"
	" OPTO7 --35|O O|36--  OC2\n"
	" DAC4  --37|O O|38--  OPTO8\n"
	" 12VEXT--39|O O|40--  DAC1\n"
	"           |---|\n";

void usage(void)
{
	int i = 0;
	while (gCmdArray[i] != NULL)
	{
		if (gCmdArray[i]->name != NULL)
		{
			if (strlen(gCmdArray[i]->usage1) > 2)
			{
				printf("%s", gCmdArray[i]->usage1);
			}
			if (strlen(gCmdArray[i]->usage2) > 2)
			{
				printf("%s", gCmdArray[i]->usage2);
			}
		}
		i++;
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
int doHelp(int argc, char *argv[]);
const CliCmdType CMD_HELP =
{
	"-h",
	1,
	&doHelp,
	"\t-h		Display the list of command options or one command option details\n",
	"\tUsage:		ioplus -h    Display command options list\n",
	"\tUsage:		ioplus -h <param>   Display help for <param> command option\n",
	"\tExample:		ioplus -h rread    Display help for \"rread\" command option\n"};

int doHelp(int argc, char *argv[])
{
	int i = 0;
	if (argc == 3)
	{
		while (NULL != gCmdArray[i])
		{
			if (gCmdArray[i]->name != NULL)
			{
				if (strcasecmp(argv[2], gCmdArray[i]->name) == 0)
				{
					printf("%s%s%s%s", gCmdArray[i]->help, gCmdArray[i]->usage1,
						gCmdArray[i]->usage2, gCmdArray[i]->example);
					break;
				}
			}
			i++;
		}
		if (NULL == gCmdArray[i])
		{
			printf("Option \"%s\" not found\n", argv[2]);
			i = 0;
			while (NULL != gCmdArray[i])
			{
				if (gCmdArray[i]->name != NULL)
				{
					printf("%s", gCmdArray[i]->help);
					break;
				}
				i++;
			}
		}
	}
	else
	{
		i = 0;
		while (NULL != gCmdArray[i])
		{
			if (gCmdArray[i]->name != NULL)
			{
				printf("%s", gCmdArray[i]->help);
			}
			i++;
		}
	}
	return OK;
}

int doVersion(int argc, char *argv[]);
const CliCmdType CMD_VERSION =
{
	"-v",
	1,
	&doVersion,
	"\t-v		Display the ioplus command version number\n",
	"\tUsage:		ioplus -v\n",
	"",
	"\tExample:		ioplus -v  Display the version number\n"};

int doVersion(int argc, char *argv[])
{
	UNUSED(argc);
	UNUSED(argv);
	printf("ioplus v%d.%d.%d Copyright (c) 2016 - 2020 Sequent Microsystems\n",
	VERSION_BASE, VERSION_MAJOR, VERSION_MINOR);
	printf("\nThis is free software with ABSOLUTELY NO WARRANTY.\n");
	printf("For details type: ioplus -warranty\n");
	return OK;
}

int doWarranty(int argc, char *argv[]);
const CliCmdType CMD_WAR =
{
	"-warranty",
	1,
	&doWarranty,
	"\t-warranty	Display the warranty\n",
	"\tUsage:		ioplus -warranty\n",
	"",
	"\tExample:		ioplus -warranty  Display the warranty text\n"};

int doWarranty(int argc UNU, char *argv[] UNU)
{
	printf("%s\n", warranty);
	return OK;
}

int doDispPinout(int argc, char *argv[]);
const CliCmdType CMD_PINOUT =
{
	"-pinout",
	1,
	&doDispPinout,
	"\t-pinout		Display the board io connector pinout\n",
	"\tUsage:		ioplus -pinout\n",
	"",
	"\tExample:		ioplus -pinout  Display the board io connector pinout\n"};

int doDispPinout(int argc UNU, char *argv[] UNU)
{
	printf("%s\n", cn);
	return OK;
}

int doList(int argc, char *argv[]);
const CliCmdType CMD_LIST =
	{
		"-list",
		1,
		&doList,
		"\t-list:		List all ioplus boards connected,return the # of boards and stack level for every board\n",
		"\tUsage:		ioplus -list\n",
		"",
		"\tExample:		ioplus -list display: 1,0 \n"};

int doList(int argc, char *argv[])
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
	return OK;
}

int doBoard(int argc, char *argv[]);
const CliCmdType CMD_BOARD =
{
	"board",
	2,
	&doBoard,
	"\tboard		Display the board status and firmware version number\n",
	"\tUsage:		ioplus <stack> board\n",
	"",
	"\tExample:		ioplus 0 board  Display vcc, temperature, firmware version \n"};

int doBoard(int argc, char *argv[])
{
	int dev = -1;
	u8 buff[4];
	int resp = 0;
	int temperature = 25;
	float voltage = 3.3;

	if (argc != 3)
	{
		printf("Invalid arguments number type \"ioplus -h\" for details\n");
		exit(1);
	}
	dev = doBoardInit(atoi(argv[1]));
	if (dev <= 0)
	{
		exit(1);
	}
	resp = i2cMem8Read(dev, I2C_MEM_DIAG_TEMPERATURE_ADD, buff, 3);
	if (FAIL == resp)
	{
		printf("Fail to read board info!\n");
		exit(1);
	}
	temperature = buff[0];
	memcpy(&resp, &buff[1], 2);
	voltage = (float)resp / 1000; //read in milivolts

	resp = i2cMem8Read(dev, I2C_MEM_REVISION_HW_MAJOR_ADD, buff, 4);
	if (FAIL == resp)
	{
		printf("Fail to read board info!\n");
		exit(1);
	}
	printf(
		"Hardware %02d.%02d, Firmware %02d.%02d, CPU temperature %d C, voltage %0.2f V\n",
		(int)buff[0], (int)buff[1], (int)buff[2], (int)buff[3], temperature,
		voltage);
	return OK;
}
#ifdef HW_DEBUG
#define ERR_FIFO_MAX_SIZE 512
int doGetErrors(int argc, char *argv[]);
const CliCmdType CMD_ERR =
{
	"err",
	2,
	&doGetErrors,
	"\terr		Display the board logged errors \n",
	"\tUsage:		ioplus <stack> err\n",
	"",
	"\tExample:		ioplus 0 err  Display errors strings readed from the board \n"};

int doGetErrors(int argc, char *argv[])
{
	int dev = -1;
	u8 buff[ERR_FIFO_MAX_SIZE];
	int resp = 0;
	u16 size = 0;
	int retry = 0;

	if (argc != 3)
	{
		printf("Invalid arguments number type \"ioplus -h\" for details\n");
		exit(1);
	}
	dev = doBoardInit(atoi(argv[1]));
	if (dev <= 0)
	{
		exit(1);
	}
	buff[0] = 1;
	resp = i2cMem8Write(dev, I2C_DBG_CMD, buff, 1);
	while ( (size == 0) && (retry < 10))
	{
		resp = i2cMem8Read(dev, I2C_DBG_FIFO_SIZE, buff, 2);
		if (FAIL != resp)
		{
			memcpy(&size, buff, 2);
		}
		retry++;
	}
	if (0 == size)
	{
		printf("Fail to read board error log, fifo empty!\n");
		exit(1);
	}
	if (size > ERR_FIFO_MAX_SIZE)
	{
		size = ERR_FIFO_MAX_SIZE;
	}
	resp = i2cMem8Read(dev, I2C_DBG_FIFO_ADD, buff, size);
	if (FAIL == resp)
	{
		printf("Fail to read board error log, fifo read %d bytes error!\n",
		(int)size);
		exit(1);
	}
	buff[size - 1] = 0;

	printf("%s\n", (char*)buff);
	for (retry = 0; retry < size; retry++)
	{
		printf("%02x ", buff[retry]);
		if (0 == ((retry + 1) % 16))
		{
			printf("\n");
		}
	}
	printf("\n");
}
#endif

int relayChSet(int dev, u8 channel, OutStateEnumType state)
{
	int resp = 0;
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

int relayChGet(int dev, u8 channel, OutStateEnumType *state)
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

//	if (FAIL == i2cMem8Read(dev, I2C_MEM_RELAY_VAL_ADD, buff, 1))
//	{
//		return ERROR;
//	}
	if (OK != i2cReadByteAS(dev, I2C_MEM_RELAY_VAL_ADD, buff))
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

int relayGet(int dev, int *val)
{
	u8 buff[2];

	if (NULL == val)
	{
		return ERROR;
	}
	if (OK != i2cReadByteAS(dev, I2C_MEM_RELAY_VAL_ADD, buff))
	{
		return ERROR;
	}
	*val = buff[0];
	return OK;
}

int doRelayWrite(int argc, char *argv[]);
const CliCmdType CMD_RELAY_WRITE =
{
	"relwr",
	2,
	&doRelayWrite,
	"\trelwr:		Set relays On/Off\n",
	"\tUsage:		ioplus <id> relwr <channel> <on/off>\n",
	"\tUsage:		ioplus <id> relwr <value>\n",
	"\tExample:		ioplus 0 relwr 2 1; Set Relay #2 on Board #0 On\n"};

int doRelayWrite(int argc, char *argv[])
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
	return OK;
}

int doRelayRead(int argc, char *argv[]);
const CliCmdType CMD_RELAY_READ =
{
	"relrd",
	2,
	&doRelayRead,
	"\trelrd:		Read relays status\n",
	"\tUsage:		ioplus <id> relrd <channel>\n",
	"\tUsage:		ioplus <id> relrd\n",
	"\tExample:		ioplus 0 relrd 2; Read Status of Relay #2 on Board #0\n"};

int doRelayRead(int argc, char *argv[])
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
	return OK;
}

int doRelayTest(int argc, char *argv[]);
const CliCmdType CMD_TEST =
{
	"reltest",
	2,
	&doRelayTest,
	"\treltest:	Turn ON and OFF the relays until press a key\n",
	"\tUsage:		ioplus <id> reltest\n",
	"",
	"\tExample:		ioplus 0 reltest\n"};

int doRelayTest(int argc, char *argv[])
{
	int dev = 0;
	int i = 0;
	int retry = 0;
	int relVal;
	int valR;
	int relayResult = 0;
	FILE *file = NULL;
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
	return OK;
}

int doGpioWrite(int argc, char *argv[]);
const CliCmdType CMD_GPIO_WRITE =
{
	"gpiowr",
	2,
	&doGpioWrite,
	"\tgpiowr:		Set gpio pins On/Off\n",
	"\tUsage:		ioplus <id> gpiowr <channel> <on/off>\n",
	"\tUsage:		ioplus <id> gpiowr <value>\n",
	"\tExample:		ioplus 0 gpiowr 2 1; Set GPIO pin #2 on Board #0 to 1 logic\n"};

const CliCmdType CMD_GPIO_READ =
{
	"gpiord",
	2,
	&doGpioRead,
	"\tgpiord:		Read gpio status\n",
	"\tUsage:		ioplus <id> gpiord <channel>\n",
	"\tUsage:		ioplus <id> gpiord\n",
	"\tExample:		ioplus 0 gpiord 2; Read Status of Gpio pin #2 on Board #0\n"};

const CliCmdType CMD_GPIO_DIR_WRITE =
{
	"gpiodirwr",
	2,
	&doGpioDirWrite,
	"\tgpiodirwr:	Set gpio pins direction I/O  0- output; 1-input\n",
	"\tUsage:		ioplus <id> gpiodirwr <channel> <out/in> \n",
	"\tUsage:		ioplus <id> gpiodirwr <value>\n",
	"\tExample:	ioplus 0 gpiodirwr 2 1; Set GPIO pin #2 on Board #0 as input\n"};

const CliCmdType CMD_GPIO_DIR_READ =
	{
		"gpiodirrd",
		2,
		&doGpioDirRead,
		"\tgpiodirrd:	Read gpio direction 0 - output; 1 - input\n",
		"\tUsage:		ioplus <id> gpiodirrd <pin>\n",
		"\tUsage:		ioplus <id> gpiodirrd\n",
		"\tExample:		ioplus 0 gpiodirrd 2; Read direction of Gpio pin #2 on Board #0\n"};

const CliCmdType CMD_GPIO_EDGE_WRITE =
	{
		"gpioedgewr",
		2,
		&doGpioEdgeWrite,
		"\tgpioedgewr:	Set gpio pin counting edges  0- count disable; 1-count rising edges; 2 - count falling edges; 3 - count both edges\n",
		"\tUsage:		ioplus <id> gpioedgewr <channel> <edges> \n",
		"",
		"\tExample:	ioplus 0 gpioedgewr 2 1; Set GPIO pin #2 on Board #0 to count rising edges\n"};

const CliCmdType CMD_GPIO_EDGE_READ =
	{
		"gpioedgerd",
		2,
		&doGpioEdgeRead,
		"\tgpioEdgerd:	Read gpio counting edges 0 - none; 1 - rising; 2 - falling; 3 - both\n",
		"\tUsage:		ioplus <id> gpioedgerd <pin>\n",
		"",
		"\tExample:		ioplus 0 gpioedgerd 2; Read counting edges of Gpio pin #2 on Board #0\n"};

const CliCmdType CMD_GPIO_CNT_READ =
{
	"gpiocntrd",
	2,
	&doGpioCntRead,
	"\tgpiocntrd:	Read gpio edges count for one GPIO imput pin\n",
	"\tUsage:		ioplus <id> gpiocntrd <channel>\n",
	"",
	"\tExample:		ioplus 0 gpiocntrd 2; Read contor of Gpio pin #2 on Board #0\n"};

const CliCmdType CMD_GPIO_CNT_RESET =
	{
		"gpiocntrst",
		2,
		&doGpioCntRst,
		"\tgpiocntrst:	Reset gpio edges count for one GPIO imput pin\n",
		"\tUsage:		ioplus <id> gpiocntrst <channel>\n",
		"",
		"\tExample:		ioplus 0 gpiocntrst 2; Reset contor of Gpio pin #2 on Board #0\n"};

const CliCmdType CMD_OPTO_READ =
	{
		"optrd",
		2,
		&doOptoRead,
		"\toptrd:		Read optocoupled inputs status\n",
		"\tUsage:		ioplus <id> optrd <channel>\n",
		"\tUsage:		ioplus <id> optrd\n",
		"\tExample:		ioplus 0 optrd 2; Read Status of Optocoupled input ch #2 on Board #0\n"};

const CliCmdType CMD_OPTO_EDGE_WRITE =
	{
		"optedgewr",
		2,
		&doOptoEdgeWrite,
		"\toptedgewr:	Set optocoupled channel counting edges  0- count disable; 1-count rising edges; 2 - count falling edges; 3 - count both edges\n",
		"\tUsage:		ioplus <id> optedgewr <channel> <edges> \n",
		"",
		"\tExample:	ioplus 0 optedgewr 2 1; Set Optocoupled channel #2 on Board #0 to count rising edges\n"};

const CliCmdType CMD_OPTO_EDGE_READ =
	{
		"optedgerd",
		2,
		&doOptoEdgeRead,
		"\toptedgerd:	Read optocoupled counting edges 0 - none; 1 - rising; 2 - falling; 3 - both\n",
		"\tUsage:		ioplus <id> optedgerd <pin>\n",
		"",
		"\tExample:		ioplus 0 optedgerd 2; Read counting edges of optocoupled channel #2 on Board #0\n"};

const CliCmdType CMD_OPTO_CNT_READ =
	{
		"optcntrd",
		2,
		&doOptoCntRead,
		"\toptcntrd:	Read potocoupled inputs edges count for one pin\n",
		"\tUsage:		ioplus <id> optcntrd <channel>\n",
		"",
		"\tExample:		ioplus 0 optcntrd 2; Read contor of opto input #2 on Board #0\n"};

const CliCmdType CMD_OPTO_CNT_RESET =
	{
		"optcntrst",
		2,
		&doOptoCntReset,
		"\toptcntrst:	Reset optocoupled inputs edges count for one pin\n",
		"\tUsage:		ioplus <id> optcntrst <channel>\n",
		"",
		"\tExample:		ioplus 0 optcntrst 2; Reset contor of opto input #2 on Board #0\n"};

const CliCmdType CMD_OPTO_ENC_WRITE =
	{
		"optencwr",
		2,
		&doOptoEncoderWrite,
		"\toptencwr:	Enable / Disable optocoupled quadrature encoder, encoder 1 connected to opto ch1 and 2, encoder 2 on ch3 and 4 ... \n",
		"\tUsage:		ioplus <id> optencwr <channel> <0/1> \n",
		"",
		"\tExample:	ioplus 0 optencwr 2 1; Enable Optocoupled channel encoder  #2  on Board \n"};

const CliCmdType CMD_OPTO_ENC_READ =
	{
		"optencrd",
		2,
		&doOptoEncoderRead,
		"\toptencrd:	Read optocoupled quadrature encoder state 0- disabled 1 - enabled\n",
		"\tUsage:		ioplus <id> optencrd <pin>\n",
		"",
		"\tExample:		ioplus 0 optencrd 2; Read state of optocoupled encoder channel #2 on Board #0\n"};

const CliCmdType CMD_OPTO_ENC_CNT_READ =
	{
		"optcntencrd",
		2,
		&doOptoEncoderCntRead,
		"\toptcntencrd:	Read potocoupled encoder count for one channel\n",
		"\tUsage:		ioplus <id> optcntencrd <channel>\n",
		"",
		"\tExample:		ioplus 0 optcntencrd 2; Read contor of opto encoder #2 on Board #0\n"};

const CliCmdType CMD_OPTO_ENC_CNT_RESET =
	{
		"optcntencrst",
		2,
		&doOptoEncoderCntReset,
		"\toptcntencrst:	Reset optocoupled encoder count \n",
		"\tUsage:		ioplus <id> optcntencrst <channel>\n",
		"",
		"\tExample:		ioplus 0 optcntencrst 2; Reset contor of encoder #2 on Board #0\n"};

int odGet(int dev, int ch, float *val)
{
	u16 raw = 0;

	if ( (ch < CHANNEL_NR_MIN) || (ch > OD_CH_NR_MAX))
	{
		printf("Open drain channel out of range!\n");
		return ERROR;
	}
	if (OK != i2cReadWordAS(dev, I2C_MEM_OD_PWM_VAL_RAW_ADD + 2 * (ch - 1), &raw))
	{
		printf("Fail to read!\n");
		return ERROR;
	}
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

int doOdRead(int argc, char *argv[]);
const CliCmdType CMD_OD_READ =
	{
		"odrd",
		2,
		&doOdRead,
		"\todrd:		Read open drain output pwm value (0% - 100%)\n",
		"\tUsage:		ioplus <id> odrd <channel>\n",
		"",
		"\tExample:		ioplus 0 odrd 2; Read pwm value of open drain channel #2 on Board #0\n"};

int doOdRead(int argc, char *argv[])
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
	return OK;
}

int doOdWrite(int argc, char *argv[]);
const CliCmdType CMD_OD_WRITE =
	{
		"odwr",
		2,
		&doOdWrite,
		"\todwr:		Write open drain output pwm value (0% - 100%), Warning: This function change the output of the coresponded DAC channel\n",
		"\tUsage:		ioplus <id> odwr <channel> <value>\n",
		"",
		"\tExample:		ioplus 0 odwr 2 12.5; Write pwm 12.5% to open drain channel #2 on Board #0\n"};

int doOdWrite(int argc, char *argv[])
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
	return OK;
}

int dacGet(int dev, int ch, float *val)
{
	u16 raw = 0;

	if ( (ch < CHANNEL_NR_MIN) || (ch > DAC_CH_NR_MAX))
	{
		printf("DAC channel out of range!\n");
		return ERROR;
	}
	if (OK != i2cReadWordAS(dev, I2C_MEM_DAC_VAL_MV_ADD + 2 * (ch - 1), &raw))
	{
		printf("Fail to read!\n");
		return ERROR;
	}
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

int doDacRead(int argc, char *argv[]);
const CliCmdType CMD_DAC_READ =
	{
		"dacrd",
		2,
		&doDacRead,
		"\tdacrd:		Read DAC voltage value (0 - 10V)\n",
		"\tUsage:		ioplus <id> dacrd <channel>\n",
		"",
		"\tExample:		ioplus 0 dacrd 2; Read the voltage on DAC channel #2 on Board #0\n"};

int doDacRead(int argc, char *argv[])
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
	return OK;
}

int doDacWrite(int argc, char *argv[]);
const CliCmdType CMD_DAC_WRITE =
	{
		"dacwr",
		2,
		&doDacWrite,
		"\tdacwr:		Write DAC output voltage value (0..10V), Warning: This function change the output of the coresponded open-drain channel\n",
		"\tUsage:		ioplus <id> dacwr <channel> <value>\n",
		"",
		"\tExample:		ioplus 0 dacwr 2 2.5; Write 2.5V to DAC channel #2 on Board #0\n"};

int doDacWrite(int argc, char *argv[])
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
	return OK;
}

int adcGet(int dev, int ch, float *val)
{
	u16 raw = 0;

	if ( (ch < CHANNEL_NR_MIN) || (ch > ADC_CH_NR_MAX))
	{
		printf("ADC channel out of range!\n");
		return ERROR;
	}
	if (OK != i2cReadWordAS(dev, I2C_MEM_ADC_VAL_MV_ADD + 2 * (ch - 1), &raw))
	{
		printf("Fail to read!\n");
		return ERROR;
	}
	*val = (float)raw / 1000;
	return OK;
}

int doAdcRead(int argc, char *argv[]);
const CliCmdType CMD_ADC_READ =
	{
		"adcrd",
		2,
		&doAdcRead,
		"\tadcrd:		Read ADC input voltage value (0 - 3.3V)\n",
		"\tUsage:		ioplus <id> adcrd <channel>\n",
		"",
		"\tExample:		ioplus 0 adcrd 2; Read the voltage input on ADC channel #2 on Board #0\n"};

int doAdcRead(int argc, char *argv[])
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
	return OK;
}

void getCalStat(int dev)
{
	u8 buff[2];

	busyWait(100);
	if (OK != i2cReadByteAS(dev, I2C_MEM_CALIB_STATUS, buff))
	{
		printf("Fail to read calibration status!\n");
		exit(1);
	}
	switch (buff[0])
	{
	case 0:
		printf("Calibration in progress\n");
		break;
	case 1:
		printf("Calibration done\n");
		break;
	case 2:
		printf("Calibration error!\n");
		break;
	default:
		printf("Unknown calibration status\n");
		break;
	}
}

int doAdcCal(int argc, char *argv[]);
const CliCmdType CMD_ADC_CAL =
	{
		"adccal",
		2,
		&doAdcCal,
		"\tadccal:		Calibrate one ADC channel, the calibration must be done in 2 points at min 2V apart\n",
		"\tUsage:		ioplus <id> adccal <channel> <value>\n",
		"",
		"\tExample:		ioplus 0 adccal 2 0.5; Calibrate the voltage input on ADC channel #2 on Board #0 at 0.5V\n"};

int doAdcCal(int argc, char *argv[])
{
	int ch = 0;
	float val = 0;
	int dev = 0;
	u8 buff[4] =
	{
		0,
		0};
	u16 raw = 0;

	dev = doBoardInit(atoi(argv[1]));
	if (dev <= 0)
	{
		exit(1);
	}

	if (argc == 5)
	{
		ch = atoi(argv[3]);
		if ( (ch < CHANNEL_NR_MIN) || (ch > ADC_CH_NR_MAX))
		{
			printf("ADC channel out of range!\n");
			exit(1);
		}

		val = atof(argv[4]);
		if ( (val < 0) || (val > 3.3))
		{
			printf("ADC calibration value out of range!\n");
			exit(1);
		}
		raw = (u16)ceil(val * VOLT_TO_MILIVOLT);
		memcpy(buff, &raw, 2);
		buff[2] = ch;
		buff[3] = CALIBRATION_KEY;

		if (OK != i2cMem8Write(dev, I2C_MEM_CALIB_VALUE, buff, 4))
		{
			printf("Fail to write calibration data!\n");
			exit(1);
		}

		getCalStat(dev);
	}
	else
	{
		printf("Invalid params number:\n %s", CMD_ADC_CAL.usage1);
		exit(1);
	}
	return OK;
}

int doAdcCalRst(int argc, char *argv[]);
const CliCmdType CMD_ADC_CAL_RST =
	{
		"adccalrst",
		2,
		&doAdcCalRst,
		"\tadccalrst:	Reset the calibration for one ADC channel\n",
		"\tUsage:		ioplus <id> adccalrst <channel>\n",
		"",
		"\tExample:		ioplus 0 adccalrst 2 ; Reset the calibration on ADC channel #2 on Board #0 at factory default\n"};

int doAdcCalRst(int argc, char *argv[])
{
	int ch = 0;

	int dev = 0;
	u8 buff[4] =
	{
		0,
		0,
		0,
		0};

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

		buff[2] = ch;
		buff[3] = RESET_CALIBRATION_KEY;

		if (OK != i2cMem8Write(dev, I2C_MEM_CALIB_VALUE, buff, 4))
		{
			printf("Fail to write calibration data!\n");
			exit(1);
		}

		getCalStat(dev);
	}
	else
	{
		printf("Invalid params number:\n %s", CMD_ADC_CAL_RST.usage1);
		exit(1);
	}
	return OK;
}

int doDacCal(int argc, char *argv[]);
const CliCmdType CMD_DAC_CAL =
	{
		"daccal",
		2,
		&doDacCal,
		"\tdaccal:		Calibrate one DAC channel, the calibration must be done in 2 points at min 5V apart\n",
		"\tUsage:		ioplus <id> daccal <channel> <value>\n",
		"",
		"\tExample:		ioplus 0 daccal 2 0.5; Calibrate the voltage outputs on DAC channel #2 on Board #0 at 0.5V\n"};

int doDacCal(int argc, char *argv[])
{
	int ch = 0;
	float val = 0;
	int dev = 0;
	u8 buff[4] =
	{
		0,
		0};
	u16 raw = 0;

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

		val = atof(argv[4]);
		if ( (val < 0) || (val > 10))
		{
			printf("DAC calibration value out of range!\n");
			exit(1);
		}
		raw = (u16)ceil(val * VOLT_TO_MILIVOLT);
		memcpy(buff, &raw, 2);
		buff[2] = ch + ADC_CH_NR_MAX;
		buff[3] = CALIBRATION_KEY;

		if (OK != i2cMem8Write(dev, I2C_MEM_CALIB_VALUE, buff, 4))
		{
			printf("Fail to write calibration data!\n");
			exit(1);
		}

		getCalStat(dev);
	}
	else
	{
		printf("Invalid params number:\n %s", CMD_DAC_CAL.usage1);
		exit(1);
	}
	return OK;
}

int doDacCalRst(int argc, char *argv[]);
const CliCmdType CMD_DAC_CAL_RST =
	{
		"daccalrst",
		2,
		&doDacCalRst,
		"\tdaccalrst:	Reset calibration for one DAC channel\n",
		"\tUsage:		ioplus <id> daccalrst <channel>\n",
		"",
		"\tExample:		ioplus 0 daccalrst 2; Reset calibration data on DAC channel #2 on Board #0 at factory default\n"};

int doDacCalRst(int argc, char *argv[])
{
	int ch = 0;

	int dev = 0;
	u8 buff[4] =
	{
		0,
		0,
		0,
		0};
	u16 raw = 0;

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

		memcpy(buff, &raw, 2);
		buff[2] = ch + ADC_CH_NR_MAX;
		buff[3] = RESET_CALIBRATION_KEY;

		if (OK != i2cMem8Write(dev, I2C_MEM_CALIB_VALUE, buff, 4))
		{
			printf("Fail to write calibration data!\n");
			exit(1);
		}
		getCalStat(dev);
	}
	else
	{
		printf("Invalid params number:\n %s", CMD_DAC_CAL_RST.usage1);
		exit(1);
	}
	return OK;
}

int doWdtReload(int argc, char *argv[]);
const CliCmdType CMD_WDT_RELOAD =
	{
		"wdtr",
		2,
		&doWdtReload,
		"\twdtr:		Reload the watchdog timer and enable the watchdog if is disabled\n",
		"\tUsage:		ioplus <id> wdtr\n",
		"",
		"\tExample:		ioplus 0 wdtr; Reload the watchdog timer on Board #0 with the period \n"};

int doWdtReload(int argc, char *argv[])
{
	int dev = 0;
	u8 buff[2] =
	{
		0,
		0};

	dev = doBoardInit(atoi(argv[1]));
	if (dev <= 0)
	{
		exit(1);
	}

	if (argc == 3)
	{
		buff[0] = WDT_RESET_SIGNATURE;
		if (OK != i2cMem8Write(dev, I2C_MEM_WDT_RESET_ADD, buff, 1))
		{
			printf("Fail to write watchdog reset key!\n");
			exit(1);
		}
	}
	else
	{
		printf("Invalid params number:\n %s", CMD_WDT_RELOAD.usage1);
		exit(1);
	}
	return OK;
}

int doWdtSetPeriod(int argc, char *argv[]);
const CliCmdType CMD_WDT_SET_PERIOD =
	{
		"wdtpwr",
		2,
		&doWdtSetPeriod,
		"\twdtpwr:		Set the watchdog period in seconds, reload command must be issue in this interval to prevent Raspberry Pi power off\n",
		"\tUsage:		ioplus <id> wdtpwr <val> \n",
		"",
		"\tExample:		ioplus 0 wdtpwr 10; Set the watchdog timer period on Board #0 at 10 seconds \n"};

int doWdtSetPeriod(int argc, char *argv[])
{
	int dev = 0;
	u16 period;
	u8 buff[2] =
	{
		0,
		0};

	dev = doBoardInit(atoi(argv[1]));
	if (dev <= 0)
	{
		exit(1);
	}

	if (argc == 4)
	{
		period = (u16)atoi(argv[3]);
		if (0 == period)
		{
			printf("Invalid period!\n");
			exit(1);
		}
		memcpy(buff, &period, 2);
		if (OK != i2cMem8Write(dev, I2C_MEM_WDT_INTERVAL_SET_ADD, buff, 2))
		{
			printf("Fail to write watchdog period!\n");
			exit(1);
		}
	}
	else
	{
		printf("Invalid params number:\n %s", CMD_WDT_SET_PERIOD.usage1);
		exit(1);
	}
	return OK;
}

int doWdtGetPeriod(int argc, char *argv[]);
const CliCmdType CMD_WDT_GET_PERIOD =
	{
		"wdtprd",
		2,
		&doWdtGetPeriod,
		"\twdtprd:		Get the watchdog period in seconds, reload command must be issue in this interval to prevent Raspberry Pi power off\n",
		"\tUsage:		ioplus <id> wdtprd \n",
		"",
		"\tExample:		ioplus 0 wdtprd; Get the watchdog timer period on Board #0\n"};

int doWdtGetPeriod(int argc, char *argv[])
{
	int dev = 0;
	u16 period;

	dev = doBoardInit(atoi(argv[1]));
	if (dev <= 0)
	{
		exit(1);
	}

	if (argc == 3)
	{
		if (OK != i2cReadWordAS(dev, I2C_MEM_WDT_INTERVAL_GET_ADD, &period))
		{
			printf("Fail to read watchdog period!\n");
			exit(1);
		}
		printf("%d\n", (int)period);
	}
	else
	{
		printf("Invalid params number:\n %s", CMD_WDT_GET_PERIOD.usage1);
		exit(1);
	}
	return OK;
}

int doWdtSetInitPeriod(int argc, char *argv[]);
const CliCmdType CMD_WDT_SET_INIT_PERIOD =
	{
		"wdtipwr",
		2,
		&doWdtSetInitPeriod,
		"\twdtipwr:	Set the watchdog initial period in seconds, This period is loaded after power cycle, giving Raspberry time to boot\n",
		"\tUsage:		ioplus <id> wdtipwr <val> \n",
		"",
		"\tExample:		ioplus 0 wdtipwr 10; Set the watchdog timer initial period on Board #0 at 10 seconds \n"};

int doWdtSetInitPeriod(int argc, char *argv[])
{
	int dev = 0;
	u16 period;
	u8 buff[2] =
	{
		0,
		0};

	dev = doBoardInit(atoi(argv[1]));
	if (dev <= 0)
	{
		exit(1);
	}

	if (argc == 4)
	{
		period = (u16)atoi(argv[3]);
		if (0 == period)
		{
			printf("Invalid period!\n");
			exit(1);
		}
		memcpy(buff, &period, 2);
		if (OK != i2cMem8Write(dev, I2C_MEM_WDT_INIT_INTERVAL_SET_ADD, buff, 2))
		{
			printf("Fail to write watchdog period!\n");
			exit(1);
		}
	}
	else
	{
		printf("Invalid params number:\n %s", CMD_WDT_SET_INIT_PERIOD.usage1);
		exit(1);
	}
	return OK;
}

int doWdtGetInitPeriod(int argc, char *argv[]);
const CliCmdType CMD_WDT_GET_INIT_PERIOD =
	{
		"wdtiprd",
		2,
		&doWdtGetInitPeriod,
		"\twdtiprd:	Get the watchdog initial period in seconds. This period is loaded after power cycle, giving Raspberry time to boot\n",
		"\tUsage:		ioplus <id> wdtiprd \n",
		"",
		"\tExample:		ioplus 0 wdtiprd; Get the watchdog timer initial period on Board #0\n"};

int doWdtGetInitPeriod(int argc, char *argv[])
{
	int dev = 0;
	u16 period;

	dev = doBoardInit(atoi(argv[1]));
	if (dev <= 0)
	{
		exit(1);
	}

	if (argc == 3)
	{
		if (OK != i2cReadWordAS(dev, I2C_MEM_WDT_INIT_INTERVAL_GET_ADD, &period))
		{
			printf("Fail to read watchdog period!\n");
			exit(1);
		}
		printf("%d\n", (int)period);
	}
	else
	{
		printf("Invalid params number:\n %s", CMD_WDT_GET_INIT_PERIOD.usage1);
		exit(1);
	}
	return OK;
}

int doWdtSetOffPeriod(int argc, char *argv[]);
const CliCmdType CMD_WDT_SET_OFF_PERIOD =
	{
		"wdtopwr",
		2,
		&doWdtSetOffPeriod,
		"\twdtopwr:	Set the watchdog off period in seconds (max 48 days), This is the time that watchdog mantain Raspberry turned off \n",
		"\tUsage:		ioplus <id> wdtopwr <val> \n",
		"",
		"\tExample:		ioplus 0 wdtopwr 10; Set the watchdog off interval on Board #0 at 10 seconds \n"};

int doWdtSetOffPeriod(int argc, char *argv[])
{
	int dev = 0;
	u32 period;
	u8 buff[4] =
	{
		0,
		0,
		0,
		0};

	dev = doBoardInit(atoi(argv[1]));
	if (dev <= 0)
	{
		exit(1);
	}

	if (argc == 4)
	{
		period = (u32)atoi(argv[3]);
		if ( (0 == period) || (period > WDT_MAX_OFF_INTERVAL_S))
		{
			printf("Invalid period!\n");
			exit(1);
		}
		memcpy(buff, &period, 4);
		if (OK
			!= i2cMem8Write(dev, I2C_MEM_WDT_POWER_OFF_INTERVAL_SET_ADD, buff, 4))
		{
			printf("Fail to write watchdog period!\n");
			exit(1);
		}
	}
	else
	{
		printf("Invalid params number:\n %s", CMD_WDT_SET_OFF_PERIOD.usage1);
		exit(1);
	}
	return OK;
}

int doWdtGetOffPeriod(int argc, char *argv[]);
const CliCmdType CMD_WDT_GET_OFF_PERIOD =
	{
		"wdtoprd",
		2,
		&doWdtGetOffPeriod,
		"\twdtoprd:	Get the watchdog off period in seconds (max 48 days), This is the time that watchdog mantain Raspberry turned off \n",
		"\tUsage:		ioplus <id> wdtoprd \n",
		"",
		"\tExample:		ioplus 0 wdtoprd; Get the watchdog off period on Board #0\n"};

int doWdtGetOffPeriod(int argc, char *argv[])
{
	int dev = 0;
	u32 period;

	dev = doBoardInit(atoi(argv[1]));
	if (dev <= 0)
	{
		exit(1);
	}

	if (argc == 3)
	{
		if (OK != i2cReadDWordAS(dev, I2C_MEM_WDT_POWER_OFF_INTERVAL_GET_ADD, &period))
		{
			printf("Fail to read watchdog period!\n");
			exit(1);
		}
		printf("%d\n", (int)period);
	}
	else
	{
		printf("Invalid params number:\n %s", CMD_WDT_GET_OFF_PERIOD.usage1);
		exit(1);
	}
	return OK;
}

int doLoopbackTest(int argc, char *argv[]);
const CliCmdType CMD_IO_TEST =
{
	"iotst",
	2,
	&doLoopbackTest,
	"\tiotst:		Test the ioplus with loopback card inserted \n",
	"\tUsage:		ioplus <id> iotst\n",
	"",
	"\tExample:		ioplus 0 iotst; Run the tests \n"};

int doLoopbackTest(int argc, char *argv[])
{
	int dev = 0;
	u8 i = 0;
	OutStateEnumType state;
	int pass = 0;
	int total = 0;
	float val = 0;
	const u8 t1OptoCh[4] =
	{
		2,
		1,
		4,
		3};
	const u8 t2OptoCh[4] =
	{
		8,
		7,
		6,
		5};
	const u8 adcCh1[4] =
	{
		2,
		5,
		1,
		7};
	const u8 adcCh2[4] =
	{
		4,
		6,
		3,
		8};

	dev = doBoardInit(atoi(argv[1]));
	if (dev <= 0)
	{
		exit(1);
	}

	if (argc == 3)
	{
		//GPIO -> OPTO
//Opto ON		
		for (i = 0; i < 4; i++)
		{
			if (OK != gpioChDirSet(dev, i + 1, 0))
			{
				printf("Fail to set GPIO direction!\n");
				exit(1);
			}
			if (OK != gpioChSet(dev, i + 1, 0))
			{
				printf("Fail to set GPIO !\n");
				exit(1);
			}
			busyWait(50);
			if (OK != optoChGet(dev, t1OptoCh[i], &state))
			{
				printf("Fail to read opto!\n");
				exit(1);
			}
			total++;
			if (state == 1)
			{
				printf("Gpio %d to Opto %d Turn ON  PASS\n", (int)i + 1,
					(int)t1OptoCh[i]);
				pass++;
			}
			else
			{
				printf("Gpio %d to Opto %d Turn ON  FAIL!\n", (int)i + 1,
					(int)t1OptoCh[i]);
			}
//Opto OFF
			if (OK != gpioChDirSet(dev, i + 1, 1))
			{
				printf("Fail to set GPIO direction!\n");
				exit(1);
			}

			busyWait(50);
			if (OK != optoChGet(dev, t1OptoCh[i], &state))
			{
				printf("Fail to read opto!\n");
				exit(1);
			}
			total++;
			if (state == 0)
			{
				printf("Gpio %d to Opto %d Turn OFF  PASS\n", (int)i + 1,
					(int)t1OptoCh[i]);
				pass++;
			}
			else
			{
				printf("Gpio %d to Opto %d Turn OFF  FAIL!\n", (int)i + 1,
					(int)t1OptoCh[i]);
			}
		}

		//Open drain -> OPTO
//Opto ON		
		for (i = 0; i < 4; i++)
		{
			if (OK != odSet(dev, i + 1, 100))
			{
				printf("Fail to set Open drains output!\n");
				exit(1);
			}
			busyWait(250);
			if (OK != optoChGet(dev, t2OptoCh[i], &state))
			{
				printf("Fail to read opto!\n");
				exit(1);
			}
			total++;
			if (state == 1)
			{
				printf("OD %d to Opto %d Turn ON  PASS\n", (int)i + 1,
					(int)t2OptoCh[i]);
				pass++;
			}
			else
			{
				printf("OD %d to Opto %d Turn ON  FAIL!\n", (int)i + 1,
					(int)t2OptoCh[i]);
			}
//Opto OFF
			if (OK != odSet(dev, i + 1, 0))
			{
				printf("Fail to set Open drains output!\n");
				exit(1);
			}
			busyWait(250);
			if (OK != optoChGet(dev, t2OptoCh[i], &state))
			{
				printf("Fail to read opto!\n");
				exit(1);
			}
			total++;
			if (state == 0)
			{
				printf("OD %d to Opto %d Turn OFF  PASS\n", (int)i + 1,
					(int)t2OptoCh[i]);
				pass++;
			}
			else
			{
				printf("OD %d to Opto %d Turn OFF  FAIL!\n", (int)i + 1,
					(int)t2OptoCh[i]);
			}
		}
		//DAC -> ADC
//DAC 2V		
		for (i = 0; i < 4; i++)
		{

			if (OK != dacSet(dev, i + 1, 2))
			{
				printf("Fail to set DAC output!\n");
				exit(1);
			}
			busyWait(250);
			if (OK != adcGet(dev, adcCh1[i], &val))
			{
				printf("Fail to read adc\n");
				exit(1);
			}
			total++;
			if ( (val < 2.1) && (val > 1.9))
			{
				printf("DAC %d to ADC %d @2V  PASS\n", (int)i + 1, (int)adcCh1[i]);
				pass++;
			}
			else
			{
				printf("DAC %d to ADC %d @2V  FAIL!\n", (int)i + 1, (int)adcCh1[i]);
			}

			if (OK != adcGet(dev, adcCh2[i], &val))
			{
				printf("Fail to read adc\n");
				exit(1);
			}
			total++;
			if ( (val < 2.1) && (val > 1.9))
			{
				printf("DAC %d to ADC %d @2V  PASS\n", (int)i + 1, (int)adcCh2[i]);
				pass++;
			}
			else
			{
				printf("DAC %d to ADC %d @2V  FAIL!\n", (int)i + 1, (int)adcCh2[i]);
			}

			if (OK != dacSet(dev, i + 1, 0))
			{
				printf("Fail to set DAC output!\n");
				exit(1);
			}
			busyWait(250);
			if (OK != adcGet(dev, adcCh1[i], &val))
			{
				printf("Fail to read adc\n");
				exit(1);
			}
			total++;
			if ( (val < 0.1) && (val > -0.1))
			{
				printf("DAC %d to ADC %d @0V  PASS\n", (int)i + 1, (int)adcCh1[i]);
				pass++;
			}
			else
			{
				printf("DAC %d to ADC %d @0V  FAIL!\n", (int)i + 1, (int)adcCh1[i]);
			}

			if (OK != adcGet(dev, adcCh2[i], &val))
			{
				printf("Fail to read adc\n");
				exit(1);
			}
			total++;
			if ( (val < 0.1) && (val > -0.1))
			{
				printf("DAC %d to ADC %d @0V  PASS\n", (int)i + 1, (int)adcCh2[i]);
				pass++;
			}
			else
			{
				printf("DAC %d to ADC %d @0V  FAIL!\n", (int)i + 1, (int)adcCh2[i]);
			}

		}
		if (pass == total)
		{
			printf("\n === All tests PASS === \n");
		}
		else
		{
			printf("\n === Tests FAIL/from -> %d/%d !=== \n", total - pass, total);
		}
	}
	else
	{
		printf("Invalid params number:\n %s", CMD_IO_TEST.usage1);
		exit(1);
	}
	return OK;
}

const CliCmdType *gCmdArray[] =
{
	&CMD_VERSION,
	&CMD_HELP,
	&CMD_WAR,
	&CMD_PINOUT,
	&CMD_LIST,
	&CMD_BOARD,
#ifdef HW_DEBUG
	&CMD_ERR,
#endif
	&CMD_RELAY_WRITE,
	&CMD_RELAY_READ,
	&CMD_TEST,
	&CMD_GPIO_WRITE,
	&CMD_GPIO_READ,
	&CMD_GPIO_DIR_WRITE,
	&CMD_GPIO_DIR_READ,
	&CMD_GPIO_EDGE_WRITE,
	&CMD_GPIO_EDGE_READ,
	&CMD_GPIO_CNT_READ,
	&CMD_GPIO_CNT_RESET,
	&CMD_OPTO_READ,
	&CMD_OPTO_EDGE_READ,
	&CMD_OPTO_EDGE_WRITE,
	&CMD_OPTO_CNT_READ,
	&CMD_OPTO_CNT_RESET,
	&CMD_OPTO_ENC_WRITE,
	&CMD_OPTO_ENC_READ,
	&CMD_OPTO_ENC_CNT_READ,
	&CMD_OPTO_ENC_CNT_RESET,
	&CMD_OD_READ,
	&CMD_OD_WRITE,
	&CMD_DAC_READ,
	&CMD_DAC_WRITE,
	&CMD_ADC_READ,
	&CMD_ADC_CAL,
	&CMD_ADC_CAL_RST,
	&CMD_DAC_CAL,
	&CMD_DAC_CAL_RST,
	&CMD_WDT_RELOAD,
	&CMD_WDT_SET_PERIOD,
	&CMD_WDT_GET_PERIOD,
	&CMD_WDT_SET_INIT_PERIOD,
	&CMD_WDT_GET_INIT_PERIOD,
	&CMD_WDT_SET_OFF_PERIOD,
	&CMD_WDT_GET_OFF_PERIOD,
	&CMD_IO_TEST,
	NULL}; //null terminated array of cli structure pointers

int main(int argc, char *argv[])
{
	int i = 0;
	int ret = OK;

	if (argc == 1)
	{
		usage();
		return -1;
	}
	while (NULL != gCmdArray[i])
	{
		if ( (gCmdArray[i]->name != NULL) && (gCmdArray[i]->namePos < argc))
		{
			if (strcasecmp(argv[gCmdArray[i]->namePos], gCmdArray[i]->name) == 0)
			{
				ret = gCmdArray[i]->pFunc(argc, argv);
				if (ret == ARG_CNT_ERR)
				{
					printf("Invalid parameters number!\n");
					printf("%s", gCmdArray[i]->usage1);
					if (strlen(gCmdArray[i]->usage2) > 2)
					{
						printf("%s", gCmdArray[i]->usage2);
					}
				}
				return ret;
			}
		}
		i++;
	}
	printf("Invalid command option\n");
	usage();

	return -1;
}
