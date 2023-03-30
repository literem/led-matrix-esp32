/*
 * Receive.c
 *
 *  Created on: 2022年11月25日
 *      Author: literem
 */
#include "receive.h"
#include "setting.h"
#include "getting.h"
#include "display.h"
#include "display_toggle.h"
#include "bluetooth.h"
#include "command.h"
#include "response.h"

/**
 * 返回处理的结果
 */
void responseResult(u8 state,u8 highBit,u8 lowBit)
{
	cleanResponseData();
	setResponseNumber(highBit,lowBit);
	btSendResponse(createResponseData(state));
}

/**
 * 处理获取数据的命令
 */
void handleGetCommand(u8 *recData)
{
	u8 *result;
	switch(CMD2_BIT(recData))
	{
		case DEVICE_INFO: 	 result = getDeviceInfo(CMD3_BIT(recData)); break;//获取设备信息
		case DISPLAY_SCROLL: result = getDisplayScroll(CMD3_BIT(recData)); break;//滚动显示
		case DISPLAY_STATIC: result = getDisplayStatic(CMD3_BIT(recData)); break;//静态显示
		case DISPLAY_CLOCK:	 result = getDisplayClock(CMD3_BIT(recData));break;//时钟显示
		default:cleanResponseData();result = createResponseData(CMD_ERROR);break;
	}
	setResponseNumber(RSP_H(recData),RSP_L(recData));
	btSendResponse(result);
}

/**
 * 处理设置命令
 */
void handleSetCommand(u8 *recData,u16 len)
{
	u8 result;
	switch(CMD2_BIT(recData))
	{
		case DISPLAY_STATIC: result = setDisplayStatic(CMD3_BIT(recData),recData,len); break;//静态显示设置
		case DISPLAY_CUSTOM: result = setDisplayCustom(CMD3_BIT(recData),recData,len); break;//自定义显示
		case DISPLAY_SCROLL: result = setDisplayScroll(CMD3_BIT(recData),recData,len); break;//滚动显示设置
		case DISPLAY_TOGGLE: result = setDisplayToggle(CMD3_BIT(recData),recData,len); break;//自动切换设置
		case DISPLAY_CLOCK:  result = setDisplayClock(CMD3_BIT(recData),recData,len);  break;//时钟显示设置
		case MATRIX_DATA:result = setMatrixData(CMD3_BIT(recData),CMD_VALUE_1BIT(recData));break;//设置点阵数据
		default: result = CMD_ERROR;
	}
	cleanResponseData();
	setResponseDataType(SET);
	setResponseNumber(RSP_H(recData),RSP_L(recData));
	btSendResponse(createResponseData(result));
}

/**
 * 命令处理调度函数
 * recData:接收的数据
 * len:接收的数据长度
 */
void receiveHandle(u8 *recData,u16 len)
{
	//检查起始命令是否为0xff
	//检查结束命令是否为0x00
	//检查命令长度是否正确
	if(START_BIT(recData) != 0xff || END_BIT(recData,len) != 0x00 || CMD_LEN_BIT(recData) != len)
	{
		responseResult(CMD_ERROR,RSP_H(recData),RSP_L(recData));
		return;
	}

	if(CMD1_BIT(recData) == SET)
		handleSetCommand(recData,len);
	else if(CMD1_BIT(recData) == GET)
		handleGetCommand(recData);
	else
		responseResult(CMD_ERROR,RSP_H(recData),RSP_L(recData));
}

