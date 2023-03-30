/*
 * timer.h
 *
 *  Created on: 2022年12月7日
 *      Author: literem
 */

#ifndef PROJECT_TIMER_TIMER_H_
#define PROJECT_TIMER_TIMER_H_
#include "dzCommon.h"

#define RUN		1
#define STOP	0

extern u8 timerState;
extern void (*timer1Function)(void);

//#define checkTimerStop()		{if(timerState!=STATE_NONE) stopTimer();}

void timerInit(void);
void startTimer(u16 period);
void stopTimer(void);
void setTimerState(u8 state,u16 period);

#endif /* PROJECT_TIMER_TIMER_H_ */
