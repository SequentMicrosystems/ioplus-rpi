#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <math.h>

#include "comm.h"
#include "ioplus.h"
#include "thread.h"

//#define TEST_VERB

typedef struct {
	char* name;
	u8 wrAdd;
	u8 wrSize;
	int wrVal;
	u8 rdAdd;
	u8 rdSize;
	int rdMask;
	int minVal;
	int maxVal;	
}loopTestType;

const loopTestType leftTest[]={
	{"RELAY1-4 to OPTO 1-4", 0, 1, 0, 3, 1, 15, 0, 0},
	{"GPIO direction", 7, 1, 15, 4, 1, 15, 11, 11},
	{"Relay 1 to OPTO 1", 0, 1, 1, 3, 1, 15, 1, 1},
	{"Relay 2 to OPTO 2", 0, 1, 2, 3, 1, 15, 2, 2},
	{"Relay 3 to OPTO 3", 0, 1, 4, 3, 1, 15, 4, 4},
	{"Relay 4 to OPTO 4", 0, 1, 8, 3, 1, 15, 8, 8},
	{"Relay 1 to GPIO 1", 0, 1, 14, 4, 1, 15, 1, 1},
	{"Relay 2 to GPIO 2", 0, 1, 13, 4, 1, 15, 2, 2},
	{"Relay 4 to GPIO 4", 0, 1, 7, 4, 1, 15, 8, 8},
	{"", 0, 1, 0, 3, 1, 240, 0, 0},
	{"DONE", 0, 0, 0, 0, 0, 0, 0, 0},
};

const loopTestType rightTest[]={
	{"Relay 5 to OPTO 5", 0, 1, 16, 3, 1, 240, 16, 16},
	{"Relay 6 to OPTO 6", 0, 1, 32, 3, 1, 240, 32, 32},
	{"Relay 7 to OPTO 7", 0, 1, 64, 3, 1, 240, 64, 64},
	{"Relay 8 to OPTO 8", 0, 1, 128, 3, 1, 240, 128, 128},
	{"", 0, 1, 0, 3, 1, 240, 0, 0},
	{"OD 4 to OPTO 5", 54, 2, 10000, 3, 1, 16, 16, 16},
	{"", 54, 2, 0, 3, 1, 240, 0, 0},
	{"OD 3 to OPTO 6", 52, 2, 10000, 3, 1, 32, 32, 32},
	{"", 52, 2, 0, 3, 1, 240, 0, 0},
	{"OD 2 to OPTO 7", 50, 2, 10000, 3, 1, 64, 64, 64},
	{"", 50, 2, 0, 3, 1, 240, 0, 0},
	{"OD 1 to OPTO 8", 48, 2, 10000, 3, 1, 128, 128, 128},
	{"", 48, 2, 0, 3, 1, 240, 0, 0},	
	{"DONE", 0, 0, 0, 0, 0, 0, 0, 0},
};

const loopTestType bottomTest[]={
	{"DAC 1 to ADC 4 @2V", 40, 2, 2000, 30, 2, 0xffff, 1800, 2100},
	{"DAC 1 to ADC 4 @0V", 40, 2, 0, 30, 2, 0xffff, 0, 700},
	{"DAC 2 to ADC 3 @2V", 42, 2, 2000, 28, 2, 0xffff, 1800, 2100},
	{"DAC 2 to ADC 3 @0V", 42, 2, 0, 28, 2, 0xffff, 0, 700},
	{"DAC 3 to ADC 2 @2V", 44, 2, 2000, 26, 2, 0xffff, 1800, 2100},
	{"DAC 3 to ADC 2 @0V", 44, 2, 0, 26, 2, 0xffff, 0, 700},
	{"DAC 4 to ADC 1 @2V", 46, 2, 2000, 24, 2, 0xffff, 1800, 2100},
	{"DAC 4 to ADC 1 @0V", 46, 2, 0, 24, 2, 0xffff, 0, 700},
	{"DAC 1 to ADC 8 @2V", 40, 2, 2000, 38, 2, 0xffff, 1800, 2100},
	{"DAC 1 to ADC 8 @0V", 40, 2, 0, 38, 2, 0xffff, 0, 700},
	{"DAC 2 to ADC 7 @2V", 42, 2, 2000, 36, 2, 0xffff, 1800, 2100},
	{"DAC 2 to ADC 7 @0V", 42, 2, 0, 36, 2, 0xffff, 0, 700},
	{"DAC 3 to ADC 6 @2V", 44, 2, 2000, 34, 2, 0xffff, 1800, 2100},
	{"DAC 3 to ADC 6 @0V", 44, 2, 0, 34, 2, 0xffff, 0, 700},
	{"DAC 4 to ADC 5 @2V", 46, 2, 2000, 32, 2, 0xffff, 1800, 2100},
	{"DAC 4 to ADC 5 @0V", 46, 2, 0, 32, 2, 0xffff, 0, 700},
	{"DONE", 0, 0, 0, 0, 0, 0, 0, 0},
};


