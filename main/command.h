/*
 * command.h
 *
 *  Created on: 2022年11月30日
 *      Author: literem
 */

#ifndef MAIN_COMMAND_H_
#define MAIN_COMMAND_H_

//返回的操作码
#define CMD_ERROR		0
#define VALUE_ERROR		1
#define WRITE_ERROR		2
#define RESPONSE_OK		100

//数据类型描述，GBK地址类型和字模数据类型
#define DATA_TYPE_GBK		0x0F
#define DATA_TYPE_FONT		0xF0

/*********** 第一级命令 ***********/
#define SET					0xA1	//设置命令
#define GET					0xA2	//获取数据命令

/*********** 第二级命令 ***********/
#define DISPLAY_NONE        0xB0	//无显示
#define DISPLAY_SCROLL      0xB1	//滚动显示
#define DISPLAY_STATIC      0xB2	//静态显示
#define DISPLAY_TOGGLE		0xB3	//自动切换显示
#define DISPLAY_CLOCK       0xB4	//时钟显示
#define DISPLAY_CUSTOM		0xB5	//自定义显示
#define MATRIX_DATA			0xB6	//设置点阵模块数据
#define DEVICE_INFO			0xB7	//获取设备信息
#define DISPLAY_SPEED		0xB8	//获取速度
#define DISPLAY_ANIM		0xB9	//获取动画

/*********** 第三级命令 ***********/
#define MODE				0xC0	//模式
#define SPEED				0xC1	//速度
#define ANIM_IN				0xC2	//进入动画
#define ANIM_OUT			0xC3	//退出动画
#define SET_RUN_ON_START	0xC4	//设置是否开机运行
#define DATETIME			0xC5	//设置时间
#define MODULE_SIZE			0xC6	//获取\设置模块数量
#define INFO_ALL			0xC7	//获取全部信息
#define DISPLAY_STATE		0xC8	//获取显示状态
#define CHARGE_STATE		0xC9	//获取电池充电状态
#define POWER_STATE			0xCA	//获取5v输入状态
#define SAVED_MODE			0xCB	//保存的显示模式
#define RUN_ON_START		0xCC	//

//静态显示下的移动操作
#define MOVE_HOR			0x10	//水平移动
#define MOVE_ANIM			0x20	//自定义动画滚动
#define MOVE_PAGE_UP		0x30	//上翻页滚动
#define MOVE_PAGE_DOWN		0x40	//下翻页滚动

//自定义显示
#define FONT_DATA			0x50	//自定义字模数据
#define FONT_FRAME_HEAD		0x51	//帧头
#define FONT_FRAME_PART		0x52	//帧内容
#define FONT_FRAME_END		0x53	//帧尾
#define FONT_FRAME_SPEED	0x54	//自定义帧动画播放速度

/*********** 动画值 ***********/
#define anim_scroll_bottom  0xE0 //进场动画独有
#define anim_scroll_top		0xE1 //进场动画独有
#define anim_left_right		0xE2 //进入、退出动画共有
#define anim_middle_open	0xE3 //进入、退出动画共有
#define anim_both_close     0xE4 //进入、退出动画共有
#define anim_bottom_top		0xE5 //进入、退出动画共有
#define anim_top_bottom		0xE6 //进入、退出动画共有
#define anim_right_left		0xE7 //退场动画独有

/*********** 显示状态 ***********/
#define STATE_NONE			0x00
#define STATE_SCROLL		0x01
#define STATE_TOGGLE		0x02
#define STATE_FRAME			0x03
#define STATE_CLOCK			0x04
#define STATE_RUN			0x05

#endif /* MAIN_COMMAND_H_ */
