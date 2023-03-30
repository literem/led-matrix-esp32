/*
 * setting.h
 *
 *  Created on: 2022年11月26日
 *      Author: literem
 */

#ifndef PROJECT_RECEIVE_SETTING_H_
#define PROJECT_RECEIVE_SETTING_H_
#include "dzCommon.h"

u8 writeRunOnStart(u8 displayMode);
u8 setSpeed(u8 type,u16 speed);
void setAnimation(u8 type,u8 value);
u8 setMatrixData(u8 cmd3,u8 value);
void copyFontData(u8 *dat,u8 pos,u16 len);

u8 setDisplayCustom(u8 cmd3,u8 *recData,u16 len);
u8 setDisplayScroll(u8 cmd3,u8 *recData,u16 len);
u8 setDisplayStatic(u8 cmd3,u8 *recData,u16 len);
u8 setDisplayToggle(u8 cmd3,u8 *recData,u16 len);
u8 setDisplayClock(u8 cmd3,u8 *recData,u16 len);

#endif /* PROJECT_RECEIVE_SETTING_H_ */
