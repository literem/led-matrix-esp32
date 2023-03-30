/*
 * display_clock.c
 *
 *  Created on: 2023年1月8日
 *      Author: literem
 */

#include "display_clock.h"
#include "display.h"
#include "display_static.h"
#include "timer.h"
#include "command.h"
#include "rx8025t.h"
#include "freertos/task.h"
#include <string.h>

#define MH	10

u8 num_8x16[][16] = {
	{0x00,0x00,0x00,0x18,0x24,0x42,0x42,0x42,0x42,0x42,0x42,0x42,0x24,0x18,0x00,0x00},/*"0",0*/
	{0x00,0x00,0x00,0x08,0x38,0x08,0x08,0x08,0x08,0x08,0x08,0x08,0x08,0x3E,0x00,0x00},/*"1",1*/
	{0x00,0x00,0x00,0x3C,0x42,0x42,0x42,0x02,0x04,0x08,0x10,0x20,0x42,0x7E,0x00,0x00},/*"2",2*/
	{0x00,0x00,0x00,0x3C,0x42,0x42,0x02,0x04,0x18,0x04,0x02,0x42,0x42,0x3C,0x00,0x00},/*"3",3*/
	{0x00,0x00,0x00,0x04,0x0C,0x0C,0x14,0x24,0x24,0x44,0x7F,0x04,0x04,0x1F,0x00,0x00},/*"4",4*/
	{0x00,0x00,0x00,0x7E,0x40,0x40,0x40,0x78,0x44,0x02,0x02,0x42,0x44,0x38,0x00,0x00},/*"5",5*/
	{0x00,0x00,0x00,0x18,0x24,0x40,0x40,0x5C,0x62,0x42,0x42,0x42,0x22,0x1C,0x00,0x00},/*"6",6*/
	{0x00,0x00,0x00,0x7E,0x42,0x04,0x04,0x08,0x08,0x10,0x10,0x10,0x10,0x10,0x00,0x00},/*"7",7*/
	{0x00,0x00,0x00,0x3C,0x42,0x42,0x42,0x24,0x18,0x24,0x42,0x42,0x42,0x3C,0x00,0x00},/*"8",8*/
	{0x00,0x00,0x00,0x38,0x44,0x42,0x42,0x42,0x46,0x3A,0x02,0x02,0x24,0x18,0x00,0x00},/*"9",9*/
	{0x00,0x00,0x00,0x00,0x00,0x00,0x18,0x18,0x00,0x00,0x00,0x00,0x18,0x18,0x00,0x00} /*":",10*/
};
u8 num_8x8[][8]={
	{0x1C,0x22,0x26,0x2A,0x32,0x22,0x1C,0x00},
	{0x08,0x18,0x08,0x08,0x08,0x08,0x1C,0x00},
	{0x1C,0x22,0x02,0x04,0x08,0x10,0x3E,0x00},
	{0x1C,0x22,0x02,0x0C,0x02,0x22,0x1C,0x00},
	{0x04,0x0C,0x14,0x24,0x3E,0x04,0x04,0x00},
	{0x3E,0x20,0x3C,0x02,0x02,0x22,0x1C,0x00},
	{0x0C,0x10,0x20,0x3C,0x22,0x22,0x1C,0x00},
	{0x3E,0x02,0x04,0x08,0x10,0x10,0x10,0x00},
	{0x1C,0x22,0x22,0x1C,0x22,0x22,0x1C,0x00},
	{0x1C,0x22,0x22,0x1E,0x02,0x04,0x18,0x00}
};


u8 startNumberPos = 0;
u8 clockMode = 0;
u8 secondFlag = 0;
DateTime datetime;

void copyData8x16(u8 number,u8 pos)
{
	u8 i = (pos%2==0) ? 1 : 0;
	u8 *ptr = (u8 *)&p[pos/2][0];
	for(pos=0;i<32;i+=2,pos++)
	{
		ptr[i] = num_8x16[number][pos];
	}
}

void copyData8x8(u8 number,u8 pos)
{
	u8 i = (pos%2==0) ? 17 : 16;
	u8 *ptr = (u8 *)&p[pos/2][0];
	for(pos=0;i<32;i+=2,pos++)
	{
		ptr[i] = num_8x8[number][pos];
	}
}

