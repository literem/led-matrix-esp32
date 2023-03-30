/*
 * setting.c
 *
 *  Created on: 2022年11月26日
 *      Author: literem
 */
#include "display.h"
#include "display_static.h"
#include "display_clock.h"
#include "display_toggle.h"
#include "receive.h"
#include "setting.h"
#include "command.h"
#include "timer.h"
#include "at24cxx.h"
#include <string.h>


u8 writeRunOnStart(u8 displayMode)
{
	if(at24cxxWrite(ER_DISPLAY_MODE,displayMode) == WRITE_ERROR) //写入显示类型
		return WRITE_ERROR;
	return at24cxxWrite(ER_RUN_START,1);//设置开机自启
}

void setAnim(u8 type,u8 value,InAnimFunction in,OutAnimFunction out)
{
	if(type == ANIM_IN)
	{
		matrixData.entryAnimation = in;
		matrixData.inAnim = value;
	}
	else
	{
		matrixData.exitAnimation = out;
		matrixData.outAnim = value;
	}
}

/**
 * 设置显示速度
 */
u8 setSpeed(u8 type,u16 speed)
{
	if(speed == 0)//如果值设为0则是停掉定时器
	{
		stopTimer();
	}
	else if(type == DISPLAY_TOGGLE)//设置切换速度
	{
		//范围是1 - 30秒
		if(speed < 1000 || speed > 30000) return VALUE_ERROR;
		matrixData.speedToggle = speed;
		at24cxxWrite2Byte(ER_SPEED_TOGGLE,speed);
		if(timerState == STATE_TOGGLE) startTimer(matrixData.speedToggle);
	}
	else if(type == DISPLAY_SCROLL)//设置滚动速度
	{
		if(speed == 1)//设置保存的速度
		{
			if(matrixData.displayStatus == DISPLAY_SCROLL) setTimerState(STATE_SCROLL,matrixData.speedScroll);
			return RESPONSE_OK;
		}
		//范围是30 - 3000毫秒
		if(speed < 30 || speed > 3000 ) return VALUE_ERROR;
		matrixData.speedScroll = speed;
		at24cxxWrite2Byte(ER_SPEED_SCROLL,speed);
		if(timerState == STATE_SCROLL) startTimer(matrixData.speedScroll);
	}
	else if(type == DISPLAY_CUSTOM)//设置帧切换速度
	{
		if(speed < 50 || speed > 10000 ) return VALUE_ERROR;
		if(matrixData.displayStatus == DISPLAY_CUSTOM) setTimerState(STATE_FRAME,speed);//速度大于50ms并且当前为帧显示状态才能设置时间
	}
	else
		return CMD_ERROR;
	return RESPONSE_OK;
}

/**
 * 设置静态显示的切换动画
 * type:进入或退出动画类型
 * value:具体的动画
 */
void setAnimation(u8 type,u8 value)
{
	switch(value)
	{
		//进入动画独有
		case anim_scroll_bottom:setAnim(ANIM_IN,value,anim_bottom_to_top_scroll,anim_none_out);break;
		case anim_scroll_top:	setAnim(ANIM_IN,value,anim_top_to_bottom_scroll,anim_none_out);break;

		//进入、退出动画共有
		case anim_left_right:	setAnim(type,value,anim_left_to_right_in,anim_left_to_right_out);break;
		case anim_middle_open:	setAnim(type,value,anim_middle_open_in,anim_middle_open_out);break;
		case anim_both_close:	setAnim(type,value,anim_both_close_in,anim_both_close_out);break;
		case anim_bottom_top:	setAnim(type,value,anim_bottom_top_in,anim_bottom_top_out);break;
		case anim_top_bottom:	setAnim(type,value,anim_top_bottom_in,anim_top_bottom_out);break;

		//退出动画独有
		case anim_right_left:	setAnim(ANIM_OUT,value,anim_none_in,anim_right_to_left_out);break;
		default:				setAnim(type,0,anim_none_in,anim_none_out);break;
	}
}

/**
 * 复制接收的字模数据到p数组中
 * dat:要复制的字模数据
 * pos:复制到p数组的哪个位置
 * len:数据长度必须为32的倍数
 */
