/*
 * display_static.h
 *
 *  Created on: 2022年11月23日
 *      Author: literem
 */

#ifndef DISPLAY_STATIC_H_
#define DISPLAY_STATIC_H_

#include "dzCommon.h"

//anim_bottom_top  从底部滚动的动画
void anim_bottom_top_in(u8 pos,u8 len);
void anim_bottom_top_out(void);

//anim_top_bottom  从顶部滚动的动画
void anim_top_bottom_in(u8 pos,u8 len);
void anim_top_bottom_out(void);

//anim_middle_open  从中间展开的动画
void anim_middle_open_in(u8 pos,u8 len);
void anim_middle_open_out(void);

//anim_both_close  从两边向中间收缩的动画
void anim_both_close_in(u8 pos,u8 len);
void anim_both_close_out(void);

//anim_scroll_bottom 上下滚动的动画
void anim_bottom_to_top_scroll(u8 index,u8 len);

//anim_scroll_top 上下滚动的动画
void anim_top_to_bottom_scroll(u8 index,u8 len);

//anim_left_right  左右显示的动画
void anim_left_to_right_in(u8 pos,u8 len);
void anim_left_to_right_out(void);

//anim_right_left
void anim_right_to_left_out(void);

//无动画
void anim_none_in(u8 pos,u8 len);
void anim_none_out(void);

//垂直滚动
void setHorizontalScroll(u8 len);

//刷新一行
void refresh(u8 pos);

//计算显示位置
void calculateDisplayPosition(u8 start,u8 len);

#endif
