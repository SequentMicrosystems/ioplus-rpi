/*
 * ioplus.c:
 *	Command-line interface to the Raspberry
 *	Pi's IOPLUS card.
 *	Copyright (c) 2016-2023 Sequent Microsystem
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

#include <fcntl.h>
#include <sys/stat.h>
#include <semaphore.h>

#define VERSION_BASE	(int)1
#define VERSION_MAJOR	(int)3
#define VERSION_MINOR	(int)6

#define UNUSED(X) (void)X      /* To avoid gcc/g++ warnings */


#define THREAD_SAFE
#define MOVE_PROFILE

u8 gHwVer = 0;

char *warranty =
	"	       Copyright (c) 2016-2023 Sequent Microsystems\n"
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
	printf("Where: <stack> = Board level id = 0..7\n");
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
	if (ERROR == i2cMem8Read(dev, I2C_MEM_REVISION_HW_MAJOR_ADD, buff, 1))
	{
		printf("IO-PLUS id %d not detected\n", stack);
		return ERROR;
	}
	gHwVer = buff[0];
	return dev;
}

u8 getHwVer(void)
{
	return gHwVer;
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
const CliCmdType CMD_HELP = {"-h", 1, &doHelp,
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
const CliCmdType CMD_VERSION = {"-v", 1, &doVersion,
	"\t-v		Display the ioplus command version number\n", "\tUsage:		ioplus -v\n",
	"", "\tExample:		ioplus -v  Display the version number\n"};

int doVersion(int argc, char *argv[])
{
	UNUSED(argc);
	UNUSED(argv);
	printf("ioplus v%d.%d.%d Copyright (c) 2016 - 2023 Sequent Microsystems\n",
	VERSION_BASE, VERSION_MAJOR, VERSION_MINOR);
	printf("\nThis is free software with ABSOLUTELY NO WARRANTY.\n");
	printf("For details type: ioplus -warranty\n");
	return OK;
}

int doWarranty(int argc, char *argv[]);
const CliCmdType CMD_WAR = {"-warranty", 1, &doWarranty,
	"\t-warranty	Display the warranty\n", "\tUsage:		ioplus -warranty\n", "",
	"\tExample:		ioplus -warranty  Display the warranty text\n"};

int doWarranty(int argc UNU, char *argv[] UNU)
{
	printf("%s\n", warranty);
	return OK;
}

int doDispPinout(int argc, char *argv[]);
const CliCmdType CMD_PINOUT = {"-pinout", 1, &doDispPinout,
	"\t-pinout		Display the board io connector pinout\n",
	"\tUsage:		ioplus -pinout\n", "",
	"\tExample:		ioplus -pinout  Display the board io connector pinout\n"};

int doDispPinout(int argc UNU, char *argv[] UNU)
{
	printf("%s\n", cn);
	return OK;
}

int doList(int argc, char *argv[]);
const CliCmdType CMD_LIST =
	{"-list", 1, &doList,
		"\t-list:		List all ioplus boards connected,return the # of boards and stack level for every board\n",
		"\tUsage:		ioplus -list\n", "", "\tExample:		ioplus -list display: 1,0 \n"};

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
const CliCmdType CMD_BOARD = {"board", 2, &doBoard,
	"\tboard		Display the board status and firmware version number\n",
	"\tUsage:		ioplus <stack> board\n", "",
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
		return (ARG_ERR);
	}
	dev = doBoardInit(atoi(argv[1]));
	if (dev <= 0)
	{
		return (FAIL);
	}
	resp = i2cMem8Read(dev, I2C_MEM_DIAG_TEMPERATURE_ADD, buff, 3);
	if (FAIL == resp)
	{
		printf("Fail to read board info!\n");
		return (FAIL);
	}
	temperature = buff[0];
	memcpy(&resp, &buff[1], 2);
	voltage = (float)resp / 1000; //read in milivolts

	resp = i2cMem8Read(dev, I2C_MEM_REVISION_HW_MAJOR_ADD, buff, 4);
	if (FAIL == resp)
	{
		printf("Fail to read board info!\n");
		return (FAIL);
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
		return(FAIL);
	}
	dev = doBoardInit(atoi(argv[1]));
	if (dev <= 0)
	{
		return(FAIL);
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
		return(FAIL);
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
		return(FAIL);
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
const CliCmdType CMD_RELAY_WRITE = {"relwr", 2, &doRelayWrite,
	"\trelwr:		Set relays On/Off\n",
	"\tUsage:		ioplus <stack> relwr <channel> <on/off>\n",
	"\tUsage:		ioplus <stack> relwr <value>\n",
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
		return (FAIL);
	}

	dev = doBoardInit(atoi(argv[1]));
	if (dev <= 0)
	{
		return (FAIL);
	}
	if (argc == 5)
	{
		pin = atoi(argv[3]);
		if ( (pin < CHANNEL_NR_MIN) || (pin > RELAY_CH_NR_MAX))
		{
			printf("Relay number value out of range\n");
			return (FAIL);
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
				return (FAIL);
			}
			state = (OutStateEnumType)atoi(argv[4]);
		}

		retry = RETRY_TIMES;

		while ( (retry > 0) && (stateR != state))
		{
			if (OK != relayChSet(dev, pin, state))
			{
				printf("Fail to write relay\n");
				return (FAIL);
			}
			if (OK != relayChGet(dev, pin, &stateR))
			{
				printf("Fail to read relay\n");
				return (FAIL);
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
			return (FAIL);
		}
	}
	else
	{
		val = atoi(argv[3]);
		if (val < 0 || val > 255)
		{
			printf("Invalid relay value\n");
			return (FAIL);
		}

		retry = RETRY_TIMES;
		valR = -1;
		while ( (retry > 0) && (valR != val))
		{

			if (OK != relaySet(dev, val))
			{
				printf("Fail to write relay!\n");
				return (FAIL);
			}
			if (OK != relayGet(dev, &valR))
			{
				printf("Fail to read relay!\n");
				return (FAIL);
			}
		}
		if (retry == 0)
		{
			printf("Fail to write relay!\n");
			return (FAIL);
		}
	}
	return OK;
}

int doRelayRead(int argc, char *argv[]);
const CliCmdType CMD_RELAY_READ = {"relrd", 2, &doRelayRead,
	"\trelrd:		Read relays status\n",
	"\tUsage:		ioplus <stack> relrd <channel>\n",
	"\tUsage:		ioplus <stack> relrd\n",
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
		return (FAIL);
	}

	if (argc == 4)
	{
		pin = atoi(argv[3]);
		if ( (pin < CHANNEL_NR_MIN) || (pin > RELAY_CH_NR_MAX))
		{
			printf("Relay number value out of range!\n");
			return (FAIL);
		}

		if (OK != relayChGet(dev, pin, &state))
		{
			printf("Fail to read!\n");
			return (FAIL);
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
			return (FAIL);
		}
		printf("%d\n", val);
	}
	else
	{
		printf("%s", CMD_RELAY_READ.usage1);
		printf("%s", CMD_RELAY_READ.usage2);
		return (FAIL);
	}
	return OK;
}

int doRelayTest(int argc, char *argv[]);
const CliCmdType CMD_TEST = {"reltest", 2, &doRelayTest,
	"\treltest:	Turn ON and OFF the relays until press a key\n",
	"\tUsage:		ioplus <stack> reltest\n", "", "\tExample:		ioplus 0 reltest\n"};

int doRelayTest(int argc, char *argv[])
{
	int dev = 0;
	int i = 0;
	int retry = 0;
	int relVal;
	int valR;
	int relayResult = 0;
	FILE *file = NULL;
	const u8 relayOrder[8] = {1, 2, 3, 4, 5, 6, 7, 8};

	dev = doBoardInit(atoi(argv[1]));
	if (dev <= 0)
	{
		return (FAIL);
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
					return (FAIL);
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
					return (FAIL);
				}
				busyWait(150);
			}
		}
	}
	else
	{
		usage();
		return (FAIL);
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
const CliCmdType CMD_GPIO_WRITE = {"gpiowr", 2, &doGpioWrite,
	"\tgpiowr:		Set gpio pins On/Off\n",
	"\tUsage:		ioplus <stack> gpiowr <channel> <on/off>\n",
	"\tUsage:		ioplus <stack> gpiowr <value>\n",
	"\tExample:		ioplus 0 gpiowr 2 1; Set GPIO pin #2 on Board #0 to 1 logic\n"};

const CliCmdType CMD_GPIO_READ = {"gpiord", 2, &doGpioRead,
	"\tgpiord:		Read gpio status\n",
	"\tUsage:		ioplus <stack> gpiord <channel>\n",
	"\tUsage:		ioplus <stack> gpiord\n",
	"\tExample:		ioplus 0 gpiord 2; Read Status of Gpio pin #2 on Board #0\n"};

const CliCmdType CMD_GPIO_DIR_WRITE = {"gpiodirwr", 2, &doGpioDirWrite,
	"\tgpiodirwr:	Set gpio pins direction I/O  0- output; 1-input\n",
	"\tUsage:		ioplus <stack> gpiodirwr <channel> <out/in> \n",
	"\tUsage:		ioplus <stack> gpiodirwr <value>\n",
	"\tExample:	ioplus 0 gpiodirwr 2 1; Set GPIO pin #2 on Board #0 as input\n"};

const CliCmdType CMD_GPIO_DIR_READ =
	{"gpiodirrd", 2, &doGpioDirRead,
		"\tgpiodirrd:	Read gpio direction 0 - output; 1 - input\n",
		"\tUsage:		ioplus <stack> gpiodirrd <pin>\n",
		"\tUsage:		ioplus <stack> gpiodirrd\n",
		"\tExample:		ioplus 0 gpiodirrd 2; Read direction of Gpio pin #2 on Board #0\n"};

const CliCmdType CMD_GPIO_EDGE_WRITE =
	{"gpioedgewr", 2, &doGpioEdgeWrite,
		"\tgpioedgewr:	Set gpio pin counting edges  0- count disable; 1-count rising edges; 2 - count falling edges; 3 - count both edges\n",
		"\tUsage:		ioplus <stack> gpioedgewr <channel> <edges> \n", "",
		"\tExample:	ioplus 0 gpioedgewr 2 1; Set GPIO pin #2 on Board #0 to count rising edges\n"};

const CliCmdType CMD_GPIO_EDGE_READ =
	{"gpioedgerd", 2, &doGpioEdgeRead,
		"\tgpioEdgerd:	Read gpio counting edges 0 - none; 1 - rising; 2 - falling; 3 - both\n",
		"\tUsage:		ioplus <stack> gpioedgerd <pin>\n", "",
		"\tExample:		ioplus 0 gpioedgerd 2; Read counting edges of Gpio pin #2 on Board #0\n"};

const CliCmdType CMD_GPIO_CNT_READ = {"gpiocntrd", 2, &doGpioCntRead,
	"\tgpiocntrd:	Read gpio edges count for one GPIO imput pin\n",
	"\tUsage:		ioplus <stack> gpiocntrd <channel>\n", "",
	"\tExample:		ioplus 0 gpiocntrd 2; Read contor of Gpio pin #2 on Board #0\n"};

const CliCmdType CMD_GPIO_CNT_RESET =
	{"gpiocntrst", 2, &doGpioCntRst,
		"\tgpiocntrst:	Reset gpio edges count for one GPIO imput pin\n",
		"\tUsage:		ioplus <stack> gpiocntrst <channel>\n", "",
		"\tExample:		ioplus 0 gpiocntrst 2; Reset contor of Gpio pin #2 on Board #0\n"};

const CliCmdType CMD_GPIO_ENC_CNT_READ = {"cntencrd", 2, &doGpioEncoderCntRead,
	"\tcntencrd:	Read PLC Pi08 encoder count \n",
	"\tUsage:		ioplus <stack> cntencrd \n", "",
	"\tExample:		ioplus 0 cntencrd ; Read couter of the PLC Pi08 encoder \n"};

const CliCmdType CMD_GPIO_ENC_CNT_RESET = {"cntencrst", 2,
	&doGpioEncoderCntReset, "\tcntencrst:	Reset PLC Pi08 encoder count \n",
	"\tUsage:		ioplus <stack> cntencrst \n", "",
	"\tExample:		ioplus 0 cntencrst 2; Reset contor of the PLC Pi08 encoder\n"};

const CliCmdType CMD_OPTO_OD_CMD_SET =
	{"incmd", 2, &doInCmdSet,
		"\tincmd:	Set PLC Pi08 command for input channel \n",
		"\tUsage:		ioplus <stack> incmd <inCh> <outCh> <cnt>\n", "",
		"\tExample:		ioplus 0 incmd 2 1 1000; PLC Pi08 od channel 1 will start 1000 pulses on rising edge of the input channel 2\n"};

const CliCmdType CMD_OPTO_READ =
	{"optrd", 2, &doOptoRead, "\toptrd:		Read optocoupled inputs status\n",
		"\tUsage:		ioplus <stack> optrd <channel>\n",
		"\tUsage:		ioplus <stack> optrd\n",
		"\tExample:		ioplus 0 optrd 2; Read Status of Optocoupled input ch #2 on Board #0\n"};

const CliCmdType CMD_OPTO_EDGE_WRITE =
	{"optedgewr", 2, &doOptoEdgeWrite,
		"\toptedgewr:	Set optocoupled channel counting edges  0- count disable; 1-count rising edges; 2 - count falling edges; 3 - count both edges\n",
		"\tUsage:		ioplus <stack> optedgewr <channel> <edges> \n", "",
		"\tExample:	ioplus 0 optedgewr 2 1; Set Optocoupled channel #2 on Board #0 to count rising edges\n"};

const CliCmdType CMD_OPTO_EDGE_READ =
	{"optedgerd", 2, &doOptoEdgeRead,
		"\toptedgerd:	Read optocoupled counting edges 0 - none; 1 - rising; 2 - falling; 3 - both\n",
		"\tUsage:		ioplus <stack> optedgerd <pin>\n", "",
		"\tExample:		ioplus 0 optedgerd 2; Read counting edges of optocoupled channel #2 on Board #0\n"};

const CliCmdType CMD_OPTO_CNT_READ =
	{"optcntrd", 2, &doOptoCntRead,
		"\toptcntrd:	Read potocoupled inputs edges count for one pin\n",
		"\tUsage:		ioplus <stack> optcntrd <channel>\n", "",
		"\tExample:		ioplus 0 optcntrd 2; Read contor of opto input #2 on Board #0\n"};

const CliCmdType CMD_OPTO_CNT_RESET =
	{"optcntrst", 2, &doOptoCntReset,
		"\toptcntrst:	Reset optocoupled inputs edges count for one pin\n",
		"\tUsage:		ioplus <stack> optcntrst <channel>\n", "",
		"\tExample:		ioplus 0 optcntrst 2; Reset contor of opto input #2 on Board #0\n"};

const CliCmdType CMD_OPTO_ENC_WRITE =
	{"optencwr", 2, &doOptoEncoderWrite,
		"\toptencwr:	Enable / Disable optocoupled quadrature encoder, encoder 1 connected to opto ch1 and 2, encoder 2 on ch3 and 4 ... \n",
		"\tUsage:		ioplus <stack> optencwr <channel> <0/1> \n", "",
		"\tExample:	ioplus 0 optencwr 2 1; Enable encoder on opto channel 3/4  on Board stack level 0\n"};

const CliCmdType CMD_OPTO_ENC_READ =
	{"optencrd", 2, &doOptoEncoderRead,
		"\toptencrd:	Read optocoupled quadrature encoder state 0- disabled 1 - enabled\n",
		"\tUsage:		ioplus <stack> optencrd <channel>\n", "",
		"\tExample:		ioplus 0 optencrd 2; Read state of optocoupled encoder channel #2 on Board #0\n"};

const CliCmdType CMD_OPTO_ENC_CNT_READ =
	{"optcntencrd", 2, &doOptoEncoderCntRead,
		"\toptcntencrd:	Read potocoupled encoder count for one channel\n",
		"\tUsage:		ioplus <stack> optcntencrd <channel>\n", "",
		"\tExample:		ioplus 0 optcntencrd 2; Read contor of opto encoder #2 on Board #0\n"};

const CliCmdType CMD_OPTO_ENC_CNT_RESET =
	{"optcntencrst", 2, &doOptoEncoderCntReset,
		"\toptcntencrst:	Reset optocoupled encoder count \n",
		"\tUsage:		ioplus <stack> optcntencrst <channel>\n", "",
		"\tExample:		ioplus 0 optcntencrst 2; Reset contor of encoder #2 on Board #0\n"};

int odGet(int dev, int ch, float *val)
{
	u16 raw = 0;

	if ( (ch < CHANNEL_NR_MIN) || (ch > OD_CH_NR_MAX))
	{
		printf("Open drain channel out of range!\n");
		return ERROR;
	}
	if (OK
		!= i2cReadWordAS(dev, I2C_MEM_OD_PWM_VAL_RAW_ADD + 2 * (ch - 1), &raw))
	{
		printf("Fail to read!\n");
		return ERROR;
	}
	*val = 100 * (float)raw / OD_PWM_VAL_MAX;
	return OK;
}

int odSet(int dev, int ch, float val)
{
	u8 buff[2] = {0, 0};
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
	{"odrd", 2, &doOdRead,
		"\todrd:		Read open drain output pwm value (0% - 100%)\n",
		"\tUsage:		ioplus <stack> odrd <channel>\n", "",
		"\tExample:		ioplus 0 odrd 2; Read pwm value of open drain channel #2 on Board #0\n"};

int doOdRead(int argc, char *argv[])
{
	int ch = 0;
	float val = 0;
	int dev = 0;

	dev = doBoardInit(atoi(argv[1]));
	if (dev <= 0)
	{
		return (FAIL);
	}

	if (argc == 4)
	{
		ch = atoi(argv[3]);
		if ( (ch < CHANNEL_NR_MIN) || (ch > OD_CH_NR_MAX))
		{
			printf("Open drain channel out of range!\n");
			return (FAIL);
		}

		if (OK != odGet(dev, ch, &val))
		{
			printf("Fail to read!\n");
			return (FAIL);
		}

		printf("%0.2f\n", val);
	}
	else
	{
		printf("Invalid params number:\n %s", CMD_OD_READ.usage1);
		return (FAIL);
	}
	return OK;
}

int doOdWrite(int argc, char *argv[]);
const CliCmdType CMD_OD_WRITE =
	{"odwr", 2, &doOdWrite,
		"\todwr:		Write open drain output pwm value (0% - 100%), Warning: This function change the output of the coresponded DAC channel\n",
		"\tUsage:		ioplus <stack> odwr <channel> <value>\n", "",
		"\tExample:		ioplus 0 odwr 2 12.5; Write pwm 12.5% to open drain channel #2 on Board #0\n"};

int doOdWrite(int argc, char *argv[])
{
	int ch = 0;
	int dev = 0;
	float proc = 0;

	dev = doBoardInit(atoi(argv[1]));
	if (dev <= 0)
	{
		return (FAIL);
	}

	if (argc == 5)
	{
		ch = atoi(argv[3]);
		if ( (ch < CHANNEL_NR_MIN) || (ch > OD_CH_NR_MAX))
		{
			printf("Open drain channel out of range!\n");
			return (FAIL);
		}
		proc = atof(argv[4]);
		if (proc < 0 || proc > 100)
		{
			printf("Invalid open drain pwm value, must be 0..100 \n");
			return (FAIL);
		}

		if (OK != odSet(dev, ch, proc))
		{
			printf("Fail to write!\n");
			return (FAIL);
		}
		printf("done\n");
	}
	else
	{
		printf("Invalid params number:\n %s", CMD_OD_WRITE.usage1);
		return (FAIL);
	}
	return OK;
}

//----------------------------------- OD pulses --------------------------------------------------------
#define SINGLE_TRANSFER

int odWritePulses(int dev, int ch, unsigned int val)
{
	u8 buff[5] = {0, 0, 0, 0, 0};
	u32 raw = 0;

	if ( (ch < CHANNEL_NR_MIN) || (ch > 2 * OD_CH_NR_MAX))// channel from 5 to 8 are channel 1 to 4 in oposite direction
	{
		printf("Open drain channel out of range!\n");
		return ERROR;
	}
	raw = (u32)val;
	memcpy(buff, &raw, 4);

#ifdef SINGLE_TRANSFER
	buff[4] = ch;
	if (OK != i2cMem8Write(dev, I2C_MEM_OD_P_SET_VALUE, buff, 5)) // write the value
	{
		printf("Fail to write!\n");
		return ERROR;
	}

#else

	if (OK != i2cMem8Write(dev, I2C_MEM_OD_P_SET_VALUE, buff, 4)) // write the value
	{
		printf("Fail to write!\n");
		return ERROR;
	}
	buff[0] = ch;
	if (OK != i2cMem8Write(dev, I2C_MEM_OD_P_SET_CMD, buff, 1))// update command
	{
		printf("Fail to write!\n");
		return ERROR;
	}
#endif
	return OK;
}

int odResetPulses(int dev, int ch)
{
	return odWritePulses(dev, ch, 0);
}

int odReadPulses(int dev, int ch, unsigned int *val)
{
	u32 raw = 0;

	if ( (ch < CHANNEL_NR_MIN) || (ch > OD_CH_NR_MAX))
	{
		printf("Open drain channel out of range!\n");
		return ERROR;
	}
	if (OK != i2cReadDWord(dev, I2C_MEM_OD_PULSE_CNT_SET + 4 * (ch - 1), &raw))
	{
		printf("Fail to read!\n");
		return ERROR;
	}
	*val = raw;
	return OK;
}

int doOdCntRead(int argc, char *argv[]);
const CliCmdType CMD_OD_CNT_READ =
	{"odcrd", 2, &doOdCntRead,
		"\todcrd:		Read open drain remaining pulses to perform\n",
		"\tUsage:		ioplus <stack> odcrd <channel>\n", "",
		"\tExample:		ioplus 0 odcrd 2; Read remaining pulses to perform of open drain channel #2 on Board #0\n"};

int doOdCntRead(int argc, char *argv[])
{
	int ch = 0;
	unsigned int val = 0;
	int dev = 0;

	dev = doBoardInit(atoi(argv[1]));
	if (dev <= 0)
	{
		return (FAIL);
	}
	if (argc == 4)
	{
		ch = atoi(argv[3]);
		if (OK != odReadPulses(dev, ch, &val))
		{
			return (FAIL);
		}
		printf("%d\n", val);
	}
	else
	{
		return ARG_CNT_ERR;
	}
	return OK;
}

int doOdCntWrite(int argc, char *argv[]);
const CliCmdType CMD_OD_CNT_WRITE =
	{"odcwr", 2, &doOdCntWrite,
		"\todcwr:			Write open drain output pulses to perform, value 0..65535. The open-drain channel will output <value> # of pulses 50% fill factor with current pwm frequency\n",
		"\tUsage:		ioplus <stack> odcwr <channel> <value>\n", "",
		"\tExample:		ioplus 0 odwr 2 100; set 100 pulses to perform for open drain channel #2 on Board #0\n"};

int doOdCntWrite(int argc, char *argv[])
{
	int ch = 0;
	int dev = 0;
	long int inVal = 0;
	unsigned int value = 0;

	dev = doBoardInit(atoi(argv[1]));
	if (dev <= 0)
	{
		return (FAIL);
	}

	if (argc == 5)
	{
		ch = atoi(argv[3]);
		inVal = atol(argv[4]);
		value = (unsigned int)inVal;
		if (OK != odWritePulses(dev, ch, value))
		{
			return (FAIL);
		}
		printf("done\n");
	}
	else
	{
		printf("Invalid params number:\n %s", CMD_OD_WRITE.usage1);
		return (FAIL);
	}
	return OK;
}

int doOdCntReset(int argc, char *argv[]);
const CliCmdType CMD_OD_CNT_RST =
	{"odcrst", 2, &doOdCntReset,
		"\todcrst:			Reset open drain output pulses to perform\n",
		"\tUsage:		ioplus <stack> odcrst <channel>\n", "",
		"\tExample:		ioplus 0 odwr 2; stop pulses for open drain channel #2 on Board #0\n"};

int doOdCntReset(int argc, char *argv[])
{
	int ch = 0;
	int dev = 0;

	dev = doBoardInit(atoi(argv[1]));
	if (dev <= 0)
	{
		return (FAIL);
	}

	if (argc == 4)
	{
		ch = atoi(argv[3]);

		if (OK != odResetPulses(dev, ch))
		{
			return (FAIL);
		}
		printf("done\n");
	}
	else
	{
		printf("Invalid params number:\n %s", CMD_OD_CNT_RST.usage1);
		return (FAIL);
	}
	return OK;
}

// ------------------------------- DAC ------------------------------------------------------------------
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
	u8 buff[2] = {0, 0};
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
	{"dacrd", 2, &doDacRead, "\tdacrd:		Read DAC voltage value (0 - 10V)\n",
		"\tUsage:		ioplus <stack> dacrd <channel>\n", "",
		"\tExample:		ioplus 0 dacrd 2; Read the voltage on DAC channel #2 on Board #0\n"};

int doDacRead(int argc, char *argv[])
{
	int ch = 0;
	float val = 0;
	int dev = 0;

	dev = doBoardInit(atoi(argv[1]));
	if (dev <= 0)
	{
		return (FAIL);
	}

	if (argc == 4)
	{
		ch = atoi(argv[3]);
		if ( (ch < CHANNEL_NR_MIN) || (ch > DAC_CH_NR_MAX))
		{
			printf("DAC channel out of range!\n");
			return (FAIL);
		}

		if (OK != dacGet(dev, ch, &val))
		{
			printf("Fail to read!\n");
			return (FAIL);
		}

		printf("%0.3f\n", val);
	}
	else
	{
		printf("Invalid params number:\n %s", CMD_DAC_READ.usage1);
		return (FAIL);
	}
	return OK;
}

int doDacWrite(int argc, char *argv[]);
const CliCmdType CMD_DAC_WRITE =
	{"dacwr", 2, &doDacWrite,
		"\tdacwr:		Write DAC output voltage value (0..10V), Warning: This function change the output of the coresponded open-drain channel\n",
		"\tUsage:		ioplus <stack> dacwr <channel> <value>\n", "",
		"\tExample:		ioplus 0 dacwr 2 2.5; Write 2.5V to DAC channel #2 on Board #0\n"};

int doDacWrite(int argc, char *argv[])
{
	int ch = 0;
	int dev = 0;
	float volt = 0;

	dev = doBoardInit(atoi(argv[1]));
	if (dev <= 0)
	{
		return (FAIL);
	}

	if (argc == 5)
	{
		ch = atoi(argv[3]);
		if ( (ch < CHANNEL_NR_MIN) || (ch > DAC_CH_NR_MAX))
		{
			printf("DAC channel out of range!\n");
			return (FAIL);
		}
		volt = atof(argv[4]);
		if (volt < 0 || volt > 10)
		{
			printf("Invalid DAC voltage value, must be 0..10 \n");
			return (FAIL);
		}

		if (OK != dacSet(dev, ch, volt))
		{
			printf("Fail to write!\n");
			return (FAIL);
		}
		printf("done\n");
	}
	else
	{
		printf("Invalid params number:\n %s", CMD_DAC_WRITE.usage1);
		return (FAIL);
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
	{"adcrd", 2, &doAdcRead,
		"\tadcrd:		Read ADC input voltage value (0 - 3.3V)\n",
		"\tUsage:		ioplus <stack> adcrd <channel>\n", "",
		"\tExample:		ioplus 0 adcrd 2; Read the voltage input on ADC channel #2 on Board #0\n"};

int doAdcRead(int argc, char *argv[])
{
	int ch = 0;
	float val = 0;
	int dev = 0;

	dev = doBoardInit(atoi(argv[1]));
	if (dev <= 0)
	{
		return (FAIL);
	}

	if (argc == 4)
	{
		ch = atoi(argv[3]);
		if ( (ch < CHANNEL_NR_MIN) || (ch > ADC_CH_NR_MAX))
		{
			printf("ADC channel out of range!\n");
			return (FAIL);
		}

		if (OK != adcGet(dev, ch, &val))
		{
			printf("Fail to read!\n");
			return (FAIL);
		}

		printf("%0.3f\n", val);
	}
	else
	{
		printf("Invalid params number:\n %s", CMD_ADC_READ.usage1);
		return (FAIL);
	}
	return OK;
}

int adcGetMax(int dev, int ch, float *val)
{
	u16 raw = 0;

	if ( (ch < CHANNEL_NR_MIN) || (ch > ADC_CH_NR_MAX / 2))
	{
		printf("ADC channel for process min/max out of range!\n");
		return ERROR;
	}
	if (OK != i2cReadWordAS(dev, I2C_MEM_ADC_MAX + 2 * (ch - 1), &raw))
	{
		printf("Fail to read!\n");
		return ERROR;
	}
	*val = (float)raw / 1000;
	return OK;
}

int doAdcReadMax(int argc, char *argv[]);
const CliCmdType CMD_ADC_READ_MAX =
	{"adcrdmax", 2, &doAdcReadMax,
		"\tadcrdmax:		Read ADC input voltage maxim value (0 - 3.3V) Min calculated on last n samples\n",
		"\tUsage:		ioplus <stack> adcrdmax <channel>\n", "",
		"\tExample:		ioplus 0 adcrdmax 2; Read the maxim voltage input on ADC channel #2 on Board #0\n"};

int doAdcReadMax(int argc, char *argv[])
{
	int ch = 0;
	float val = 0;
	int dev = 0;

	dev = doBoardInit(atoi(argv[1]));
	if (dev <= 0)
	{
		return (FAIL);
	}

	if (argc == 4)
	{
		ch = atoi(argv[3]);
		if ( (ch < CHANNEL_NR_MIN) || (ch > ADC_CH_NR_MAX / 2))
		{
			printf("ADC channel for process min/max out of range!\n");
			return (FAIL);
		}

		if (OK != adcGetMax(dev, ch, &val))
		{
			printf("Fail to read!\n");
			return (FAIL);
		}

		printf("%0.3f\n", val);
	}
	else
	{
		printf("Invalid params number:\n %s", CMD_ADC_READ.usage1);
		return (FAIL);
	}
	return OK;
}

int adcGetMin(int dev, int ch, float *val)
{
	u16 raw = 0;

	if ( (ch < CHANNEL_NR_MIN) || (ch > ADC_CH_NR_MAX / 2))
	{
		printf("ADC channel for process min/max out of range!\n");
		return ERROR;
	}
	if (OK != i2cReadWordAS(dev, I2C_MEM_ADC_MIN + 2 * (ch - 1), &raw))
	{
		printf("Fail to read!\n");
		return ERROR;
	}
	*val = (float)raw / 1000;
	return OK;
}

int doAdcReadMin(int argc, char *argv[]);
const CliCmdType CMD_ADC_READ_MIN =
	{"adcrdmin", 2, &doAdcReadMin,
		"\tadcrdmin:		Read ADC input voltage minim value (0 - 3.3V). Min calculated on last n samples\n",
		"\tUsage:		ioplus <stack> adcrdmin <channel>\n", "",
		"\tExample:		ioplus 0 adcrdmin 2; Read the Minim voltage input on ADC channel #2 on Board #0\n"};

int doAdcReadMin(int argc, char *argv[])
{
	int ch = 0;
	float val = 0;
	int dev = 0;

	dev = doBoardInit(atoi(argv[1]));
	if (dev <= 0)
	{
		return (FAIL);
	}

	if (argc == 4)
	{
		ch = atoi(argv[3]);
		if ( (ch < CHANNEL_NR_MIN) || (ch > ADC_CH_NR_MAX / 2))
		{
			printf("ADC channel for process min/max out of range!\n");
			return (FAIL);
		}

		if (OK != adcGetMin(dev, ch, &val))
		{
			printf("Fail to read!\n");
			return (FAIL);
		}

		printf("%0.3f\n", val);
	}
	else
	{
		printf("Invalid params number:\n %s", CMD_ADC_READ.usage1);
		return (FAIL);
	}
	return OK;
}

int getCalStat(int dev)
{
	u8 buff[2];

	busyWait(100);
	if (OK != i2cReadByteAS(dev, I2C_MEM_CALIB_STATUS, buff))
	{
		printf("Fail to read calibration status!\n");
		return FAIL;
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
	return OK;
}

int doAdcCal(int argc, char *argv[]);
const CliCmdType CMD_ADC_CAL =
	{"adccal", 2, &doAdcCal,
		"\tadccal:		Calibrate one ADC channel, the calibration must be done in 2 points at min 2V apart\n",
		"\tUsage:		ioplus <stack> adccal <channel> <value>\n", "",
		"\tExample:		ioplus 0 adccal 2 0.5; Calibrate the voltage input on ADC channel #2 on Board #0 at 0.5V\n"};

int doAdcCal(int argc, char *argv[])
{
	int ch = 0;
	float val = 0;
	int dev = 0;
	u8 buff[4] = {0, 0};
	u16 raw = 0;

	dev = doBoardInit(atoi(argv[1]));
	if (dev <= 0)
	{
		return (FAIL);
	}

	if (argc == 5)
	{
		ch = atoi(argv[3]);
		if ( (ch < CHANNEL_NR_MIN) || (ch > ADC_CH_NR_MAX))
		{
			printf("ADC channel out of range!\n");
			return (FAIL);
		}

		val = atof(argv[4]);
		if ( (val < 0) || (val > 3.3))
		{
			printf("ADC calibration value out of range!\n");
			return (FAIL);
		}
		raw = (u16)ceil(val * VOLT_TO_MILIVOLT);
		memcpy(buff, &raw, 2);
		buff[2] = ch;
		buff[3] = CALIBRATION_KEY;

		if (OK != i2cMem8Write(dev, I2C_MEM_CALIB_VALUE, buff, 4))
		{
			printf("Fail to write calibration data!\n");
			return (FAIL);
		}

		getCalStat(dev);
	}
	else
	{
		printf("Invalid params number:\n %s", CMD_ADC_CAL.usage1);
		return (FAIL);
	}
	return OK;
}

int doAdcCalRst(int argc, char *argv[]);
const CliCmdType CMD_ADC_CAL_RST =
	{"adccalrst", 2, &doAdcCalRst,
		"\tadccalrst:	Reset the calibration for one ADC channel\n",
		"\tUsage:		ioplus <stack> adccalrst <channel>\n", "",
		"\tExample:		ioplus 0 adccalrst 2 ; Reset the calibration on ADC channel #2 on Board #0 at factory default\n"};

int doAdcCalRst(int argc, char *argv[])
{
	int ch = 0;

	int dev = 0;
	u8 buff[4] = {0, 0, 0, 0};

	dev = doBoardInit(atoi(argv[1]));
	if (dev <= 0)
	{
		return (FAIL);
	}

	if (argc == 4)
	{
		ch = atoi(argv[3]);
		if ( (ch < CHANNEL_NR_MIN) || (ch > ADC_CH_NR_MAX))
		{
			printf("ADC channel out of range!\n");
			return (FAIL);
		}

		buff[2] = ch;
		buff[3] = RESET_CALIBRATION_KEY;

		if (OK != i2cMem8Write(dev, I2C_MEM_CALIB_VALUE, buff, 4))
		{
			printf("Fail to write calibration data!\n");
			return (FAIL);
		}

		getCalStat(dev);
	}
	else
	{
		printf("Invalid params number:\n %s", CMD_ADC_CAL_RST.usage1);
		return (FAIL);
	}
	return OK;
}

int doDacCal(int argc, char *argv[]);
const CliCmdType CMD_DAC_CAL =
	{"daccal", 2, &doDacCal,
		"\tdaccal:		Calibrate one DAC channel, the calibration must be done in 2 points at min 5V apart\n",
		"\tUsage:		ioplus <stack> daccal <channel> <value>\n", "",
		"\tExample:		ioplus 0 daccal 2 0.5; Calibrate the voltage outputs on DAC channel #2 on Board #0 at 0.5V\n"};

int doDacCal(int argc, char *argv[])
{
	int ch = 0;
	float val = 0;
	int dev = 0;
	u8 buff[4] = {0, 0};
	u16 raw = 0;

	dev = doBoardInit(atoi(argv[1]));
	if (dev <= 0)
	{
		return (FAIL);
	}

	if (argc == 5)
	{
		ch = atoi(argv[3]);
		if ( (ch < CHANNEL_NR_MIN) || (ch > DAC_CH_NR_MAX))
		{
			printf("DAC channel out of range!\n");
			return (FAIL);
		}

		val = atof(argv[4]);
		if ( (val < 0) || (val > 10))
		{
			printf("DAC calibration value out of range!\n");
			return (FAIL);
		}
		raw = (u16)ceil(val * VOLT_TO_MILIVOLT);
		memcpy(buff, &raw, 2);
		buff[2] = ch + ADC_CH_NR_MAX;
		buff[3] = CALIBRATION_KEY;

		if (OK != i2cMem8Write(dev, I2C_MEM_CALIB_VALUE, buff, 4))
		{
			printf("Fail to write calibration data!\n");
			return (FAIL);
		}

		getCalStat(dev);
	}
	else
	{
		printf("Invalid params number:\n %s", CMD_DAC_CAL.usage1);
		return (FAIL);
	}
	return OK;
}

int doDacCalRst(int argc, char *argv[]);
const CliCmdType CMD_DAC_CAL_RST =
	{"daccalrst", 2, &doDacCalRst,
		"\tdaccalrst:	Reset calibration for one DAC channel\n",
		"\tUsage:		ioplus <stack> daccalrst <channel>\n", "",
		"\tExample:		ioplus 0 daccalrst 2; Reset calibration data on DAC channel #2 on Board #0 at factory default\n"};

int doDacCalRst(int argc, char *argv[])
{
	int ch = 0;

	int dev = 0;
	u8 buff[4] = {0, 0, 0, 0};
	u16 raw = 0;

	dev = doBoardInit(atoi(argv[1]));
	if (dev <= 0)
	{
		return (FAIL);
	}

	if (argc == 4)
	{
		ch = atoi(argv[3]);
		if ( (ch < CHANNEL_NR_MIN) || (ch > DAC_CH_NR_MAX))
		{
			printf("DAC channel out of range!\n");
			return (FAIL);
		}

		memcpy(buff, &raw, 2);
		buff[2] = ch + ADC_CH_NR_MAX;
		buff[3] = RESET_CALIBRATION_KEY;

		if (OK != i2cMem8Write(dev, I2C_MEM_CALIB_VALUE, buff, 4))
		{
			printf("Fail to write calibration data!\n");
			return (FAIL);
		}
		getCalStat(dev);
	}
	else
	{
		printf("Invalid params number:\n %s", CMD_DAC_CAL_RST.usage1);
		return (FAIL);
	}
	return OK;
}

int doWdtReload(int argc, char *argv[]);
const CliCmdType CMD_WDT_RELOAD =
	{"wdtr", 2, &doWdtReload,
		"\twdtr:		Reload the watchdog timer and enable the watchdog if is disabled\n",
		"\tUsage:		ioplus <stack> wdtr\n", "",
		"\tExample:		ioplus 0 wdtr; Reload the watchdog timer on Board #0 with the period \n"};

int doWdtReload(int argc, char *argv[])
{
	int dev = 0;
	u8 buff[2] = {0, 0};

	dev = doBoardInit(atoi(argv[1]));
	if (dev <= 0)
	{
		return (FAIL);
	}

	if (argc == 3)
	{
		buff[0] = WDT_RESET_SIGNATURE;
		if (OK != i2cMem8Write(dev, I2C_MEM_WDT_RESET_ADD, buff, 1))
		{
			printf("Fail to write watchdog reset key!\n");
			return (FAIL);
		}
	}
	else
	{
		printf("Invalid params number:\n %s", CMD_WDT_RELOAD.usage1);
		return (FAIL);
	}
	return OK;
}

int doWdtSetPeriod(int argc, char *argv[]);
const CliCmdType CMD_WDT_SET_PERIOD =
	{"wdtpwr", 2, &doWdtSetPeriod,
		"\twdtpwr:		Set the watchdog period in seconds, reload command must be issue in this interval to prevent Raspberry Pi power off\n",
		"\tUsage:		ioplus <stack> wdtpwr <val> \n", "",
		"\tExample:		ioplus 0 wdtpwr 10; Set the watchdog timer period on Board #0 at 10 seconds \n"};

int doWdtSetPeriod(int argc, char *argv[])
{
	int dev = 0;
	u16 period;
	u8 buff[2] = {0, 0};

	dev = doBoardInit(atoi(argv[1]));
	if (dev <= 0)
	{
		return (FAIL);
	}

	if (argc == 4)
	{
		period = (u16)atoi(argv[3]);
		if (0 == period)
		{
			printf("Invalid period!\n");
			return (FAIL);
		}
		memcpy(buff, &period, 2);
		if (OK != i2cMem8Write(dev, I2C_MEM_WDT_INTERVAL_SET_ADD, buff, 2))
		{
			printf("Fail to write watchdog period!\n");
			return (FAIL);
		}
	}
	else
	{
		printf("Invalid params number:\n %s", CMD_WDT_SET_PERIOD.usage1);
		return (FAIL);
	}
	return OK;
}

int doWdtGetPeriod(int argc, char *argv[]);
const CliCmdType CMD_WDT_GET_PERIOD =
	{"wdtprd", 2, &doWdtGetPeriod,
		"\twdtprd:		Get the watchdog period in seconds, reload command must be issue in this interval to prevent Raspberry Pi power off\n",
		"\tUsage:		ioplus <stack> wdtprd \n", "",
		"\tExample:		ioplus 0 wdtprd; Get the watchdog timer period on Board #0\n"};

int doWdtGetPeriod(int argc, char *argv[])
{
	int dev = 0;
	u16 period;

	dev = doBoardInit(atoi(argv[1]));
	if (dev <= 0)
	{
		return (FAIL);
	}

	if (argc == 3)
	{
		if (OK != i2cReadWordAS(dev, I2C_MEM_WDT_INTERVAL_GET_ADD, &period))
		{
			printf("Fail to read watchdog period!\n");
			return (FAIL);
		}
		printf("%d\n", (int)period);
	}
	else
	{
		printf("Invalid params number:\n %s", CMD_WDT_GET_PERIOD.usage1);
		return (FAIL);
	}
	return OK;
}

int doWdtSetInitPeriod(int argc, char *argv[]);
const CliCmdType CMD_WDT_SET_INIT_PERIOD =
	{"wdtipwr", 2, &doWdtSetInitPeriod,
		"\twdtipwr:	Set the watchdog initial period in seconds, This period is loaded after power cycle, giving Raspberry time to boot\n",
		"\tUsage:		ioplus <stack> wdtipwr <val> \n", "",
		"\tExample:		ioplus 0 wdtipwr 10; Set the watchdog timer initial period on Board #0 at 10 seconds \n"};

int doWdtSetInitPeriod(int argc, char *argv[])
{
	int dev = 0;
	u16 period;
	u8 buff[2] = {0, 0};

	dev = doBoardInit(atoi(argv[1]));
	if (dev <= 0)
	{
		return (FAIL);
	}

	if (argc == 4)
	{
		period = (u16)atoi(argv[3]);
		if (0 == period)
		{
			printf("Invalid period!\n");
			return (FAIL);
		}
		memcpy(buff, &period, 2);
		if (OK != i2cMem8Write(dev, I2C_MEM_WDT_INIT_INTERVAL_SET_ADD, buff, 2))
		{
			printf("Fail to write watchdog period!\n");
			return (FAIL);
		}
	}
	else
	{
		printf("Invalid params number:\n %s", CMD_WDT_SET_INIT_PERIOD.usage1);
		return (FAIL);
	}
	return OK;
}

int doWdtGetInitPeriod(int argc, char *argv[]);
const CliCmdType CMD_WDT_GET_INIT_PERIOD =
	{"wdtiprd", 2, &doWdtGetInitPeriod,
		"\twdtiprd:	Get the watchdog initial period in seconds. This period is loaded after power cycle, giving Raspberry time to boot\n",
		"\tUsage:		ioplus <stack> wdtiprd \n", "",
		"\tExample:		ioplus 0 wdtiprd; Get the watchdog timer initial period on Board #0\n"};

int doWdtGetInitPeriod(int argc, char *argv[])
{
	int dev = 0;
	u16 period;

	dev = doBoardInit(atoi(argv[1]));
	if (dev <= 0)
	{
		return (FAIL);
	}

	if (argc == 3)
	{
		if (OK != i2cReadWordAS(dev, I2C_MEM_WDT_INIT_INTERVAL_GET_ADD, &period))
		{
			printf("Fail to read watchdog period!\n");
			return (FAIL);
		}
		printf("%d\n", (int)period);
	}
	else
	{
		printf("Invalid params number:\n %s", CMD_WDT_GET_INIT_PERIOD.usage1);
		return (FAIL);
	}
	return OK;
}

int doWdtSetOffPeriod(int argc, char *argv[]);
const CliCmdType CMD_WDT_SET_OFF_PERIOD =
	{"wdtopwr", 2, &doWdtSetOffPeriod,
		"\twdtopwr:	Set the watchdog off period in seconds (max 48 days), This is the time that watchdog mantain Raspberry turned off \n",
		"\tUsage:		ioplus <stack> wdtopwr <val> \n", "",
		"\tExample:		ioplus 0 wdtopwr 10; Set the watchdog off interval on Board #0 at 10 seconds \n"};

int doWdtSetOffPeriod(int argc, char *argv[])
{
	int dev = 0;
	u32 period;
	u8 buff[4] = {0, 0, 0, 0};

	dev = doBoardInit(atoi(argv[1]));
	if (dev <= 0)
	{
		return (FAIL);
	}

	if (argc == 4)
	{
		period = (u32)atoi(argv[3]);
		if ( (0 == period) || (period > WDT_MAX_OFF_INTERVAL_S))
		{
			printf("Invalid period!\n");
			return (FAIL);
		}
		memcpy(buff, &period, 4);
		if (OK
			!= i2cMem8Write(dev, I2C_MEM_WDT_POWER_OFF_INTERVAL_SET_ADD, buff, 4))
		{
			printf("Fail to write watchdog period!\n");
			return (FAIL);
		}
	}
	else
	{
		printf("Invalid params number:\n %s", CMD_WDT_SET_OFF_PERIOD.usage1);
		return (FAIL);
	}
	return OK;
}

int doWdtGetOffPeriod(int argc, char *argv[]);
const CliCmdType CMD_WDT_GET_OFF_PERIOD =
	{"wdtoprd", 2, &doWdtGetOffPeriod,
		"\twdtoprd:	Get the watchdog off period in seconds (max 48 days), This is the time that watchdog mantain Raspberry turned off \n",
		"\tUsage:		ioplus <stack> wdtoprd \n", "",
		"\tExample:		ioplus 0 wdtoprd; Get the watchdog off period on Board #0\n"};

int doWdtGetOffPeriod(int argc, char *argv[])
{
	int dev = 0;
	u32 period;

	dev = doBoardInit(atoi(argv[1]));
	if (dev <= 0)
	{
		return (FAIL);
	}

	if (argc == 3)
	{
		if (OK
			!= i2cReadDWordAS(dev, I2C_MEM_WDT_POWER_OFF_INTERVAL_GET_ADD,
				&period))
		{
			printf("Fail to read watchdog period!\n");
			return (FAIL);
		}
		printf("%d\n", (int)period);
	}
	else
	{
		printf("Invalid params number:\n %s", CMD_WDT_GET_OFF_PERIOD.usage1);
		return (FAIL);
	}
	return OK;
}

int doLoopbackTest(int argc, char *argv[]);
const CliCmdType CMD_IO_TEST = {"iotest", 2, &doLoopbackTest,
	"\tiotest:		Test the ioplus with loopback card inserted \n",
	"\tUsage:		ioplus <stack> iotest\n",
	"\tUsage:		ioplus <stack> iotest <test type>\n",
	"\tExample:		ioplus 0 iotest; Run the tests \n"};

//*************************************************************************************

int pwmFreqGet(int dev, int *val)
{
	u16 raw = 0;

	if (OK != i2cReadWordAS(dev, I2C_MEM_OD_PWM_FREQUENCY, &raw))
	{
		printf("Fail to read!\n");
		return ERROR;
	}
	*val = raw;
	return OK;
}

int pwmFreqSet(int dev, int val)
{
	u8 buff[2] = {0, 0};
	u16 raw = 0;

	if (val < 10)
	{
		val = 10;
	}
	if (val > 65500)
	{
		val = 65500;
	}
	raw = (u16)val;
	memcpy(buff, &raw, 2);
	if (OK != i2cMem8Write(dev, I2C_MEM_OD_PWM_FREQUENCY, buff, 2))
	{
		printf("Fail to write!\n");
		return ERROR;
	}
	return OK;
}

int pwmChFreqSet(int dev, int ch, int val)
{
	u8 buff[2] = {0, 0};
	u16 raw = 0;

	if (val < 10)
	{
		val = 10;
	}
	if (val > 65500)
	{
		val = 65500;
	}
	raw = (u16)val;
	memcpy(buff, &raw, 2);
	if (OK
		!= i2cMem8Write(dev, I2C_MEM_OD_PWM_FREQUENCY_CH1 + (ch - 1) * 2, buff,
			2))
	{
		printf("Fail to write!\n");
		return ERROR;
	}
	return OK;
}

int doPwmFreqRead(int argc, char *argv[]);
const CliCmdType CMD_PWM_FREQ_READ =
	{"pwmfrd", 2, &doPwmFreqRead,
		"\tpwmfrd:		Read open-drain pwm frequency in Hz \n",
		"\tUsage:		ioplus <stack> pwmfrd\n", "",
		"\tExample:		ioplus 0 pwmfrd; Read the pwm frequency for all open drain output channels\n"};

int doPwmFreqRead(int argc, char *argv[])
{
	int val = 0;
	int dev = 0;

	dev = doBoardInit(atoi(argv[1]));
	if (dev <= 0)
	{
		return (FAIL);
	}
	if (gHwVer < 3)
	{
		printf(
			"This feature is available on hardware versions greater or equal to 3.0!\n");
		return (FAIL);
	}
	if (argc == 3)
	{

		if (OK != pwmFreqGet(dev, &val))
		{
			printf("Fail to read!\n");
			return (FAIL);
		}

		printf("%d Hz\n", val);
	}
	else
	{
		printf("Invalid params number:\n %s", CMD_PWM_FREQ_READ.usage1);
		return (FAIL);
	}
	return OK;
}

int doPwmFreqWrite(int argc, char *argv[]);
const CliCmdType CMD_PWM_FREQ_WRITE =
	{"pwmfwr", 2, &doPwmFreqWrite,
		"\tpwmfwr:		Write open dran output pwm frequency in Hz [10..64000]\n",
		"\tUsage:		ioplus <stack> pwmfwr <value>\n",
		"\tUsage:		ioplus <stack> pwmfwr <channel> <value>\n",
		"\tExample:		ioplus 0 dacwr 200; Set the open-drain output pwm frequency to 200Hz \n"};

int doPwmFreqWrite(int argc, char *argv[])
{
	int dev = 0;
	int val = 0;
	int channel = 0;

	dev = doBoardInit(atoi(argv[1]));
	if (dev <= 0)
	{
		return (FAIL);
	}
	if (gHwVer < 3)
	{
		printf(
			"This feature is available on hardware versions greater or equal to 3.0!\n");
		return (FAIL);
	}
	if (argc == 4)
	{
		val = atof(argv[3]);
		if (val < 10 || val > 65500)
		{
			printf("Invalid pwm frequency value, must be 10..65000 \n");
			return (FAIL);
		}

		if (OK != pwmFreqSet(dev, val))
		{
			printf("Fail to write!\n");
			return (FAIL);
		}
		printf("done\n");
	}
	else if (argc == 5)
	{
		channel = atoi(argv[3]);
		if (channel < 1 || channel > 4)
		{
			printf("Invalid channel number, must be 1..4 \n");
			return (FAIL);
		}
		val = atof(argv[4]);
		if (val < 10 || val > 65500)
		{
			printf("Invalid pwm frequency value, must be 10..65000 \n");
			return (FAIL);
		}

		if (OK != pwmChFreqSet(dev, channel, val))
		{
			printf("Fail to write!\n");
			return (FAIL);
		}
		printf("done\n");
	}
	else
	{
		printf("Invalid params number:\n %s", CMD_PWM_FREQ_WRITE.usage1);
		return (FAIL);
	}
	return OK;
}
#define MAX_ACC 60000
#define MAX_SPEED 60000
#define MIN_SPEED 10

int odOutMoveSet(int dev, int ch, int acc, int dec, int minSpd, int maxSpd)
{
	uint8_t buff[8];
	uint16_t aux16 = 0;

	if (ch <= 0 || ch > 4)
	{
		printf("invalid Channel number [1..4]\n");
		return -1;
	}
	if (acc < 0 || acc > MAX_ACC)
	{
		printf("Invalid acceleration value\n");
		return -1;
	}
	if (dec < 0 || dec > MAX_ACC)
	{
		printf("Invalid deceleration value\n");
		return -1;
	}
	if (maxSpd < MIN_SPEED || maxSpd > MAX_SPEED)
	{
		printf("Invalid speed [10..60000]\n");
	}

	if (minSpd < MIN_SPEED || minSpd > maxSpd)
	{
		printf("Invalid speed [10..60000]\n");
	}

	aux16 = (u16)acc;
	memcpy(buff, &aux16, sizeof(uint16_t));
	aux16 = (u16)dec;
	memcpy(buff + 2, &aux16, sizeof(uint16_t));
	aux16 = (u16)maxSpd;
	memcpy(buff + 4, &aux16, sizeof(uint16_t));
	aux16 = (u16)minSpd;
	memcpy(buff + 6, &aux16, sizeof(uint16_t));
	if (OK != i2cMem8Write(dev, I2C_MEM_ODP_ACC, buff, 8))
	{
		printf("Fail to write\n");
		return -1;
	}
	buff[0] = (uint8_t)ch;
	if (OK != i2cMem8Write(dev, I2C_MEM_ODP_CMD, buff, 1))
	{
		printf("Fail to write\n");
		return -1;
	}
	return OK;
}

int doMoveParWrite(int argc, char *argv[]);
const CliCmdType CMD_MV_P_WRITE =
	{"mvpwr", 2, &doMoveParWrite,
		"\tmvpwr:		Write open drain output movement profile parameters\n",
		"\tUsage:		ioplus <stack> mvpwr <channel> <acc> <dec> <min_speed> <max_speed>\n",
		"",
		"\tExample:		ioplus 0 mvpwr 1 1000 500 1000 20000; Set the open-drain output profile parameters \n"};

int doMoveParWrite(int argc, char *argv[])
{
	int dev = -1;
	int channel = 0;
	int acc = 0;
	int dec = 0;
	int maxSpd = 0;
	int minSpd = 0;

	dev = doBoardInit(atoi(argv[1]));
		if (dev <= 0)
		{
			return (FAIL);
		}
		if (gHwVer < 3)
		{
			printf(
				"This feature is available on hardware versions greater or equal to 3.0!\n");
			return (FAIL);
		}

	if(argc != 8)
	{
		printf("Invalid argument number %s", CMD_MV_P_WRITE.usage1);
		return(FAIL);
	}
	channel = atoi(argv[3]);
	acc = atoi(argv[4]);
	dec = atoi(argv[5]);
	minSpd = atoi(argv[6]);
	maxSpd = atoi(argv[7]);

	return odOutMoveSet(dev, channel, acc, dec, minSpd, maxSpd);
}

//***************************************************MIN/MAX sample count read write**********************************************
int minMaxSamplesGet(int dev, int *val)
{
	u8 raw = 0;

	if (OK != i2cReadByteAS(dev, I2C_MEM_MIN_MAX_SAMPLES, &raw))
	{
		printf("Fail to read!\n");
		return ERROR;
	}
	*val = raw;
	return OK;
}

int minMaxSamplesSet(int dev, int val)
{
	u8 buff[2] = {0, 0};

	if (val < 5)
	{
		val = 10;
	}
	if (val > 250)
	{
		val = 250;
	}
	buff[0] = (u8)val;
	if (OK != i2cMem8Write(dev, I2C_MEM_MIN_MAX_SAMPLES, buff, 1))
	{
		printf("Fail to write!\n");
		return ERROR;
	}
	return OK;
}

int doMinMaxSamplesRead(int argc, char *argv[]);
const CliCmdType CMD_MIN_MAX_SAMPLE_READ =
	{"mmsrd", 2, &doMinMaxSamplesRead,
		"\tmmsrd:		Read the number of the samples used for min / max search in analog inputs\n",
		"\tUsage:		ioplus <stack> mmsrd\n", "",
		"\tExample:		ioplus 0 mmsrd; Read the niumber of samples used in min/max search\n"};

int doMinMaxSamplesRead(int argc, char *argv[])
{
	int val = 0;
	int dev = 0;

	dev = doBoardInit(atoi(argv[1]));
	if (dev <= 0)
	{
		return (FAIL);
	}
	if (gHwVer < 3)
	{
		printf(
			"This feature is available on hardware versions greater or equal to 3.0!\n");
		return (FAIL);
	}
	if (argc == 3)
	{

		if (OK != minMaxSamplesGet(dev, &val))
		{
			printf("Fail to read!\n");
			return (FAIL);
		}

		printf("%d\n", val);
	}
	else
	{
		printf("Invalid params number:\n %s", CMD_MIN_MAX_SAMPLE_READ.usage1);
		return (FAIL);
	}
	return OK;
}

int minMaxSamplesWrite(int argc, char *argv[]);
const CliCmdType CMD_MIN_MAX_SAMPLE_WRITE =
	{"mmswr", 2, &minMaxSamplesWrite,
		"\tmmswr:		Write the number of the samples used for min / max search in analog inputs[5..250]\n",
		"\tUsage:		ioplus <stack> mmswr <value>\n", "",
		"\tExample:		ioplus 0 mmswr 200; Set the number at 200 samples \n"};

int minMaxSamplesWrite(int argc, char *argv[])
{
	int dev = 0;
	int val = 0;

	dev = doBoardInit(atoi(argv[1]));
	if (dev <= 0)
	{
		return (FAIL);
	}
	if (gHwVer < 3)
	{
		printf(
			"This feature is available on hardware versions greater or equal to 3.0!\n");
		return (FAIL);
	}
	if (argc == 4)
	{
		val = atof(argv[3]);
		if (val < 5 || val > 250)
		{
			printf("Invalid number of samples, must be 5..250 \n");
			return (FAIL);
		}

		if (OK != minMaxSamplesSet(dev, val))
		{
			printf("Fail to write!\n");
			return (FAIL);
		}
		printf("done\n");
	}
	else
	{
		printf("Invalid params number:\n %s", CMD_MIN_MAX_SAMPLE_WRITE.usage1);
		return (FAIL);
	}
	return OK;
}

//******************************************** One Wire Bus *************************************************
int doOwbGet(int argc, char *argv[]);
const CliCmdType CMD_OWB_RD =
	{"owbtrd", 2, &doOwbGet,
		"\towbtrd		Display the temperature readed from a one wire bus connected sensor\n",
		"\tUsage:		ioplus <stack> owbtrd <sensor (1..10)>\n", "",
		"\tExample:		ioplus 0 owbtrd 1 Display the temperature of the sensor #1\n"};

int doOwbGet(int argc, char *argv[])
{
	int dev = -1;
	u8 buff[5];
	int resp = 0;
	int channel = 0;
	float temp = 0;
	s16 saux16 = 0;
	int retry = 3;

	if (argc != 4)
	{
		return ARG_CNT_ERR;
	}
	channel = atoi(argv[3]);
	if (channel < 1 || channel > 16)
	{
		return ERROR;
	}
	dev = doBoardInit(atoi(argv[1]));
	if (dev <= 0)
	{
		return ERROR;
	}
	resp = i2cMem8Read(dev, I2C_MEM_1WB_DEV, buff, 1);
	if (FAIL == resp)
	{
		printf("Fail to read one wire bus info!\n");
		return ERROR;
	}
	if (channel > buff[0])
	{
		printf("Invalid channel number, only %d sensors connected!\n", buff[0]);
		return ERROR;
	}
	saux16 = -1;
	retry = 4;
	while ( (saux16 == -1) && (retry > 0))
	{
		resp = i2cMem8Read(dev, I2C_MEM_1WB_T1 + (channel - 1) * OWB_TEMP_SIZE_B,
			buff, OWB_TEMP_SIZE_B);
		if (FAIL == resp)
		{
			printf("Fail to read one wire bus info!\n");
			return ERROR;
		}
		memcpy(&saux16, &buff[0], 2);
		retry--;
	}
	if (saux16 == -1)
	{
		return ERROR;
	}
	temp = (float)saux16 / 100;

	printf("%0.2f C\n", temp);
	return OK;
}

int doOwbIdGet(int argc, char *argv[]);
const CliCmdType CMD_OWB_ID_RD =
	{"owbidrd", 2, &doOwbIdGet,
		"\towbidrd		Display the 64bits ROM ID of the one wire bus connected sensor\n",
		"\tUsage:		ioplus <stack> owbidrd <sensor (1..10)>\n", "",
		"\tExample:		ioplus 0 owbidrd 1 Display the ROM ID of the sensor #1\n"};

int doOwbIdGet(int argc, char *argv[])
{
	int dev = -1;
	u8 buff[8];
	int resp = 0;
	int channel = 0;
	uint64_t romID = 0;

	if (argc != 4)
	{
		return ARG_CNT_ERR;
	}
	channel = atoi(argv[3]);
	if (channel < 1 || channel > 16)
	{
		return ERROR;
	}
	dev = doBoardInit(atoi(argv[1]));
	if (dev <= 0)
	{
		return ERROR;
	}
	buff[0] = 0xff & (channel - 1);
	resp = i2cMem8Write(dev, I2C_MEM_1WB_ROM_CODE_IDX, buff, 1); //Select sensor ID to read
	if (FAIL == resp)
	{
		printf("Fail to read one wire bus info!\n");
		return ERROR;
	}
	resp = i2cMem8Read(dev, I2C_MEM_1WB_DEV, buff, 1); //check the number of connected sensors
	if (FAIL == resp)
	{
		printf("Fail to read one wire bus info!\n");
		return ERROR;
	}
	if (channel > buff[0])
	{
		printf("Invalid channel number, only %d sensors connected!\n", buff[0]);
		return ERROR;
	}

	resp = i2cMem8Read(dev, I2C_MEM_1WB_ROM_CODE, buff, 8);
	if (FAIL == resp)
	{
		printf("Fail to read one wire bus info!\n");
		return ERROR;
	}

	memcpy(&romID, &buff[0], 8);

	printf("0x%lx\n", romID);
	return OK;
}

int doOwbSensCountRead(int argc, char *argv[]);
const CliCmdType CMD_OWB_SNS_CNT_RD = {"owbcntrd", 2, &doOwbSensCountRead,
	"\towbcntrd		Display the number of One Wire Bus connected sensors\n",
	"\tUsage:		ioplus <stack> owbcntrd\n", "",
	"\tExample:		ioplus 0 owbcntrd  Display the number of sensors connected\n"};

int doOwbSensCountRead(int argc, char *argv[])
{
	int dev = -1;
	u8 buff[2];
	int resp = 0;

	if (argc != 3)
	{
		return ARG_CNT_ERR;
	}
	dev = doBoardInit(atoi(argv[1]));
	if (dev <= 0)
	{
		return ERROR;
	}
	resp = i2cMem8Read(dev, I2C_MEM_1WB_DEV, buff, 1);
	if (FAIL == resp)
	{
		printf("Fail to read!\n");
		return ERROR;
	}

	printf("%d\n", buff[0]);
	return OK;
}

int doOwbScan(int argc, char *argv[]);
const CliCmdType CMD_OWB_SCAN = {"owbscan", 2, &doOwbScan,
	"\towbscan		Start One Wire Bus scaning procedure\n",
	"\tUsage:		ioplus <stack> owbscan\n", "",
	"\tExample:		ioplus 0 owbscan  Start One Wire Bus scaning procedure\n"};

int doOwbScan(int argc, char *argv[])
{
	int dev = -1;
	u8 buff[2];
	int resp = 0;

	if (argc != 3)
	{
		return ARG_CNT_ERR;
	}
	dev = doBoardInit(atoi(argv[1]));
	if (dev <= 0)
	{
		return ERROR;
	}
	buff[0] = 0xaa;
	resp = i2cMem8Write(dev, I2C_MEM_1WB_START_SEARCH, buff, 1);
	if (FAIL == resp)
	{
		printf("Fail to write!\n");
		return ERROR;
	}

	printf("OK\n");
	return OK;
}

const CliCmdType *gCmdArray[] = {&CMD_VERSION, &CMD_HELP, &CMD_WAR, &CMD_PINOUT,
	&CMD_LIST, &CMD_BOARD,
#ifdef HW_DEBUG
	&CMD_ERR,
#endif
	&CMD_RELAY_WRITE, &CMD_RELAY_READ, &CMD_TEST, &CMD_GPIO_WRITE,
	&CMD_GPIO_READ, &CMD_GPIO_DIR_WRITE, &CMD_GPIO_DIR_READ,
	&CMD_GPIO_EDGE_WRITE, &CMD_GPIO_EDGE_READ, &CMD_GPIO_CNT_READ,
	&CMD_GPIO_CNT_RESET, &CMD_GPIO_ENC_CNT_READ, &CMD_GPIO_ENC_CNT_RESET,
	&CMD_OPTO_READ, &CMD_OPTO_EDGE_READ, &CMD_OPTO_EDGE_WRITE,
	&CMD_OPTO_CNT_READ, &CMD_OPTO_CNT_RESET, &CMD_OPTO_ENC_WRITE,
	&CMD_OPTO_ENC_READ, &CMD_OPTO_ENC_CNT_READ, &CMD_OPTO_ENC_CNT_RESET,
	&CMD_OD_READ, &CMD_OD_WRITE, &CMD_OD_CNT_READ, &CMD_OD_CNT_WRITE,
	&CMD_OD_CNT_RST, &CMD_DAC_READ, &CMD_DAC_WRITE, &CMD_ADC_READ,
	&CMD_ADC_READ_MAX, &CMD_ADC_READ_MIN, &CMD_MIN_MAX_SAMPLE_WRITE,
	&CMD_MIN_MAX_SAMPLE_READ, &CMD_ADC_CAL, &CMD_ADC_CAL_RST, &CMD_DAC_CAL,
	&CMD_DAC_CAL_RST, &CMD_WDT_RELOAD, &CMD_WDT_SET_PERIOD, &CMD_WDT_GET_PERIOD,
	&CMD_WDT_SET_INIT_PERIOD, &CMD_WDT_GET_INIT_PERIOD, &CMD_WDT_SET_OFF_PERIOD,
	&CMD_WDT_GET_OFF_PERIOD, &CMD_IO_TEST, &CMD_PWM_FREQ_READ,
	&CMD_PWM_FREQ_WRITE, &CMD_OWB_RD, &CMD_OWB_ID_RD, &CMD_OWB_SNS_CNT_RD,
	&CMD_OWB_SCAN, &CMD_OPTO_OD_CMD_SET,
#ifdef MOVE_PROFILE
	&CMD_MV_P_WRITE,
#endif
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
#ifdef THREAD_SAFE
	sem_t *semaphore = sem_open("/SMI2C_SEM", O_CREAT);
	int semVal = 2;
	sem_wait(semaphore);
#endif
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
#ifdef THREAD_SAFE
				sem_getvalue(semaphore, &semVal);
				if (semVal < 1)
				{
					sem_post(semaphore);
				}
#endif
				return ret;
			}
		}
		i++;
	}
	printf("Invalid command option\n");
	usage();
#ifdef THREAD_SAFE
	sem_getvalue(semaphore, &semVal);
	if (semVal < 1)
	{
		sem_post(semaphore);
	}
#endif
	return -1;
}