u8 copyDisplayCustomData(u8 *dat,u8 pos,u16 len)
{
	if(pos >= matrixData.moduleSize) return VALUE_ERROR;//检查pos是否正确
	if(len == 0 || (len%32) != 0) return VALUE_ERROR;//检查数据长度是否正确
	if(matrixData.displayStatus != DISPLAY_CUSTOM)//如果当前模式不处于display_custom，则进行初始化操作
	{
		matrixData.displayStatus = DISPLAY_CUSTOM;//设置为DISPLAY_CUSTOM状态
		stopTimer();
		memset(p,0,32*matrixData.moduleSize);//清空moduleSize*32个数组的长度
		calculateDisplayPosition(0,matrixData.moduleSize);//计算显示位置
		setShowState(SHOW_STATIC);//设置静态显示
	}
	memcpy(&p[pos][0],dat,len);
	return RESPONSE_OK;
}

//滚动显示
u8 setDisplayScroll(u8 cmd3,u8 *recData,u16 len)
{
	switch(cmd3)
	{
		case SPEED://设置速度
			return setSpeed(DISPLAY_SCROLL,CMD_VALUE_2BIT(recData));

		case RUN_ON_START://加载开机自启的滚动模式
			matrixData.displayStatus = DISPLAY_SCROLL;
			recData[0] = DISPLAY_SCROLL;//设置命令值，发送到加载字模的任务中处理
			sendBuff(recData,len);
			break;

		case SET_RUN_ON_START://保存开机自启的数据
			CHECK_TYPE_IS_GBK(recData);
			matrixData.runOnStart = 1;
			matrixData.saveDisplayMode = DISPLAY_SCROLL;
			matrixData.displayStatus = DISPLAY_SCROLL;
			DATA_TYPE_BIT(recData) = DISPLAY_SCROLL;//设置命令值
			sendBuff(&DATA_TYPE_BIT(recData),DATA_TYPE_LEN(len));
			if(writeRunOnStart(DISPLAY_SCROLL) == WRITE_ERROR) return WRITE_ERROR;//设置开机自启
			return writeMultipleData(&DATA_TYPE_BIT(recData),DATA_TYPE_LEN(len));

		case MODE://设置滚动模式
			CHECK_TYPE_IS_GBK(recData);
			matrixData.displayStatus = DISPLAY_SCROLL;
			DATA_TYPE_BIT(recData) = DISPLAY_SCROLL;//设置命令值，发送到加载字模的任务中处理
			sendBuff(&DATA_TYPE_BIT(recData),DATA_TYPE_LEN(len));
			break;

		default:
			return CMD_ERROR;
	}
	return RESPONSE_OK;
}

//静态显示设置
u8 setDisplayStatic(u8 cmd3,u8 *recData,u16 len)
{
	switch(cmd3)
	{
		case MOVE_HOR://水平移动给定距离
			if(matrixData.displayStatus != DISPLAY_STATIC) matrixData.displayStatus = DISPLAY_STATIC;
			arrayData.fontColumnSize = CMD_VALUE_1BIT(recData);//设置滚动的长度
			if(arrayData.fontColumnSize == 0) return CMD_ERROR;//校验移动长度
			sendNotify(MOVE_HOR);
			break;

		case MOVE_ANIM://动画显示
		case MOVE_PAGE_UP://上翻页显示
		case MOVE_PAGE_DOWN://下翻页显示
			CHECK_TYPE_IS_GBK(recData);//检查是否为GBK类型
			if(matrixData.displayStatus != DISPLAY_STATIC) matrixData.displayStatus = DISPLAY_STATIC;
			DATA_TYPE_BIT(recData) = cmd3;//设置切换命令值，发送到加载字模的任务中处理
			sendBuff(&DATA_TYPE_BIT(recData),DATA_TYPE_LEN(len));
			break;

		case ANIM_IN://设置进入动画
			setAnimation(ANIM_IN, CMD_VALUE_1BIT(recData));
			return at24cxxWrite(ER_IN_ANIM, CMD_VALUE_1BIT(recData));

		case ANIM_OUT://设置退出动画
			setAnimation(ANIM_OUT,CMD_VALUE_1BIT(recData));
			return at24cxxWrite(ER_OUT_ANIM, CMD_VALUE_1BIT(recData));

		default:return CMD_ERROR;
	}
	return RESPONSE_OK;
}

