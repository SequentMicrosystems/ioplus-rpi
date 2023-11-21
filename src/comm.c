/*
 * comm.c:
 *	Communication routines "platform specific" for Raspberry Pi
 *	
 *	Copyright (c) 2016-2020 Sequent Microsystem
 *	<http://www.sequentmicrosystem.com>
 ***********************************************************************
 *	Author: Alexandru Burcea
 ***********************************************************************
 */
#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <linux/i2c-dev.h>
#include "comm.h"

#define I2C_SLAVE	0x0703
#define I2C_SMBUS	0x0720	/* SMBus-level access */

#define I2C_SMBUS_READ	1
#define I2C_SMBUS_WRITE	0

// SMBus transaction types

#define I2C_SMBUS_QUICK		    0
#define I2C_SMBUS_BYTE		    1
#define I2C_SMBUS_BYTE_DATA	    2
#define I2C_SMBUS_WORD_DATA	    3
#define I2C_SMBUS_PROC_CALL	    4
#define I2C_SMBUS_BLOCK_DATA	    5
#define I2C_SMBUS_I2C_BLOCK_BROKEN  6
#define I2C_SMBUS_BLOCK_PROC_CALL   7		/* SMBus 2.0 */
#define I2C_SMBUS_I2C_BLOCK_DATA    8

// SMBus messages

#define I2C_SMBUS_BLOCK_MAX	512	/* As specified in SMBus standard */
#define I2C_SMBUS_I2C_BLOCK_MAX	512	/* Not specified but we use same structure */

int i2cSetup(int addr)
{
	int file;
	char filename[40];
	sprintf(filename, "/dev/i2c-1");

	if ( (file = open(filename, O_RDWR)) < 0)
	{
		printf("Failed to open the bus.");
		return -1;
	}
	if (ioctl(file, I2C_SLAVE, addr) < 0)
	{
		printf("Failed to acquire bus access and/or talk to slave.\n");
		return -1;
	}

	return file;
}

int i2cMem8Read(int dev, int add, uint8_t* buff, int size)
{
	uint8_t intBuff[I2C_SMBUS_BLOCK_MAX];

	if (NULL == buff)
	{
		return -1;
	}

	if (size > I2C_SMBUS_BLOCK_MAX)
	{
		return -1;
	}

	intBuff[0] = 0xff & add;

	if (write(dev, intBuff, 1) != 1)
	{
		//printf("Fail to select mem add!\n");
		return -1;
	}
	if (read(dev, buff, size) != size)
	{
		//printf("Fail to read memory!\n");
		return -1;
	}
	return 0; //OK
}

int i2cMem8Write(int dev, int add, uint8_t* buff, int size)
{
	uint8_t intBuff[I2C_SMBUS_BLOCK_MAX];

	if (NULL == buff)
	{
		return -1;
	}

	if (size > I2C_SMBUS_BLOCK_MAX - 1)
	{
		return -1;
	}

	intBuff[0] = 0xff & add;
	memcpy(&intBuff[1], buff, size);

	if (write(dev, intBuff, size + 1) != size + 1)
	{
		//printf("Fail to write memory!\n");
		return -1;
	}
	return 0;
}
#define SPURIOUS_RETRY	10 
int i2cReadByteAS(int dev, int add, uint8_t* val)
{
	uint8_t read = 255;
	int valA = 256;
	int retry = SPURIOUS_RETRY;

	while ( (read != valA) && (retry > 0))
	{
		retry--;
		valA = read;
		if (0 != i2cMem8Read(dev, add, &read, 1))
		{
			return -1;
		}
	}
	if (retry == 0)
	{
		return -1;
	}
	*val = read;
	return 0;
}

int i2cReadWordAS(int dev, int add, uint16_t* val)
{
	uint8_t buff[2];
	uint16_t read = 50000;
	int valA = 60000;
	int retry = SPURIOUS_RETRY;
	
	while ( ((read & 0xfffc) != (valA & 0xfffc)) && (retry > 0))
	{
		retry--;
		valA = read;
		if (0 != i2cMem8Read(dev, add, buff, 2))
		{
			return -1;
		}
		memcpy(&read, buff, 2);
	}
	if (retry == 0)
	{
		return -1;
	}
	*val = read;
	return 0;
}


int i2cReadDWordAS(int dev, int add, uint32_t* val)
{
	uint8_t buff[4];
	uint32_t read = 50000;
	uint32_t valA = 60000;
	int retry = SPURIOUS_RETRY;
	
	while ( ((read & 0xfffffffc) != (valA & 0xfffffffc)) && (retry > 0))
	{
		retry--;
		valA = read;
		if (0 != i2cMem8Read(dev, add, buff, 4))
		{
			return -1;
		}
		memcpy(&read, buff, 4);
	}
	if (retry == 0)
	{
		return -1;
	}
	*val = read;
	return 0;
}

int i2cReadDWord(int dev, int add, uint32_t* val)
{
	uint8_t buff[4];
	uint32_t read = 50000;

	if (0 != i2cMem8Read(dev, add, buff, 4))
	{
		return -1;
	}
	memcpy(&read, buff, 4);
	*val = read;
	return 0;
}


int i2cReadIntAS(int dev, int add, int* val)
{
	uint8_t buff[4];
	int read = 50000;
	int valA = 60000;
	int retry = SPURIOUS_RETRY;
	
	while ( ((read & 0xfffffffc) != (valA & 0xfffffffc)) && (retry > 0))
	{
		retry--;
		valA = read;
		if (0 != i2cMem8Read(dev, add, buff, 4))
		{
			return -1;
		}
		memcpy(&read, buff, 4);
	}
	if (retry == 0)
	{
		return -1;
	}
	*val = read;
	return 0;
}
