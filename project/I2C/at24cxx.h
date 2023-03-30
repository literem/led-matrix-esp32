/*
 * at24cxx.h
 *
 *  Created on: 2023年1月15日
 *      Author: literem
 */

#ifndef PROJECT_I2C_AT24CXX_H_
#define PROJECT_I2C_AT24CXX_H_
#include "dzCommon.h"
#define AT24CXX_ADDR		0xA0	//写地址（器件地址）
#define AT24CXX_READ_ADDR	0xA1	//读地址（写地址+1）

//一些点阵数据的地址
#define ER_CHECK_ADDR	0 //默认第0个地址为检查器件地址
#define ER_MODULE_SIZE 	1 //模块大小
#define ER_IN_ANIM		2 //进入动画
#define ER_OUT_ANIM		3 //退出动画
#define ER_RUN_START	4 //是否开机自启
#define ER_SPEED_TOGGLE	5 //5、6 切换速度（两个字节）
#define ER_SPEED_SCROLL	7 //7、8 滚动速度（两个字节）
#define ER_DISPLAY_MODE 9 //保存的是什么数据

#define ER_TOGGLE_LEN	25
#define ER_TOGGLE_ANIM	26


#define ER_DATA_LEN		30	//30、31 保存的数据长度
#define ER_DATA_ADDR	32	//保存的数据起始地址

u8 at24cxxInit(void);
u8 at24cxxRead(u16 addr);
u8 at24cxxWrite(u16 addr,u8 data);
u8 readMultipleData(u8 *buf,u16 len);
u8 writeMultipleData(u8 *buf,u16 len);
u8 at24cxxWrite2Byte(u16 addr,u16 data);
u8 loadDataAndCheck(u8 lower,u8 upper,u8 defaultValue,u8 addr);
u16 load2ByteDataAndCheck(u16 lower,u16 upper,u16 defaultValue,u8 addr);
#endif /* PROJECT_I2C_AT24CXX_H_ */
