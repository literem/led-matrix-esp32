/*
 * i2c_devices.c
 *
 *  Created on: 2023年1月15日
 *      Author: literem
 */

#include "i2c_device.h"
#include "driver/gpio.h"


u8 I2CInit()
{
	/***************** 初始化I2C引脚 *****************/
	gpio_config_t cfg = {
		.pin_bit_mask = BIT64(SDA_PIN_NUM) | BIT64(SCL_PIN_NUM),
		.mode = GPIO_MODE_OUTPUT,
		.pull_up_en = 1,
		.pull_down_en = 0,
		.intr_type = GPIO_INTR_DISABLE,
	};
	gpio_config(&cfg);

	/***************** 初始化I2C *****************/
	i2c_config_t conf = {
		.mode = I2C_MODE_MASTER,
		.sda_io_num = SDA_PIN_NUM,
		.scl_io_num = SCL_PIN_NUM,
		.sda_pullup_en = GPIO_PULLUP_ENABLE,
		.scl_pullup_en = GPIO_PULLUP_ENABLE,
		.master.clk_speed = 100000     /* 标准模式(100 kbit/s) */
	};
	i2c_param_config(IIC_CTRL_NUM, &conf);
	return (i2c_driver_install(IIC_CTRL_NUM, conf.mode, 0, 0, 0) == ESP_OK);
}

