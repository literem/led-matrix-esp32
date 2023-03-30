/*
 * display.h
 *
 *  Created on: 2022年11月23日
 *      Author: literem
 */
#ifndef PROJECT_DISPLAY_DISPLAY_H_
#define PROJECT_DISPLAY_DISPLAY_H_
#include "dzCommon.h"
#include "scanning.h"
#include "freertos/FreeRTOS.h"
#include "freertos/message_buffer.h"

#define FONT_LEN     	120 //字模数组的长度，保存字模数据，实际字节数：FONT_LEN * 32
#define FONT_HALF_LEN	60	//保存两份不同的字模数据，以达到翻页切换的效果，这是下一个数据的起始位置
#define GBK_ARRAY_LEN	400 //GBK地址数组长度，保存GBK地址数据
#define RX_BUF_LEN	 	200 //接收缓冲区的数据长度，用于接收GBK地址数据，设置200，一次可接收100个字模地址的数据

//displayTask任务中刷新的状态
#define SHOW_STATIC		1
#define SHOW_SCROLL		2
#define SHOW_EXIT		100
#define SHOW_NONE		101

//定义进入和退出动画的函数指针
typedef void (*InAnimFunction)(unsigned char,unsigned char);
typedef void (*OutAnimFunction)(void);

typedef struct{
	u8 moduleSize;		//16*16点阵模块的数量
	u8 columnSize;		//全部点阵列的长度
	u8 displayStatus;	//显示状态
	u8 inAnim;			//进入动画
	u8 outAnim;			//退出动画
	u8 runOnStart;		//上电自动显示的状态（为0不显示，为DISPLAY_SCROLL会上电自动滚动）
	u8 saveDisplayMode;	//保存的显示模式
	u16 speedToggle;	//静态显示切换速度
	u16 speedScroll;	//滚动速度
	InAnimFunction entryAnimation;//进入动画函数指针
	OutAnimFunction exitAnimation;//退出动画函数指针
}MatrixData;

typedef struct{
	u8 gbkSize;		//GBK字模地址的长度
	u8 gbkIndex;	//GBK字模地址索引
	u8 fontIndex;	//字模数组当前索引（静态显示时为了不干扰，需要把接收的字模存到另一半当中）
	u8 fontSize;	//字模数组大小
	u16 fontColumnSize;//字模数组 有数据列的长度
}ArrayData;

typedef struct{
	u8 anim;		//动画
	u8 rowLength;	//总行数
	u8 length;		//字模地址长度
	u8 dataPos;		//数据索引
	u8 lenPos;		//行数长度索引
}ToggleData;

/***************** 全局变量声明 *******************/
//点阵屏幕数据的结构体
extern MatrixData matrixData;

//字模数组数据的结构体
extern ArrayData arrayData;

//切换信息的结构体
extern ToggleData toggleData;

//字模数组
extern u16 p[FONT_LEN][16];

//GBK数组
extern u8 gbkArrayData[GBK_ARRAY_LEN];

//显示状态
extern u8 showState;

//for循环用到的变量
extern s8 i,j,k,n;

//接收GBK地址的缓冲区
extern MessageBufferHandle_t MessageBufferHandle;

void displayInit(void);
void sendNotify(u8 notify);
void sendBuff(u8 *tx_buff,u8 buff_len);
void setShowState(u8 state);
#endif /* PROJECT_DISPLAY_DISPLAY_H_ */