//自动切换显示
u8 setDisplayToggle(u8 cmd3,u8 *recData,u16 len)
{
	switch(cmd3)
	{
		case SPEED: //设置切换速度
			return setSpeed(DISPLAY_TOGGLE,CMD_VALUE_2BIT(recData));
		case DISPLAY_ANIM: //设置切换动画
			toggleData.anim = (CMD_VALUE_BIT1(recData) > MOVE_PAGE_DOWN) ? MOVE_PAGE_DOWN : CMD_VALUE_BIT1(recData);
			return RESPONSE_OK;
		case MODE: //设置自动切换模式
			return initAutoToggle(&DATA_BIT(recData), DATA_LEN(len), CMD_VALUE_BIT1(recData), CMD_VALUE_BIT2(recData),0);
		case SET_RUN_ON_START: //设置自动切换模式，并保存到flash下次上电启动时自动显示
			return initAutoToggle(&DATA_BIT(recData), DATA_LEN(len), CMD_VALUE_BIT1(recData), CMD_VALUE_BIT2(recData),1);
		default:
			return CMD_ERROR;
	}
}

//时钟显示
u8 setDisplayClock(u8 cmd3,u8 *recData,u16 len)
{
	switch(cmd3)
	{
		case MODE://开启时钟
			return initDisplayClock(CMD_VALUE_1BIT(recData));
		case DATETIME://设置日期和时间
			return setClockDateTime(&DATA_BIT(recData),DATA_LEN(len));
		case SET_RUN_ON_START://设置开机自启并开启时钟
			matrixData.saveDisplayMode = DISPLAY_CLOCK;
			matrixData.runOnStart = 1;
			if(writeRunOnStart(DISPLAY_CLOCK) == WRITE_ERROR) return WRITE_ERROR;//设置开机自启
			at24cxxWrite(ER_DATA_ADDR,CMD_VALUE_1BIT(recData));//数据第0位写入时钟模式
			return initDisplayClock(CMD_VALUE_1BIT(recData));
		default:return CMD_ERROR;
	}
}

//自定义显示
u8 setDisplayCustom(u8 cmd3,u8 *recData,u16 len)
{
	switch(cmd3)
	{
		case FONT_DATA://复制字模数据
			CHECK_TYPE_IS_FONT(recData);//检查是否为FONT类型
			return copyDisplayCustomData(&DATA_BIT(recData),CMD_VALUE_1BIT(recData),DATA_LEN(len));

		case FONT_FRAME_HEAD://开始添加帧动画，做准备工作
			return setFrameStart();

		case FONT_FRAME_PART://添加新的帧动画
			CHECK_TYPE_IS_FONT(recData);//检查是否为FONT类型
			return addFramePart(&DATA_BIT(recData),DATA_LEN(len));

		case FONT_FRAME_END://结束添加帧动画，并播放帧动画
			return setFrameEnd(CMD_VALUE_1BIT(recData));

		case FONT_FRAME_SPEED://设置帧动画切换速度，当速度为0时，停止定时器
			return setSpeed(DISPLAY_CUSTOM,CMD_VALUE_2BIT(recData));

		default: return CMD_ERROR;
	}
}

//设置点阵屏的相关数据
u8 setMatrixData(u8 cmd3,u8 value)
{
	switch(cmd3)
	{
		case MODULE_SIZE://设置点阵模块大小
			if(value < 2 || value > 20) return VALUE_ERROR;
			matrixData.moduleSize = value;
			matrixData.columnSize = value*16;
			stopTimer();
			setShowState(SHOW_NONE);
			matrixData.displayStatus = DISPLAY_NONE;
			return at24cxxWrite(ER_MODULE_SIZE,value);

		case SET_RUN_ON_START://设置是否开机自启
			matrixData.runOnStart = value==0?0:1;
			return at24cxxWrite(ER_RUN_START,matrixData.runOnStart);

		case DISPLAY_NONE://停止显示
			stopTimer();
			setShowState(SHOW_NONE);
			matrixData.displayStatus = DISPLAY_NONE;
			return RESPONSE_OK;

		default:
			return CMD_ERROR;
	}
}


