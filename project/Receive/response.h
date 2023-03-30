/*
 * response.h
 *
 *  Created on: 2023年1月27日
 *      Author: literem
 */

#ifndef PROJECT_RECEIVE_RESPONSE_H_
#define PROJECT_RECEIVE_RESPONSE_H_
#include "dzCommon.h"

extern u8 responseData[30];
extern u8 responseCount;

#define RSP_NUM_HIGH_BIT		responseData[2] //响应ID（高位）
#define RSP_NUM_LOW_BIT			responseData[3] //响应ID（低位）
#define RSP_DATA_LEN			responseData[4] //响应的数据长度
#define RSP_STATE				responseData[5] //响应状态码
#define RSP_DATA_TYPE			responseData[6] //数据类型


void cleanResponseData(void);
void setResponseNumber(u8 highBit,u8 lowBit);
void setResponseDataType(u8 type);
void addResponseData(u8 flag,u16 data);
u8* createResponseData(u8 state);

#endif /* PROJECT_RECEIVE_RESPONSE_H_ */
