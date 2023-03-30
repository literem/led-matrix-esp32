/*
 * key.h
 *
 *  Created on: 2023年1月16日
 *      Author: literem
 */

#ifndef PROJECT_KEY_KEY_H_
#define PROJECT_KEY_KEY_H_

#include "dzCommon.h"
#include "driver/gpio.h"

#define KEY1	5
#define KEY2	16
#define KEY3	17
#define BUZZ	4
#define INTA	18
#define IN_5V	34	//输入5V检测
#define CH_5V	35	//充电检测

void keyInit(void);

#endif /* PROJECT_KEY_KEY_H_ */
