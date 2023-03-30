/*
 * display.c
 *
 *  Created on: 2022年11月23日
 *      Author: literem
 */
#include "display.h"
#include "display_static.h"
#include "display_scroll.h"
#include "display_toggle.h"
#include "display_clock.h"
#include "at24cxx.h"
#include "setting.h"
#include "timer.h"
#include "command.h"
#include "bluetooth.h"
#include "font.h"
#include "freertos/task.h"
#include <string.h>

#define SET_WAITING()		{isCanToggle=1;while(isCanToggle==1&&matrixData.displayStatus!=DISPLAY_NONE) vTaskDelay(1);}
#define RESET_WAITING()		isCanToggle=0

//点阵的结构体变量
MatrixData matrixData;

//数组数据的结构体变量
ArrayData arrayData;

//GBK地址缓冲
MessageBufferHandle_t MessageBufferHandle;

//静态和动态显示所需的变量
s8 i,j,k,n;

//字模数组长度：FONT_LENx16x2 bit
u16 p[FONT_LEN][16];

//GBK地址长度
u8 gbkArrayData[GBK_ARRAY_LEN];

//在task中的显示状态
u8 showState;

//是否能播放切换动画
u8 isCanToggle=1;

//任务句柄
TaskHandle_t toggleTaskHandle = NULL;
TaskHandle_t displayTaskHandle = NULL;
TaskHandle_t loadDataTaskHandle = NULL;

//任务函数
void displayToggleTask(void *pvParm);
void displayTask(void *pvParm);
void loadDataTask(void *pvParm);


void sendNotify(u8 notify)
{
	xTaskNotify(toggleTaskHandle,notify,eSetValueWithOverwrite);
}

void sendBuff(u8 *tx_buff,u8 buff_len)
{
	stopTimer();//当新数据来临时，关闭定时器
	xMessageBufferSend(MessageBufferHandle, (void*)tx_buff, buff_len, 0);
}

//设置显示状态
void setShowState(u8 state)
{
	if(showState == state) return;

	//如果当前状态为SHOW_STATIC，则退出SHOW_STATIC
	if(showState == SHOW_STATIC) isCanToggle = 1;

	//等待显示状态退出
	showState = SHOW_EXIT;
	while(showState == SHOW_EXIT) vTaskDelay(10);
	isCanToggle = 0;

	//退出后设置新的状态
	showState = state;
}

/**
 * 点阵显示初始化函数
 */
void displayInit()
{
	//读取模块数量
	matrixData.moduleSize = loadDataAndCheck(1,20,6,ER_MODULE_SIZE);
	matrixData.columnSize = matrixData.moduleSize*16;

	//读取动画
	matrixData.inAnim = loadDataAndCheck(0xE0,0xE7,0,ER_IN_ANIM);
	matrixData.outAnim = loadDataAndCheck(0xE0,0xE7,0,ER_OUT_ANIM);
	setAnimation(ANIM_IN,matrixData.inAnim);
	setAnimation(ANIM_OUT,matrixData.outAnim);

	//读取速度
	matrixData.speedScroll = load2ByteDataAndCheck(30,5000,200,ER_SPEED_SCROLL);
	matrixData.speedToggle = load2ByteDataAndCheck(1000,30000,5000,ER_SPEED_TOGGLE);

	//读取开机自动显示的状态
	matrixData.runOnStart = loadDataAndCheck(0,1,0,ER_RUN_START);

	//读取保存的模式
	matrixData.saveDisplayMode = loadDataAndCheck(0xB1,0xB5,DISPLAY_NONE,ER_DISPLAY_MODE);

	arrayData.fontSize = 0;
	arrayData.fontColumnSize = 0;
	showState = SHOW_NONE;
	MessageBufferHandle = xMessageBufferCreate(RX_BUF_LEN);
	xTaskCreatePinnedToCore(loadDataTask,"loadDataTask",1024*2,NULL,4,&loadDataTaskHandle,0);//加载字模任务
	xTaskCreatePinnedToCore(displayToggleTask,"toggleTask",1024*2,NULL,3,&toggleTaskHandle,1);//显示切换任务
	xTaskCreatePinnedToCore(displayTask,"displayTask",1024*2,NULL,5,&displayTaskHandle,1);//显示任务

	//如果不设置开启自启，则不往下执行
	if(matrixData.runOnStart == 0)
	{
		matrixData.displayStatus = DISPLAY_NONE;
		return;
	}
	u16 erDataLen = load2ByteDataAndCheck(10,300,0,ER_DATA_LEN);//读取数据的位置
	//根据保存的显示模式，调用对应的初始化函数
	switch(matrixData.saveDisplayMode)
	{
		case DISPLAY_SCROLL://滚动
			if(erDataLen < 2) break;
			if(readMultipleData(gbkArrayData,erDataLen))
				setDisplayScroll(RUN_ON_START,gbkArrayData,erDataLen);
			break;
		case DISPLAY_CUSTOM:break;
		case DISPLAY_TOGGLE://自动切换显示
			initAutoToggleFromER(erDataLen);
			break;
		case DISPLAY_CLOCK://时钟显示
			initDisplayClock(at24cxxRead(ER_DATA_ADDR));//数据第0位存放时钟的模式
			break;
		default:matrixData.displayStatus = DISPLAY_NONE;break;
	}
}