void copyDot(u8 isShow,u8 pos)
{
	u8 i = (pos%2==0) ? 1 : 0;
	u8 *ptr = (u8 *)&p[pos/2][0];
	isShow = (isShow==1) ? 0x18 : 0x00;
	ptr[i+12] = isShow;
	ptr[i+14] = isShow;
	ptr[i+24] = isShow;
	ptr[i+26] = isShow;
}

/**
 * 从分开始更新时间
 */
void increaseMinute()
{
	datetime.minute = rtcGetMinute();
	if(datetime.minute == 0)
	{
		datetime.hour = rtcGetHour();
		copyData8x16(datetime.hour/10,startNumberPos);
		copyData8x16(datetime.hour%10,startNumberPos+1);
	}
	copyData8x16(datetime.minute/10,startNumberPos+3);
	copyData8x16(datetime.minute%10,startNumberPos+4);
}

/**
 * 从秒开始更新时间
 */
void increaseSecond()
{
	datetime.second = rtcGetSecond();
	if(datetime.second == 0) increaseMinute();
	copyData8x8(datetime.second/10,startNumberPos+5);
	copyData8x8(datetime.second%10,startNumberPos+6);
}

/**
 * 读取RX8025T的时间并显示出来
 */
void displayTime()
{
	u8 start = startNumberPos;
	rtcGetDateTime(&datetime);
	copyData8x16(datetime.hour/10,start++);
	copyData8x16(datetime.hour%10,start++);
	copyData8x16(MH,start++);
	copyData8x16(datetime.minute/10,start++);
	copyData8x16(datetime.minute%10,start++);
	if(clockMode == CLOCK_HMS)//如果为显示秒的模式，则要显示秒
	{
		copyData8x8(datetime.second/10,start++);
		copyData8x8(datetime.second%10,start++);
	}
}

/*
 * 更新时间，此函数应由RX8025T产生脉冲进入外部中断执行
 * 若当前显示状态不为DISPLAY_CLOCK，自动关闭RX8025T引脚的时间更新脉冲
 */
void updateTime()
{
	if(matrixData.displayStatus != DISPLAY_CLOCK)
	{
	    rx8025tResetINTA();
		return;
	}
	if(clockMode == CLOCK_HMS) increaseSecond();
	else increaseMinute();
}

/**
 * 时钟计数，此函数由定时任务定时1000ms执行
 */
void clockCount()
{
	datetime.second++;
	if(datetime.second == 60) datetime.second=0;
	if(clockMode == CLOCK_HMS)
	{
		copyData8x8(datetime.second/10,startNumberPos+5);
		copyData8x8(datetime.second%10,startNumberPos+6);
	}
	else
	{
		copyDot(datetime.second%2,startNumberPos+2);
	}
}

/**
 * 设置时钟模式，开启点阵时钟
 */
u8 initDisplayClock(u8 mode)
{
	if(mode >= 3) return CMD_ERROR;//目前仅提供两种模式
	memset(p,0,matrixData.moduleSize * 32);
	clockMode = mode;
	startNumberPos = (clockMode==CLOCK_HMS) ? 7 : 5;//两种模式（显示时分秒和时分）分别需要7个和5个 8*16的位置
	startNumberPos = ((matrixData.moduleSize * 2) - startNumberPos) / 2;//计算开始的位置，从中间显示
	displayTime();
	if(matrixData.displayStatus != DISPLAY_CLOCK)//如果是首次设置，进行时钟显示的初始化
	{
		matrixData.displayStatus = DISPLAY_CLOCK;
		rx8025tSetINTA(CLOCK_HM);//设置产生更新中断的方式，秒中断或分中断，这里默认为分中断
		calculateDisplayPosition(0,matrixData.moduleSize);//计算显示位置
		setShowState(SHOW_STATIC);//设置静态显示
		setTimerState(STATE_CLOCK,1000);//开启定时任务，更新时间
	}
	return RESPONSE_OK;
}

/**
 * 设置时间
 */
u8 setClockDateTime(u8 *data,u8 len)
{
	if(len != 7) return VALUE_ERROR;
	datetime.year = data[0];
	datetime.month = data[1];
	datetime.day = data[2];
	datetime.week = data[3];
	datetime.hour = data[4];
	datetime.minute = data[5];
	datetime.second = data[6];
	rtcSetDateTime(&datetime);
	updateTime();
	return RESPONSE_OK;
}

