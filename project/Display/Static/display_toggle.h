/*
 * display_toggle.h
 *
 *  Created on: 2022年12月4日
 *      Author: literem
 */

#ifndef DISPLAY_TOGGLE_H_
#define DISPLAY_TOGGLE_H_

#include "dzCommon.h"

u8 initAutoToggle(u8 *dat,u16 len,u8 rowLen,u8 anim,u8 isSave);
u8 initAutoToggleFromER(u16 dataLen);
void findToggleIndex(void);

u8 setFrameStart();
u8 addFramePart(u8 *dat,u16 dataLen);
u8 setFrameEnd(u8 moduleIndex);
void fontFrame(void);
#endif