/**
 * 显示任务
 */
void displayTask(void *pvParm)
{
	u8 ii;
	for(;;)
	{
		//静态显示
		while(showState == SHOW_STATIC)
		{
			while(isCanToggle==0)
			{
				for(ii=0;ii<16;ii++) refresh(ii);
				vTaskDelay(5);
			}
			if(isCanToggle!=2) isCanToggle=2;
			vTaskDelay(5);
		}

		//滚动显示
		while(showState == SHOW_SCROLL)
		{
			scroll_head();
			scroll_content();
			scroll_end();
		}

		if(showState != SHOW_NONE) showState = SHOW_NONE;

		vTaskDelay(1000);
	}
}

/**
 *  显示切换任务
 */
void displayToggleTask(void *pvParm)
{
	u32 notifiedValue;
	for(;;)
	{
		xTaskNotifyWait(0x00, ULONG_MAX, &notifiedValue, portMAX_DELAY);//没有通知时阻塞
		switch(notifiedValue)
		{
			case MOVE_HOR://滚动一段距离
				if(matrixData.displayStatus != DISPLAY_STATIC) break;
				SET_WAITING();
				setHorizontalScroll(arrayData.fontColumnSize);
				RESET_WAITING();
				break;

			case MOVE_ANIM://动画显示
				SET_WAITING();
				matrixData.exitAnimation();
				matrixData.entryAnimation(arrayData.fontIndex,arrayData.fontSize);
				RESET_WAITING();
				break;

			case MOVE_PAGE_UP://上翻
				SET_WAITING();
				anim_top_to_bottom_scroll(arrayData.fontIndex,arrayData.fontSize);
				RESET_WAITING();
				break;

			case MOVE_PAGE_DOWN://下翻
				SET_WAITING();
				anim_bottom_to_top_scroll(arrayData.fontIndex,arrayData.fontSize);
				RESET_WAITING();
				break;
		}
		vTaskDelay(100);
	}
}

/**
 * 加载字模任务
 */
void loadDataTask(void *pvParm)
{
	u8 rx_buf[RX_BUF_LEN];
	u8 type;
	u8 start,index=0;
	u16 len;
	u16 address;
	for(;;)
	{
		len = xMessageBufferReceive(MessageBufferHandle,(void*)rx_buf,sizeof(rx_buf),portMAX_DELAY);

		//如果当前为滚动状态，先退出滚动，再加载字模数据
		if(matrixData.displayStatus == DISPLAY_SCROLL) setShowState(SHOW_NONE);
		if(matrixData.displayStatus == DISPLAY_TOGGLE)
		{
			start = 0;//DISPLAY_TOGGLE模式中没有type位
			type = toggleData.anim;
			goto NOT_SCROLL_HANDLER;
		}
		else
		{
			start = 1;
			type = rx_buf[0];
		}

		//处理滚动模式
		if(type == DISPLAY_SCROLL)
		{
			for(index=0;start<len;start+=2,index++)//加载字模数据
			{
				address = (rx_buf[start]<<8) + rx_buf[start+1];
				if(address > 22083)
				{
					memset(&p[index][0],0,32);
					continue;
				}
				memcpy(&p[index][0],&font[address][0],32);
			}
			arrayData.fontColumnSize = index * 16;//滚动显示是根据列长度判断位置的
			setShowState(SHOW_SCROLL);
			setTimerState(STATE_SCROLL,matrixData.speedScroll);
			continue;
		}

		NOT_SCROLL_HANDLER:
		index = (arrayData.fontIndex==0) ? FONT_HALF_LEN : 0;
		arrayData.fontIndex = index;//设置开始索引

		//加载字模数据
		for(;start<len;start+=2,index++)
		{
			address = (rx_buf[start]<<8) + rx_buf[start+1];
			if(address > 22083)
			{
				memset(&p[index][0],0,32);
				continue;
			}
			memcpy(&p[index][0],&font[address][0],32);
		}
		arrayData.fontSize = index - arrayData.fontIndex;//静态显示开始索引

		if(showState != SHOW_STATIC)//设置静态显示
		{
			calculateDisplayPosition(arrayData.fontIndex,arrayData.fontSize);
			setShowState(SHOW_STATIC);
			continue;
		}
		sendNotify(type);
		vTaskDelay(100);
	}
}

/*void listenerTask(void *pvParm)
{
	u16 size1=0,size2=0,size3;
	while(1)
	{
		size1 = uxTaskGetStackHighWaterMark(toggleTaskHandle);
		size2 = uxTaskGetStackHighWaterMark(displayTaskHandle);
		size3 = uxTaskGetStackHighWaterMark(loadDataTaskHandle);
		printf("task1 size:%d,task2 size:%d,task3 size:%d\n",size1,size2,size3);
		vTaskDelay(5000);
	}
}*/
