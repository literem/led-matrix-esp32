/*
 * i2c_devices.h
 *
 *  Created on: 2023年1月15日
 *      Author: literem
 */

#ifndef PROJECT_I2C_I2C_DEVICES_H_
#define PROJECT_I2C_I2C_DEVICES_H_
#include "dzCommon.h"
#include "driver/i2c.h"


#define SDA_PIN_NUM 	21              /*!< gpio number for I2C data  */
#define SCL_PIN_NUM 	22              /*!< gpio number for I2C clock */
#define ACK_VAL 		0x0             /*!< I2C ack value  */
#define NACK_VAL 		0x1             /*!< I2C nack value */
//#define WRITE_BIT 	0x00            /*!< I2C master write */
//#define READ_BIT 		0x01            /*!< I2C master read  */
#define ACK_CHECK_EN 	0x1             /*!< I2C master will check ack from slave     */
#define ACK_CHECK_DIS 	0x0             /*!< I2C master will not check ack from slave */
#define IIC_CTRL_NUM 	I2C_NUM_0       /*!< I2C port number */

u8 I2CInit(void);

#endif /* PROJECT_I2C_I2C_DEVICES_H_ */
