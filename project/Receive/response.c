/*
 * response.c
 *
 *  Created on: 2023年1月27日
 *      Author: literem
 */
#include "response.h"
#include "command.h"
#include <string.h>

u8 responseData[30];
u8 responseCount=0;

/**
 * 清空命令返回数组
 */
void cleanResponseData()
{
	memset(responseData,0,30);
	responseData[0] = 0xFF;//起始位
	responseData[1] = 0xFF;
	responseCount= 7;
}

/**
 * 设置返回数字
 */
void setResponseNumber(u8 highBit,u8 lowBit)
{
	RSP_NUM_HIGH_BIT = highBit;
	RSP_NUM_LOW_BIT  = lowBit;
}

void setResponseDataType(u8 type)
{
	RSP_DATA_TYPE = type;
}

/**
 * 添加返回数据，标志+数据
 */
void addResponseData(u8 flag,u16 data)
{
	responseData[responseCount++] = flag;
	responseData[responseCount++] = data>>8;//高位
	responseData[responseCount++] = data&0xff;//低位
}

/**
 * 生成数据并返回
 */
u8* createResponseData(u8 state)
{
	responseData[responseCount++] = 0xFF;//结束位
	responseData[responseCount++] = 0xFF;//结束位
	RSP_DATA_LEN = responseCount;//设置数据长度
	RSP_STATE = state;//设置响应状态
	return responseData;
}




