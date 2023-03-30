/*
 * rx8025t.c
 *
 *  Created on: 2023年1月15日
 *      Author: literem
 */

#include "rx8025t.h"
#include "display_clock.h"
#include "i2c_device.h"

#define HexToDec(num)	(num>>4)*10 + (num&0x0f)	/* 16进制转10进制换算函数用于时钟输出 */
#define DecToHex(num)	((num/10)<<4) | (num%10)	/* 10进制转16进制换算函数用于时钟设置 */

/**
 * rx8025t写数据 (地址,数据)
 * addr:地址
 * date:数据
 */
void rx8025tWriteData(u8 addr,u8 date)
{
	i2c_cmd_handle_t cmd = i2c_cmd_link_create();
	i2c_master_start(cmd);
	i2c_master_write_byte(cmd, 0x64, ACK_VAL);//指定总线上的器件(写给谁)
	i2c_master_write_byte(cmd, addr, ACK_VAL);//指定该器件中的数据寄存器地址(写到哪里)
	i2c_master_write_byte(cmd, date, ACK_VAL);//向指定的地址写入数据(写什么)
	i2c_master_stop(cmd);
	i2c_master_cmd_begin(IIC_CTRL_NUM, cmd, 0);
	i2c_cmd_link_delete(cmd);
}

/**
 * rx8025t读数据
 * addr:地址
 */
u8 rx8025tReadData(u8 addr)
{
	u8 date;
	i2c_cmd_handle_t cmd = i2c_cmd_link_create();
	i2c_master_start(cmd);
	i2c_master_write_byte(cmd, 0x64, 0);//指定器件（读谁的数据）
	i2c_master_write_byte(cmd, addr, 0);//指定数据的位置（在哪里读）<<4|0x01
	i2c_master_start(cmd);
	i2c_master_write_byte(cmd, 0x65, 0);//使器件进入读出状态
	i2c_master_read_byte(cmd, &date, 1);//读出数据
	i2c_master_stop(cmd);
	i2c_master_cmd_begin(IIC_CTRL_NUM, cmd, 0);
	i2c_cmd_link_delete(cmd);
	return date;
}

/**
 * 读取时间：年月日周时分秒
 */
void rtcGetDateTime(DateTime *datetime)
{
	//分7次读取 秒分时日月周年
	datetime->second = rx8025tReadData(0) & 0x7f;
	datetime->second = HexToDec(datetime->second);
	datetime->minute = rx8025tReadData(1) & 0x7f;
	datetime->minute = HexToDec(datetime->minute);
	datetime->hour = rx8025tReadData(2) & 0x3f;
	datetime->hour = HexToDec(datetime->hour);
	datetime->week = rx8025tReadData(3) & 0x7f;
	datetime->week = HexToDec(datetime->week);
	datetime->day = rx8025tReadData(4) & 0x3f;
	datetime->day = HexToDec(datetime->day);
	datetime->month = rx8025tReadData(5)& 0x1f;
	datetime->month = HexToDec(datetime->month);
	datetime->year = rx8025tReadData(6);
	datetime->year = HexToDec(datetime->year);
}

/**
 * 向RX8025写入时间，年月日周时分秒
 */
void rtcSetDateTime(DateTime *datetime)
{
	rx8025tWriteData(0,DecToHex(datetime->second));
	rx8025tWriteData(1,DecToHex(datetime->minute));
	rx8025tWriteData(2,DecToHex(datetime->hour));
	rx8025tWriteData(3,DecToHex(datetime->week));
	rx8025tWriteData(4,DecToHex(datetime->day));
	rx8025tWriteData(5,DecToHex(datetime->month));
	rx8025tWriteData(6,DecToHex(datetime->year));
}

/**
 * 获取当前秒
 */
u8 rtcGetSecond()
{
	u8 data = rx8025tReadData(RX8025T_REG_SECOND) & 0x7f;
	return HexToDec(data);
}

/**
 * 获取当前分钟
 */
u8 rtcGetMinute()
{
	u8 data = rx8025tReadData(RX8025T_REG_MINUTE) & 0x7f;
	return HexToDec(data);
}

/**
 * 获取当前小时
 */
u8 rtcGetHour()
{
	u8 data = rx8025tReadData(RX8025T_REG_HOUR) & 0x3f;
	return HexToDec(data);
}

/**
 * 开启更新中断，
 * mode:为1时1分钟产生一次中断，为2时1秒钟产生一次中断
 */
void rx8025tSetINTA(u8 mode)
{
	//设置寄存器0D的USEL位
	if(mode == CLOCK_HM)
		rx8025tWriteData(0x0D,RX8025T_EXTEN_USEL);//寄存器位5写入1：产生分中断
	else if(mode == CLOCK_HMS)
		rx8025tWriteData(0x0D,0);//寄存器位5写入0:产生秒中断
	else return;
	rx8025tWriteData(0x0F,0x20);//设置0F寄存器的UIE位，开启更新中断
}

/**
 * 关闭更新中断
 */
void rx8025tResetINTA()
{
	rx8025tWriteData(0x0F,0x00);
}

/**
 * 初始化RX8025T
 */
void rx8025tInit()
{
	u8 VLF = rx8025tReadData(0x0E);//读取VLF位的值，当读到1时说明掉电了，需要将寄存器进行初始化操作
	rx8025tWriteData(0x0F,0x00);//清除中断标志位
	if((VLF&0x02)==0x02)//如掉电VLF标志被置位为1、计时数据将丢失，初始化时间
	{
		DateTime *datetime = (DateTime *) malloc(sizeof(DateTime));
		datetime->year=23;datetime->month=1;datetime->day=1;datetime->week=7;
		datetime->hour=12;datetime->minute=0;datetime->second=0;
		rtcSetDateTime(datetime);//初始化时间为2023-1-1 12:00:00 星期日
		free(datetime);
	}
}
