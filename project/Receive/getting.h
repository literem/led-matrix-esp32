/*
 * getting.h
 *
 *  Created on: 2023年1月27日
 *      Author: literem
 */

#ifndef PROJECT_RECEIVE_GETTING_H_
#define PROJECT_RECEIVE_GETTING_H_

#include "dzCommon.h"

u8* getDeviceInfo(u8 cmd3);
u8* getDisplayStatic(u8 cmd3);
u8* getDisplayScroll(u8 cmd3);
u8* getDisplayClock(u8 cmd3);

#endif /* PROJECT_RECEIVE_GETTING_H_ */
