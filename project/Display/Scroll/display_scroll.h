/*
 * display_scroll.h
 *
 *  Created on: 2022年11月27日
 *      Author: literem
 */

#ifndef DISPLAY_SCROLL_H_
#define DISPLAY_SCROLL_H_

#include "dzCommon.h"

void scroll_head(void);
void scroll_end(void);
void scroll_content(void);
void scroll_end_to_head(void);
void increase_head(void);
void horizontalScroll(u8 start,u8 scroll_len);
#endif
