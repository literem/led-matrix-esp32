/*
 * scanning.h
 *
 *  Created on: 2022年11月21日
 *      Author: literem
 */

#ifndef SCANNING_H_
#define SCANNING_H_

#include "dzCommon.h"
#include "driver/gpio.h"
#include "esp32/rom/ets_sys.h"

/********* 引脚定义 ************/
#define A0_138	32
#define A1_138	33
#define A2_138	25
#define A3_138	26
#define OE_138	27
#define OE_595	14
#define CLK_595	12
#define CS_595	15
#define SER_595	2

#define SetGPIO(num,level)	gpio_set_level(num,level)

/********* 函数定义 ************/
void scanningInit(void);
void sendDataByLength(u16 dat,u8 n);
void sendDataByHight(u16 dat,u8 n);
void sendBitByHight(u8 dat,u8 n);
void send7BitByHight(u8 dat);
void sendData(u16 dat);
void HC138Scan(u8 rows);
void CS595(void);
void cleanData(u8 length);

#endif /* SCANNING_H_ */
