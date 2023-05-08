#ifndef IOPLUS_H_
#define IOPLUS_H_

#include <stdint.h>

#define ADC_CH_NO	8
#define DAC_CH_NO	4
#define ADC_RAW_VAL_SIZE	2
#define DAC_MV_VAL_SIZE		2
#define VOLT_TO_MILIVOLT	1000
#define OPTO_CH_NO 8
#define GPIO_CH_NO 4
#define COUNTER_SIZE 4

#define RETRY_TIMES	10
#define CALIBRATION_KEY 0xaa
#define RESET_CALIBRATION_KEY	0x55 
#define WDT_RESET_SIGNATURE 	0xCA
#define WDT_MAX_OFF_INTERVAL_S 4147200 //48 days

#define OWB_TEMP_SIZE_B 2
#define OWB_SENS_CNT 10


typedef enum
{
	I2C_MEM_RELAY_VAL_ADD = 0,
	I2C_MEM_RELAY_SET_ADD,
	I2C_MEM_RELAY_CLR_ADD,
	I2C_MEM_OPTO_IN_ADD,
	I2C_MEM_GPIO_VAL_ADD,
	I2C_MEM_GPIO_SET_ADD,
	I2C_MEM_GPIO_CLR_ADD,
	I2C_MEM_GPIO_DIR_ADD,

	I2C_MEM_ADC_VAL_RAW_ADD,
	I2C_MEM_ADC_VAL_MV_ADD = I2C_MEM_ADC_VAL_RAW_ADD
		+ ADC_CH_NO * ADC_RAW_VAL_SIZE,
	I2C_MEM_DAC_VAL_MV_ADD = I2C_MEM_ADC_VAL_MV_ADD
		+ ADC_CH_NO * ADC_RAW_VAL_SIZE,
	I2C_MEM_OD_PWM_VAL_RAW_ADD = I2C_MEM_DAC_VAL_MV_ADD
		+ DAC_CH_NO * DAC_MV_VAL_SIZE,
	I2C_MEM_OPTO_IT_RISING_ADD = I2C_MEM_OD_PWM_VAL_RAW_ADD
		+ DAC_CH_NO * DAC_MV_VAL_SIZE,
	I2C_MEM_OPTO_IT_FALLING_ADD,
	I2C_MEM_GPIO_EXT_IT_RISING_ADD,
	I2C_MEM_GPIO_EXT_IT_FALLING_ADD,
	I2C_MEM_OPTO_CNT_RST_ADD,
	I2C_MEM_GPIO_CNT_RST_ADD,

	I2C_MEM_DIAG_TEMPERATURE_ADD,

	I2C_MEM_DIAG_3V3_MV_ADD,
	I2C_MEM_DIAG_3V3_MV_ADD1,

	I2C_MEM_CALIB_VALUE,
	I2C_MEM_CALIB_CHANNEL = I2C_MEM_CALIB_VALUE + 2, //ADC channels [1,8]; DAC channels [9, 12]
	I2C_MEM_CALIB_KEY, //set calib point 0xaa; reset calibration on the channel 0x55
	I2C_MEM_CALIB_STATUS,

	I2C_MEM_OPTO_ENC_ENABLE_ADD,
	I2C_MEM_GPIO_ENC_ENABLE_ADD,
	I2C_MEM_OPTO_ENC_CNT_RST_ADD,
	I2C_MEM_GPIO_ENC_CNT_RST_ADD,

	I2C_MEM_WDT_RESET_ADD = 100,
	I2C_MEM_WDT_INTERVAL_SET_ADD,
	I2C_MEM_WDT_INTERVAL_GET_ADD = I2C_MEM_WDT_INTERVAL_SET_ADD + 2,
	I2C_MEM_WDT_INIT_INTERVAL_SET_ADD = I2C_MEM_WDT_INTERVAL_GET_ADD + 2,
	I2C_MEM_WDT_INIT_INTERVAL_GET_ADD = I2C_MEM_WDT_INIT_INTERVAL_SET_ADD + 2,
	I2C_MEM_WDT_RESET_COUNT_ADD = I2C_MEM_WDT_INIT_INTERVAL_GET_ADD + 2,
	I2C_MEM_WDT_CLEAR_RESET_COUNT_ADD = I2C_MEM_WDT_RESET_COUNT_ADD + 2,

	I2C_MEM_WDT_POWER_OFF_INTERVAL_SET_ADD,
	I2C_MEM_WDT_POWER_OFF_INTERVAL_GET_ADD = I2C_MEM_WDT_POWER_OFF_INTERVAL_SET_ADD
		+ 4,

	I2C_MEM_REVISION_HW_MAJOR_ADD = 0x78,
	I2C_MEM_REVISION_HW_MINOR_ADD,
	I2C_MEM_REVISION_MAJOR_ADD,
	I2C_MEM_REVISION_MINOR_ADD,
	I2C_DBG_FIFO_SIZE,
	I2C_DBG_FIFO_ADD = I2C_DBG_FIFO_SIZE + 2,
	I2C_DBG_CMD,
	I2C_MEM_OPTO_EDGE_COUNT_ADD,
	I2C_MEM_OPTO_EDGE_COUNT_END_ADD = I2C_MEM_OPTO_EDGE_COUNT_ADD
		+ COUNTER_SIZE * OPTO_CH_NO, //!gap
	I2C_MEM_OD_PWM_FREQUENCY, //2 bytes
	I2C_MEM_ADD_RESERVED = 0xaa,
	I2C_MEM_GPIO_EDGE_COUNT_ADD,
	I2C_MEM_OPTO_ENC_COUNT_ADD = I2C_MEM_GPIO_EDGE_COUNT_ADD
		+ COUNTER_SIZE * GPIO_CH_NO,
	I2C_MEM_GPIO_ENC_COUNT_ADD = I2C_MEM_OPTO_ENC_COUNT_ADD
		+ COUNTER_SIZE * OPTO_CH_NO / 2,
	I2C_MEM_GPIO_ENC_COUNT_END_ADD = I2C_MEM_GPIO_ENC_COUNT_ADD
		+ COUNTER_SIZE * GPIO_CH_NO / 2,
   I2C_MEM_1WB_DEV = I2C_MEM_GPIO_ENC_COUNT_END_ADD,
	I2C_MEM_1WB_ROM_CODE_IDX ,
	I2C_MEM_1WB_ROM_CODE,//rom code 64 bits
	I2C_MEM_1WB_ROM_CODE_END = I2C_MEM_1WB_ROM_CODE + 7,
	I2C_MEM_1WB_START_SEARCH,
		I2C_MEM_1WB_T1,
		I2C_MEM_1WB_T16 = I2C_MEM_1WB_T1 + OWB_SENS_CNT * OWB_TEMP_SIZE_B,
	SLAVE_BUFF_SIZE,
} I2C_MEM_ADD;

