/*
 * bluetooth.h
 *
 *  Created on: 2022年11月22日
 *      Author: literem
 */

#ifndef PROJECT_BLUETOOTH_BLUETOOTH_H_
#define PROJECT_BLUETOOTH_BLUETOOTH_H_

#include "dzCommon.h"

void btInit(void);
void btSendData(u8 *dat,u8 len);
void btSendResponse(u8 *dat);
#endif /* PROJECT_BLUETOOTH_BLUETOOTH_H_ */
