/*
 * key.c
 *
 *  Created on: 2023年1月16日
 *      Author: literem
 */

#include "key.h"
#include "display_clock.h"
#include "response.h"
#include "command.h"
#include "bluetooth.h"
#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"

//TaskHandle_t EXTtaskHandle;
void EXTtask(void *pvParm);
static u8 keyFlag,intFlag=0;
static xQueueHandle gpio_evt_queue = NULL;

/**
 * 外部触发的中断处理函数，该函数不可进行长时间运算
 */
void IRAM_ATTR ext_isr_handler(void *arg)
{
	/*u32 gpio_num = (u32) arg;
	xTaskNotifyFromISR(EXTtaskHandle,gpio_num,eSetValueWithOverwrite,NULL);*/
	u32 gpio_num = (u32) arg;
	if(gpio_num == INTA)
	{
		xQueueSendFromISR(gpio_evt_queue, &gpio_num, NULL);
	}
	else if(gpio_num == KEY1 || gpio_num == KEY2 || gpio_num == KEY3)
	{
		if(keyFlag == 0)
		{
			keyFlag=1;
			xQueueSendFromISR(gpio_evt_queue, &gpio_num, NULL);
		}
	}
	else
	{
		if(intFlag == 0)
		{
			intFlag = 1;
			xQueueSendFromISR(gpio_evt_queue, &gpio_num, NULL);
		}
	}
}

/**
 * 设置RX8025T芯片定时触发中断
 * state:0->关闭中断，1->打开中断
 */
/*void setINTA(u8 state)
{
	if(state) gpio_isr_handler_add(INTA, ext_isr_handler, (void*) INTA);
	else gpio_isr_handler_remove(INTA);
}*/

void buzzer()
{
	gpio_set_level(BUZZ,1);
	vTaskDelay(100);
	gpio_set_level(BUZZ,0);
	vTaskDelay(200);
	keyFlag = 0;
}


void keyInit()
{
	gpio_evt_queue = xQueueCreate(3, sizeof(uint32_t));

	//初始化按键等需要外部中断的IO口
	gpio_config_t io_conf = {
		.intr_type = GPIO_INTR_POSEDGE, // 低电平产生中断
		.pin_bit_mask = BIT64(KEY1)|BIT64(KEY2)|BIT64(KEY3)|BIT64(INTA),
		.mode = GPIO_MODE_INPUT, //输入模式
		.pull_up_en = 1 //上拉
	};
	gpio_config(&io_conf);

	io_conf.intr_type = GPIO_INTR_ANYEDGE;
	io_conf.pin_bit_mask = BIT64(IN_5V)|BIT64(CH_5V);
	gpio_config(&io_conf);

	//初始化蜂鸣器IO口
	io_conf.pin_bit_mask = BIT64(BUZZ);
	io_conf.pull_up_en = 0;
	io_conf.pull_down_en = 1;
	io_conf.intr_type = GPIO_INTR_DISABLE;
	io_conf.mode = GPIO_MODE_OUTPUT;
	gpio_config(&io_conf);

	gpio_install_isr_service(0);//install gpio isr service
	gpio_isr_handler_add(KEY1, ext_isr_handler, (void*) KEY1);//hook isr handler for specific gpio pin
	gpio_isr_handler_add(KEY2, ext_isr_handler, (void*) KEY2);
	gpio_isr_handler_add(KEY3, ext_isr_handler, (void*) KEY3);
	gpio_isr_handler_add(INTA, ext_isr_handler, (void*) INTA);
	gpio_isr_handler_add(IN_5V, ext_isr_handler, (void*) IN_5V);
	gpio_isr_handler_add(CH_5V, ext_isr_handler, (void*) CH_5V);
	xTaskCreatePinnedToCore(EXTtask,"EXTtask",1024*5,NULL,4,NULL,0);//加载字模任务
}

void sendPinState(u8 pin)
{
	u8 flag = (pin==IN_5V)?POWER_STATE:CHARGE_STATE;
	cleanResponseData();
	setResponseDataType(flag);
	addResponseData(flag,gpio_get_level(pin));
	u8 *dat = createResponseData(RESPONSE_OK);
	btSendResponse(dat);
	vTaskDelay(500);
	intFlag = 0;
}

/**
 * 外部中断处理任务
 */
void EXTtask(void *pvParm)
{
	u32 value;
	for(;;)
	{
		//xTaskNotifyWait(0x00, ULONG_MAX, &value, portMAX_DELAY);//没有通知时阻塞
		xQueueReceive(gpio_evt_queue, &value, portMAX_DELAY);
		switch(value)
		{
			case INTA:updateTime();break;
			case KEY1:buzzer();break;
			case KEY2:buzzer();break;
			case KEY3:buzzer();break;
			case IN_5V:sendPinState(IN_5V);break;
			case CH_5V:sendPinState(CH_5V);break;
		}
	}
}
