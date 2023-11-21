#ifndef COMM_H_
#define COMM_H_

#include <stdint.h>

int i2cSetup(int addr);
int i2cMem8Read(int dev, int add, uint8_t* buff, int size);
int i2cMem8Write(int dev, int add, uint8_t* buff, int size);
int i2cReadByteAS(int dev, int add, uint8_t* val);
int i2cReadWordAS(int dev, int add, uint16_t* val);
int i2cReadDWord(int dev, int add, uint32_t* val);
int i2cReadDWordAS(int dev, int add, uint32_t* val);
int i2cReadIntAS(int dev, int add, int* val);
#endif //COMM_H_
