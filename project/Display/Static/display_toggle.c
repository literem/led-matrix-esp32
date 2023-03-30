/*
 * display_toggle.c
 *
 *  Created on: 2022年12月4日
 *      Author: literem
 */

#include "command.h"
#include "receive.h"
#include "display.h"
#include "display_toggle.h"
#include "display_static.h"
#include "setting.h"
#include "at24cxx.h"
#include <string.h>
#include <stdio.h>
#include "bluetooth.h"
#include "timer.h"

u8 frameIndex = 0;//切换帧的索引位置
ToggleData toggleData;

/**
 * 初始化自动切换显示
 * *dat:自动切换显示数据，长度信息+数据信息
 * len:总的数据长度（长度信息+数据信息）
 * rowLen:行数
 * anim:显示的动画
 *
 * 数组的数据结构为，以3行、每行一个字的数据举例：
 * [len,len,len,data1,data1,data2,data2,data3,data3]
 */
u8 initAutoToggle(u8 *dat,u16 len,u8 rowLen,u8 anim,u8 isSave)
{
	if(len > GBK_ARRAY_LEN || rowLen == 0) return VALUE_ERROR;//判断长度
	memcpy(gbkArrayData,dat,len);//复制数据到toggleArray数组
	toggleData.length = len;//设置总的数据长度（长度信息+数据信息）
	toggleData.rowLength = rowLen;//设置行数
	toggleData.lenPos = 0;
	toggleData.dataPos = rowLen;//数据起始索引
	toggleData.anim = (anim > MOVE_PAGE_DOWN) ? MOVE_PAGE_DOWN : anim;
	matrixData.displayStatus = DISPLAY_TOGGLE;
	setTimerState(STATE_TOGGLE,matrixData.speedToggle);//开启定时器，定时读取字模地址数据到加载字模的任务中
	if(isSave == 1)
	{
		matrixData.saveDisplayMode = DISPLAY_TOGGLE;
		matrixData.runOnStart = 1;
		if(writeRunOnStart(DISPLAY_TOGGLE) == WRITE_ERROR) 		return WRITE_ERROR;//设置开机自启
		if(at24cxxWrite(ER_TOGGLE_LEN,rowLen) == WRITE_ERROR) 	return WRITE_ERROR;//数据第0位写入总行数
		if(at24cxxWrite(ER_TOGGLE_ANIM,anim) == WRITE_ERROR) 	return WRITE_ERROR;//数据第1位写入动画值
		return writeMultipleData(dat,len);//数据第2位写入GBK地址数据
	}
	return RESPONSE_OK;
}


/**
 * 从EEPROM中读取数据并初始化自动切换显示
 * dataLen:EEPROM中的数据长度
 */
u8 initAutoToggleFromER(u16 dataLen)
{
	u8 rowLen,anim;
	if(dataLen < 2 || dataLen > GBK_ARRAY_LEN ) return VALUE_ERROR;
	rowLen = at24cxxRead(ER_TOGGLE_LEN);
	if(rowLen == 0 || rowLen > dataLen) return VALUE_ERROR;
	anim = at24cxxRead(ER_TOGGLE_ANIM);
	readMultipleData(gbkArrayData,dataLen);
	toggleData.length = dataLen;//设置总的数据长度
	toggleData.rowLength = rowLen;//设置行数
	toggleData.lenPos = 0;
	toggleData.dataPos = rowLen;//数据起始索引
	toggleData.anim = (anim > MOVE_PAGE_DOWN) ? MOVE_PAGE_DOWN : anim;
	matrixData.displayStatus = DISPLAY_TOGGLE;
	setTimerState(STATE_TOGGLE,matrixData.speedToggle);//开启定时器，定时读取字模地址数据到加载字模的任务中
	return RESPONSE_OK;
}

/**
 * 寻找数组中出现两个0xFF分隔符的字模地址数据作为一行，
 * 然后发送到loadDataTask任务中读取该行字模数据并显示出来
 */
void findToggleIndex()
{
	u8 curRowLen = gbkArrayData[toggleData.lenPos] * 2;//取出当前行的长度信息
	//从toggleData.dataPos取出数据，发送curRowLen个长度
	xMessageBufferSend(MessageBufferHandle, (void*)&gbkArrayData[toggleData.dataPos], curRowLen, 0);//第0位是type位，所以-1
	toggleData.dataPos += curRowLen;//取完数据后，索引加上当前行的长度
	toggleData.lenPos++;//长度索引自增1
	if(toggleData.lenPos == toggleData.rowLength)//超过最后一个长度索引
	{
		toggleData.lenPos = 0;
		toggleData.dataPos = toggleData.rowLength;
	}
}

u8 setFrameStart()
{
	frameIndex = matrixData.moduleSize;
	stopTimer();
	if(matrixData.displayStatus != DISPLAY_CUSTOM)//如果当前模式不处于display_custom，则进行初始化操作
	{
		matrixData.displayStatus = DISPLAY_CUSTOM;//设置为DISPLAY_CUSTOM状态
		memset(p,0,32*matrixData.moduleSize);//清空moduleSize*32个数组的长度
		calculateDisplayPosition(0,matrixData.moduleSize);//计算显示位置为 0 ~ matrixData.moduleSize
		setShowState(SHOW_STATIC);//设置静态显示
	}
	return RESPONSE_OK;
}

u8 addFramePart(u8 *dat,u16 dataLen)
{
	u8 frameLen = dataLen / 32;
	if(dataLen == 0 || (dataLen%32) != 0) return VALUE_ERROR;//检查数据长度是否正确
	if((frameIndex + frameLen) >= (FONT_LEN - matrixData.moduleSize)) return VALUE_ERROR;//检查数据长度是否超出范围
	memcpy(&p[frameIndex][0],dat,dataLen);//保存帧数据
	frameIndex += frameLen;
	printf("add freame:%d\n",frameIndex);
	return RESPONSE_OK;
}

u8 setFrameEnd(u8 moduleIndex)
{
	//检查模块索引是否超过点阵长度，并检查是否未设置或只设置了1帧数据
	if(moduleIndex >= matrixData.moduleSize) return VALUE_ERROR;
	if(frameIndex == matrixData.moduleSize || frameIndex == matrixData.moduleSize+1)return VALUE_ERROR;
	arrayData.fontSize = frameIndex;//实际的帧长度，frameIndex
	arrayData.fontIndex = moduleIndex;//设置帧动画在点阵模块中的显示位置
	frameIndex = matrixData.moduleSize;//开始帧位置
	setTimerState(STATE_FRAME,200);
	return RESPONSE_OK;
}

/**
 * 帧动画切换函数
 */
void fontFrame()
{
	if(frameIndex == arrayData.fontSize) frameIndex = matrixData.moduleSize;
	memcpy(&p[arrayData.fontIndex][0],&p[frameIndex][0],32);
	frameIndex++;
}
