/*
 * scanning.c
 *
 *  Created on: 2022年11月21日
 *      Author: literem
 */
#include "scanning.h"
#include <esp_types.h>


#define CLOCK_595()		{SetGPIO(CLK_595,1);dat>>=1;SetGPIO(CLK_595,0);}

u8 send_i;
u8 di;

void delay()
{
	di=20;
	while(di--);
}

void scanningInit()
{
	gpio_config_t cfg = {
	        .pin_bit_mask = BIT64(SER_595) | BIT64(CS_595) | BIT64(CLK_595) |
							BIT64(A0_138)  | BIT64(A1_138) | BIT64(A2_138)  |
							BIT64(A3_138)  | BIT64(OE_138) | BIT64(OE_595)  ,
	        .mode = GPIO_MODE_OUTPUT,
	        .pull_up_en = 1,
	        .pull_down_en = 0,
	        .intr_type = GPIO_INTR_DISABLE,
	    };
	    gpio_config(&cfg);
	    SetGPIO(OE_138,0);
	    SetGPIO(OE_595,1);
}


/*
	函数功能：发送N bit数据
	函数说明：根据给定的长度，从低位开始发送数据，一共n bit
*/
void sendDataByLength(u16 dat,u8 n)
{
    for(send_i=0; send_i<n; send_i++)
    {
        SetGPIO(SER_595,!(dat & 0x0001));
        CLOCK_595();
    }
}

/*
	函数功能：发送16bit数据
	函数说明：根据给定的长度n，从高位截取n个bit发送

	例如：
	数据0xff要从高位发送5 bit，先右移（8-5）bit，得：
	1010 1101  >>3 = 000 10101
	因为是从高位发，要把右边的数移除，这样要发送的数就在低位了，再从低位发n个长度
*/
void sendDataByHight(u16 dat,u8 n)
{
    dat >>= (16-n);
    for(send_i=0; send_i<n; send_i++)
    {
    	SetGPIO(SER_595,!(dat & 0x0001));
    	CLOCK_595();
    }
}

/*
    函数功能：从高位开始，发送8bit数据，共发送n位
    函数说明：dat：8bit数据
    		   n：发送个数
*/
void sendBitByHight(u8 dat,u8 n)
{
    dat >>= (8-n);
    for(send_i=0; send_i<n; send_i++)
    {
        SetGPIO(SER_595,!(dat&0x01));
        CLOCK_595();
    }
}

void send7BitByHight(u8 dat)
{
    dat >>= 1;
    for(send_i=0; send_i<7; send_i++)
    {
        SetGPIO(SER_595,!(dat&0x01));
        CLOCK_595();
    }
}

/*
    函数功能：发送16bit数据
    函数说明：从低位发送长度为16bit的数据
*/
void sendData(u16 dat)
{
    for(send_i=0;send_i<16;send_i++)
    {
        SetGPIO(SER_595,!(dat & 0x0001));
        CLOCK_595();
    }
}

/*
	函数功能：清空数据
	函数说明：根据给定长度，发送全1，清屏
*/
void cleanData(u8 length)
{
    for(send_i=0;send_i<length;send_i++)
    {
        SetGPIO(SER_595,1);
        SetGPIO(CLK_595,1);
        delay();
        SetGPIO(CLK_595,0);
    }
}

void CS595()
{
	SetGPIO(CS_595,1);
	delay();
	SetGPIO(CS_595,0);
	//SetGPIO(OE_138,0);
	SetGPIO(OE_595,0);
	ets_delay_us(750);
	//SetGPIO(OE_138,1);
	SetGPIO(OE_595,1);
}

void HC138Scan(u8 rows)
{
	SetGPIO(A0_138, rows	&0x01);
	SetGPIO(A1_138,(rows>>1)&0x01);
	SetGPIO(A2_138,(rows>>2)&0x01);
	SetGPIO(A3_138,(rows>>3)&0x01);
}

