/*
 * timer.c
 *
 *  Created on: 2022年12月7日
 *      Author: literem
 */
#include "Command.h"
#include "timer.h"
#include "display.h"
#include "display_toggle.h"
#include "display_clock.h"
#include "display_scroll.h"
#include "freertos/timers.h"

void time1Task(TimerHandle_t xTimer);
void (*timer1Function)(void);
u8 isTimerRunnig;
u8 timerState;
TimerHandle_t xTimer1 = NULL;

/**
 * 定时器初始化
 */
void timerInit()
{
	//创建一个定时器：名称、时间（1000ms）、运行次数（pdFALSE只运行一次，pdTRUE）、定时器ID（可为空）、回调函数
	xTimer1 = xTimerCreate("time1",50, pdTRUE, (void *)0,time1Task);
	timerState = STATE_NONE;
	isTimerRunnig = STOP;
}

/**
 * 开启定时器，若定时器已经开启，则设置时间
 */
void startTimer(u16 period)
{
	if(isTimerRunnig == STOP)
	{
		isTimerRunnig = RUN;
		xTimerStart(xTimer1,0);
		xTimerChangePeriod(xTimer1,period,100);
	}
	else
	{
		xTimerChangePeriod(xTimer1,period,0);
	}
}

/**
 * 关闭定时器
 */
void stopTimer()
{
	if(isTimerRunnig != RUN) return;//定时器不处于运行状态，无需关闭
	xTimerStop(xTimer1,0);
	timerState = STATE_NONE;
	timer1Function = NULL;
	isTimerRunnig = STOP;
}

/**
 * 设置定时器状态，需要提供时间
 */
void setTimerState(u8 state,u16 period)
{
	timerState = state;
	switch(state)
	{
		case STATE_SCROLL: timer1Function = increase_head;break;
		case STATE_TOGGLE: timer1Function = findToggleIndex;break;
		case STATE_FRAME:  timer1Function = fontFrame;break;
		case STATE_CLOCK:  timer1Function = clockCount;break;
		case STATE_NONE:   timer1Function = NULL;stopTimer();return;
		default: 		   timer1Function = NULL;break;
	}
	startTimer(period);
}


/**
 * 定时器任务
 */
void time1Task(TimerHandle_t xTimer)
{
	if(timer1Function == NULL)
		stopTimer();
	else
		timer1Function();
}

