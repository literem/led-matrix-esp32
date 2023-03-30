/*
 * getting.c
 *
 *  Created on: 2023年1月27日
 *      Author: literem
 */

#include "getting.h"
#include "command.h"
#include "display.h"
#include "receive.h"
#include "response.h"
#include "timer.h"
#include "display_clock.h"
#include "key.h"

/**
 * 获取设备信息
 */
u8* getDeviceInfo(u8 cmd3)
{
	u8 isNotAll = 1;//默认获取单条数据，只有cmd3==DEVICE_INFO_ALL时才不break
	cleanResponseData();
	switch(cmd3)
	{
		case INFO_ALL:		 isNotAll = 0; // @suppress("No break at end of case")
		case MODULE_SIZE:	 addResponseData(MODULE_SIZE,matrixData.moduleSize);if(isNotAll) break; // @suppress("No break at end of case")
		case DISPLAY_STATE:	 addResponseData(DISPLAY_STATE,matrixData.displayStatus);if(isNotAll) break; // @suppress("No break at end of case")
		case RUN_ON_START:	 addResponseData(RUN_ON_START,matrixData.runOnStart);if(isNotAll) break; // @suppress("No break at end of case")
		case SAVED_MODE:	 addResponseData(SAVED_MODE,matrixData.saveDisplayMode);if(isNotAll) break; // @suppress("No break at end of case")
		case CHARGE_STATE:	 addResponseData(CHARGE_STATE,gpio_get_level(CH_5V));if(isNotAll) break; // @suppress("No break at end of case")
		case POWER_STATE: 	 addResponseData(POWER_STATE,gpio_get_level(IN_5V));break;// @suppress("No break at end of case")
		default:return createResponseData(CMD_ERROR);
	}
	setResponseDataType(cmd3);
	return createResponseData(RESPONSE_OK);
}

/**
 * 获取静态显示的信息
 */
u8* getDisplayStatic(u8 cmd3)
{
	cleanResponseData();
	if(cmd3 == ANIM_IN)
		addResponseData(ANIM_IN,matrixData.inAnim);
	else if(cmd3 == ANIM_OUT)
		addResponseData(ANIM_OUT,matrixData.outAnim);
	else if(cmd3 == SPEED)
		addResponseData(SPEED,matrixData.speedToggle);
	else
		return createResponseData(CMD_ERROR);
	setResponseDataType(GET);
	return createResponseData(RESPONSE_OK);
}

/**
 * 获取滚动显示的信息
 */
u8 *getDisplayScroll(u8 cmd3)
{
	cleanResponseData();
	if(cmd3 == SPEED)//获取滚动速度
		addResponseData(SPEED,matrixData.speedScroll);
	else if(cmd3 == STATE_SCROLL)//获取滚动状态
		addResponseData(STATE_SCROLL,showState==SHOW_SCROLL&&timerState==STATE_SCROLL);
	else
		return createResponseData(CMD_ERROR);
	setResponseDataType(GET);
	return createResponseData(RESPONSE_OK);
}

u8* getDisplayClock(u8 cmd3)
{
	cleanResponseData();
	if(cmd3 == MODE)
		addResponseData(MODE,clockMode);
	else
		return createResponseData(CMD_ERROR);
	setResponseDataType(GET);
	return createResponseData(RESPONSE_OK);
}