#define CHANNEL_NR_MIN		1
#define RELAY_CH_NR_MAX		8
#define OPTO_IN_CH_NR_MAX	8
#define GPIO_CH_NR_MAX		4
#define OD_CH_NR_MAX			4
#define DAC_CH_NR_MAX		4
#define ADC_CH_NR_MAX		8

#define OD_PWM_VAL_MAX	10000

#define ERROR	-1
#define OK		0
#define FAIL	-1
#define ARG_ERR -2
#define ARG_CNT_ERR -3

#define SLAVE_OWN_ADDRESS_BASE 0x28

typedef uint8_t u8;
typedef uint16_t u16;
typedef int16_t s16;
typedef uint32_t u32;
typedef int32_t s32;

typedef enum
{
	OFF = 0,
	ON,
	STATE_COUNT
} OutStateEnumType;

int doBoardInit(int stack);
u8 getHwVer(void);
int adcGet(int dev, int ch, float *val);
int odSet(int dev, int ch, float val);
int dacSet(int dev, int ch, float val);


int gpioChSet(int dev, u8 channel, OutStateEnumType state);
int gpioChGet(int dev, u8 channel, OutStateEnumType *state);
int gpioChDirSet(int dev, u8 channel, u8 state);
int doGpioRead(int argc, char *argv[]);
int doGpioDirWrite(int argc, char *argv[]);
int doGpioDirRead(int argc, char *argv[]);
int doGpioEdgeWrite(int argc, char *argv[]);
int doGpioEdgeRead(int argc, char *argv[]);
int doGpioCntRead(int argc, char *argv[]);
int doGpioCntRst(int argc, char *argv[]);
int doGpioWrite(int argc, char *argv[]);

int optoChGet(int dev, u8 channel, OutStateEnumType *state);
int doOptoRead(int argc, char *argv[]);
int doOptoEdgeWrite(int argc, char *argv[]);
int doOptoEdgeRead(int argc, char *argv[]);
int doOptoCntRead(int argc, char *argv[]);
int doOptoCntReset(int argc, char *argv[]);
int doOptoEncoderWrite(int argc, char *argv[]);
int doOptoEncoderRead(int argc, char *argv[]);
int doOptoEncoderCntRead(int argc, char *argv[]);
int doOptoEncoderCntReset(int argc, char *argv[]);

int doLoopbackTest(int argc, char *argv[]);

#endif //IOPLUS_H_
