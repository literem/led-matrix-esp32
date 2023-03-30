/*
 * at24cxx.c
 *
 *  Created on: 2023年1月15日
 *      Author: literem
 */
#include "at24cxx.h"
#include "command.h"
#include <string.h>
#include <malloc.h>
#include "i2c_device.h"

#define AT24CXX_CHECK_VALUE		0x0F

/**
 * 写入一个字节的数据
 */
u8 at24cxxWrite(u16 addr,u8 data)
{
	vTaskDelay(20);
    i2c_cmd_handle_t cmd = i2c_cmd_link_create();
    i2c_master_start(cmd);
    i2c_master_write_byte(cmd, AT24CXX_ADDR, ACK_CHECK_EN);//发送写命令
    i2c_master_write_byte(cmd, addr>>8, ACK_CHECK_EN);//发送高地址
    i2c_master_write_byte(cmd, addr&0xff, ACK_CHECK_EN);//发送低地址
    i2c_master_write_byte(cmd, data, ACK_CHECK_EN);
    i2c_master_stop(cmd);
    esp_err_t ret = i2c_master_cmd_begin(IIC_CTRL_NUM, cmd, 200);
    i2c_cmd_link_delete(cmd);
    return (ret==ESP_OK) ? RESPONSE_OK : WRITE_ERROR;
}

/**
 * 读取一个字节的数据
 */
u8 at24cxxRead(u16 addr)
{
	u8 data = 0;
	vTaskDelay(20);
    i2c_cmd_handle_t cmd = i2c_cmd_link_create();
    i2c_master_start(cmd);
    i2c_master_write_byte(cmd, AT24CXX_ADDR, ACK_CHECK_EN);//发送写命令
    i2c_master_write_byte(cmd, addr>>8, ACK_CHECK_EN);//发送高地址
    i2c_master_write_byte(cmd, addr&0xff, ACK_CHECK_EN);//发送低地址
    i2c_master_start(cmd);
    i2c_master_write_byte(cmd, AT24CXX_READ_ADDR, ACK_CHECK_EN);
    i2c_master_read_byte(cmd, &data, NACK_VAL);
    i2c_master_stop(cmd);
    i2c_master_cmd_begin(IIC_CTRL_NUM, cmd, 200);
    i2c_cmd_link_delete(cmd);
    return data;
}

/**
 * 写入多个字节的数据
 * addr:写入的开始地址
 * *buf:写入数据的内容
 * len:写入长度
 */
u8 writeMultipleData(u8* dat,u16 len)
{
	bool result = true;
	u8 *buf = (u8*)malloc(34);//开辟32+2字节空间
	u8 totalPage = len / 32;//每页32字节，计算要写入多少页
	u8 remainder = len % 32;//剩余字节
	u8 datPos = 0;//要写入的数组当前的索引，每写入一页，索引+=32
	u16 addr = ER_DATA_ADDR;//开始地址
	if(buf == NULL) return WRITE_ERROR;
	for(u8 i=0;i<totalPage;i++,addr+=32,datPos+=32)//循环写入totalPage*32字节
	{
		buf[0] = addr >> 8;
		buf[1] = addr & 0xff;
		memcpy(&buf[2],&dat[datPos],32);
		if(i2c_master_write_to_device(I2C_NUM_0,0x50,buf,34,200) != ESP_OK)
		{
			result = false;
			break;
		}
		vTaskDelay(20);
	}

	//再写入剩余的字节
	if(remainder > 0 && result == true)
	{
		buf[0] = addr >> 8;
		buf[1] = addr & 0xff;
		memcpy(&buf[2],&dat[datPos],remainder);
		vTaskDelay(20);
		if(i2c_master_write_to_device(I2C_NUM_0,0x50,buf,remainder+2,200) != ESP_OK) result = false;
	}
	free(buf);
	if(result == true)
	{
		return at24cxxWrite2Byte(ER_DATA_LEN,len);
	}

	return WRITE_ERROR;
}

/**
 * 读取多个字节的数据
 * addr:读取的开始地址
 * *buf:读取到的数据缓冲区指针
 * len:读取数据的长度
 */
u8 readMultipleData(u8 *buf,u16 len)
{
	esp_err_t ret;
	u8 *addrBuff = (u8*)malloc(2);
	u16 addr = ER_DATA_ADDR;//开始地址
	addrBuff[0] = addr >> 8;
	addrBuff[1] = addr & 0xff;
	ret = i2c_master_write_read_device(I2C_NUM_0, 0x50, addrBuff, 2, buf, len, 200);
	free(addrBuff);
	vTaskDelay(100);
	return (ret == ESP_OK);
}

/**
 * 写入两字节数据，地址会自动增加1，确保开始地址的下一个地址不会被其他数据占用
 */
u8 at24cxxWrite2Byte(u16 addr,u16 data)
{
	if(at24cxxWrite(addr,data >> 8) == WRITE_ERROR) return WRITE_ERROR;//写入高位
	return at24cxxWrite(addr+1,data & 0xff);//写入低位
}

/**
 * 读取1字节的数据并检查数据是否超过给定的阈值，若超过使用默认值，返回读取后的值
 * lower:最低阈值
 * upper:最高阈值
 * defaultValue:默认数据
 * addr:读取数据的地址
 * return：返回读取后的值
 */
u8 loadDataAndCheck(u8 lower,u8 upper,u8 defaultValue,u8 addr)
{
	u8 target = at24cxxRead(addr);
	if(target < lower || target > upper)//如果值低于下限或高于上限，则设置默认值
	{
		target = defaultValue;
	}
	return target;
}

/*
 * 读取2字节数据并检查数据是否超过给定的阈值，若超过使用默认值
 * parameter:见loadDataAndCheck函数的注释
 * return：返回读取后2字节的值
 */
u16 load2ByteDataAndCheck(u16 lower,u16 upper,u16 defaultValue,u8 addr)
{
	u16 target;
	target = at24cxxRead(addr);
	target <<= 8;//设置高位
	target += at24cxxRead(addr+1);//设置低位
	if(target < lower || target > upper)//如果值低于下限或高于上限，则设置默认值
	{
		target = defaultValue;
	}
	return target;
}

/**
 * 初始化AT24CXX
 * return:0->成功，1->失败
 */
u8 at24cxxInit()
{
	if(at24cxxRead(ER_CHECK_ADDR) == AT24CXX_CHECK_VALUE)//若读取地址0的数据是AT24CXX_CHECK_VALUE，说明器件正常
		return 0;
	//读取不到数据，就写入数据再读取，若再次读取不正确，则器件不正常
	at24cxxWrite(0,AT24CXX_CHECK_VALUE);
	if(at24cxxRead(ER_CHECK_ADDR) == AT24CXX_CHECK_VALUE)//再次读取并判断
		return 0;
	return 1;
}

