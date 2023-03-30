/*
 * Receive.h
 *
 *  Created on: 2022年11月25日
 *      Author: literem
 */

#ifndef RECEIVE_H_
#define RECEIVE_H_

#include "dzCommon.h"

#define CMD_ERR	0
#define CMD_OK	1

#define END_BIT(rec,len)		(rec[len-2] | rec[len-1]) 	//结束位， 位Length-2，位Length-1
#define START_BIT(rec)			(rec[0]&rec[1])         	//起始位，位0，位1
#define RSP_H(rec)				rec[2]						//响应回去的数据（高位）
#define RSP_L(rec)				rec[3]						//响应回去的数据（低位）
#define CMD_LEN_BIT(rec)		((rec[4]<<8) + rec[5])    	//获取2字节整条的命令长度，高位4，低位5
#define CMD1_BIT(rec)			rec[6] 					  	//CMD1 位6
#define CMD2_BIT(rec)			rec[7] 					  	//CMD2 位7
#define CMD3_BIT(rec)			rec[8] 					  	//CMD3 位8
#define CMD_VALUE_BIT1(rec)		rec[9]						//CMD命令值（高位）
#define CMD_VALUE_BIT2(rec)		rec[10]						//CMD命令值（低位）
#define CMD_VALUE_1BIT(rec)		(rec[9]|rec[10])  	     	//获取1字节CMD命令值，高位：9，低位10
#define CMD_VALUE_2BIT(rec)		((rec[9]<<8) + rec[10])   	//获取2字节CMD命令值，高位：9，低位10
#define DATA_TYPE_BIT(rec)		rec[11] 			        //数据类型位(data-type)，位9，有三种类型：0xff(GBK) 0x0f(Font) 0x00(none)
#define DATA_BIT(rec)			rec[12]				     	//有效数据开始的位置
#define DATA_TYPE_LEN(len)  	(len-13)			     	//数据长度（包含data-type位），10个命令位+2个结束位
#define DATA_LEN(len)			(len-14)			     	//数据长度（不包含data-type位）

#define CHECK_TYPE_IS_GBK(recData)  {if(recData[11]!=DATA_TYPE_GBK)return CMD_ERROR;}
#define CHECK_TYPE_IS_FONT(recData) {if(recData[11]!=DATA_TYPE_FONT)return CMD_ERROR;}

void receiveHandle(u8 *recData,u16 len);

#endif