int performV3Test(int dev, loopTestType test, u8 silent)
{
	u8 buff[2];
	int ret = 0;
	u16 val16 = 0;
	
	if(strlen(test.name) > 1 && (0 == silent))
	{
		printf("%s...", test.name);
	}
#ifdef TEST_VERB	
	else
	{
		printf("\n#start test %s\n", test.name);
	}
#endif	
	busyWait(100);
	switch(test.wrSize)
	{
	case 1:
		buff[0] = 0xff & test.wrVal;
		ret = i2cMem8Write(dev, test.wrAdd, buff, 1);
		break;
	case 2:
		val16 = (u16)test.wrVal;
		memcpy(buff, &val16, 2);
		ret = i2cMem8Write(dev, test.wrAdd, buff, 2);
		break;
	default://no write needed
		break;
	}
	if (ret != OK)
	{
		if (strlen(test.name) > 1 && (0 == silent))
		{
			printf("FAIL!\n");
		}
#ifdef TEST_VERB
			printf("Test fail to write!\n");
#endif
			return ret;
	}
	busyWait(150);
	val16 = 0;
	switch(test.rdSize)
	{
	case 1:
		ret = i2cMem8Read(dev, test.rdAdd, buff, 1);
		val16 = buff[0];
		
		break;
	case 2:
		ret = i2cMem8Read(dev, test.rdAdd, buff, 2);
		memcpy(&val16, buff, 2);
		break;
	default:

		break;
	}
	val16 = (u16)(val16 & test.rdMask);
	
#ifdef TEST_VERB
	printf("read %d...", (int)val16);	
#endif 
	if (ret != OK)
		{
			if((strlen(test.name) > 1) && (0 == silent))
			{
				printf("FAIL!\n");
			}
#ifdef TEST_VERB
			printf("Test fail to read!\n");
#endif
			return ret;
		}	
		
		if((val16 >= test.minVal) && (val16 <= test.maxVal))
		{
			if((strlen(test.name) > 1) && (0 == silent))
			{
				printf("PASS\n");
			}
			return OK;
		}
		else
		{
			if((strlen(test.name) > 1) && (0 == silent))
			{
				printf("FAIL!\n");
			}
			return ERROR;
		}

	return ret;
}

int doV3tests(int dev, int type)
{
	int i=0;
	int failCount = 0;
	
	switch(type)
	{
		case 1:
			while(strcmp("DONE",leftTest[i].name) != 0)
			{
				if(OK != performV3Test(dev, leftTest[i], 0))
				{				
					failCount ++;
				}
				i++;
			}
			break;
		case 2:
			while(strcmp("DONE",bottomTest[i].name) != 0)
			{
				if(OK != performV3Test(dev, bottomTest[i], 0))
				{
					failCount ++;
				}
				i++;
			}
			break;

		case 3:
			while(strcmp("DONE",rightTest[i].name) != 0)
			{
				if(OK != performV3Test(dev, rightTest[i], 0))
				{
					failCount ++;
				}
				i++;
			}
			break;
			
		default:
			if((OK == performV3Test(dev, leftTest[2], 1)) && (OK == performV3Test(dev, leftTest[0], 1)))
			{
				printf("Cable detected in position 1\n");
				return doV3tests(dev, 1);
			}
			else if((OK == performV3Test(dev, bottomTest[0], 1)) && (OK == performV3Test(dev, bottomTest[1], 1)))
			{
				printf("Cable detected in position 2\n");
				performV3Test(dev, leftTest[9], 1);//close the relays
				return doV3tests(dev, 2);
			}
			else if((OK == performV3Test(dev, rightTest[0], 1)) && (OK == performV3Test(dev, rightTest[4], 1)))
			{
				printf("Cable detected in position 3\n");
				return doV3tests(dev, 3);
			}
			else
			{
				printf("No loopback cable detected, plase specify the test type\n");
				performV3Test(dev, leftTest[9], 1);//close the relays
				return ERROR;
			}
			
			break;
					
	}
	return failCount;
}

int doV2Tests(int dev)
{
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
	//GPIO -> OPTO
//Opto ON
	for (i = 0; i < 4; i++)
	{
		if (OK != gpioChDirSet(dev, i + 1, 0))
		{
			printf("Fail to set GPIO direction!\n");
			return -1;
		}
		if (OK != gpioChSet(dev, i + 1, 0))
		{
			printf("Fail to set GPIO !\n");
			return -1;
		}
		busyWait(50);
		if (OK != optoChGet(dev, t1OptoCh[i], &state))
		{
			printf("Fail to read opto!\n");
			return -1;
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
			return -1;
		}

		busyWait(50);
		if (OK != optoChGet(dev, t1OptoCh[i], &state))
		{
			printf("Fail to read opto!\n");
			return -1;
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
			return -1;
		}
		busyWait(250);
		if (OK != optoChGet(dev, t2OptoCh[i], &state))
		{
			printf("Fail to read opto!\n");
			return -1;
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
			return -1;
		}
		busyWait(250);
		if (OK != optoChGet(dev, t2OptoCh[i], &state))
		{
			printf("Fail to read opto!\n");
			return -1;
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
			return -1;
		}
		busyWait(250);
		if (OK != adcGet(dev, adcCh1[i], &val))
		{
			printf("Fail to read adc\n");
			return -1;
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
			return -1;
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
			return -1;
		}
		busyWait(250);
		if (OK != adcGet(dev, adcCh1[i], &val))
		{
			printf("Fail to read adc\n");
			return -1;
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
			return -1; //exit(1);
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
	return OK;
}

int doLoopbackTest(int argc, char *argv[])
{
	int dev = 0;
	int testType = 0;

	dev = doBoardInit(atoi(argv[1]));
	if (dev <= 0)
	{
		return(-1);
	}
	
	if (argc == 3 || argc == 4)
	{
		if(getHwVer() < 3)
		{
			if(OK != doV2Tests(dev))
			{
				return(-1);
			}
		}
		else
		{
			if(argc == 4)
			{
				testType = atoi(argv[3]);	
			}
			if(OK != doV3tests(dev, testType))
			{
				printf("\n TEST FAIL! \n");
				return(-1);
			}
		}
	}
	else
	{
		return ARG_CNT_ERR;
	}
	return OK;
}
