/*
 * display_clock.h
 *
 *  Created on: 2023年1月8日
 *      Author: literem
 */

#ifndef DISPLAY_CLOCK_H_
#define DISPLAY_CLOCK_H_
#include "dzCommon.h"

#define CLOCK_HM	1
#define CLOCK_HMS	2

typedef struct{
	u8 year;
	u8 month;
	u8 day;
	u8 week;
	u8 hour;
	u8 minute;
	u8 second;
}DateTime;

extern u8 clockMode;

void displayTime(void);
void updateTime(void);
void clockCount(void);
u8 initDisplayClock(u8 mode);
u8 setClockDateTime(u8 *data,u8 len);

#endif
