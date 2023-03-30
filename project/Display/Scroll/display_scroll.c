/*
 * display_scroll.c
 *
 *  Created on: 2022年11月27日
 *      Author: literem
 */

#include "display_scroll.h"
#include "display.h"
#include "receive.h"
#include "timer.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include <stdio.h>

/**** 滚动所需的变量 *****/
u16 currPos=0;//当前滚动的位置
u8  currIndex=0;//当前显示的数组的索引
u16 endPos;//最终的位置（数组列+点阵列）
u8  endSize;//末尾的数组长度
u8  endNumber=0;
u8  headNumber=0;
u8  next_array_index;
u8  isLoop = 0;


/*
    函数功能：滚动尾部时position增加，并判断尾部数据是否显示完毕
    函数使用：定时器调用
*/
void increase_end()
{
    currPos++;//当前滚动的索引
    if(currPos == endPos){//滚到末尾结束
    	isLoop=0;
    }else{
        endNumber++;//（填0）列的数量+1
        headNumber = currPos % 16;//算出前面的补偿列的数量（即剩下的字模数量）
        if(headNumber==0){//如果滚动了16列（这列16被0填充了）
            endSize--;//数组索引-1
        }
        headNumber = 16 - headNumber;//点阵头部分=16-补偿列的数量
    }
}

/*
    函数功能：滚动尾部连接头部时position增加，并判断该数据是否显示完毕
    函数说明：使得滚动数据的头尾相连，达到循环效果
    函数使用：定时器调用
*/
void increase_end_to_head()
{
    currPos++;
    if(currPos == matrixData.columnSize){//滚到末尾结束
        currIndex = next_array_index;
        isLoop=0;
    }else{
    	endNumber = currPos%16;//算出新一行的补偿列的数量
        if(endNumber==0){//如果滚动了16列（即到下一个字模数据了）
            endSize--;//数组索引-1
            next_array_index++;//下一轮的长度+1
        }
        headNumber = 16 - endNumber;//点阵头部分=16-补偿列的数量
    }
}

/*
    函数功能：滚动头部时position增加，并判断头部数据是否显示完毕
    函数使用：定时器调用
*/
void increase_head()
{
    currPos++;//滚动的索引
    if(currPos == matrixData.columnSize){//如果索引等于点阵列的数量，头部显示完毕
    	isLoop=0;
    }else{
    	endNumber = currPos%16;//求余数，得到当前数组要发送的位置
        if(endNumber==0)//如果发送数据的位置为0，说明滚动到下一个字了
        	currIndex++;//数组索引增加
        headNumber = matrixData.columnSize - endNumber - currIndex*16;//算出补偿前面不显示的数。点阵列的数量-当前发送的位置-p索引*16
    }
}

/*
    函数功能：滚动内容时position增加，并判断内容数据是否显示完毕
    函数使用：定时器调用
*/
void increase_content()
{
    currPos++;
    if(currPos == arrayData.fontColumnSize){//滚动到数组最后一个元素
    	isLoop=0;
    }else{
        endNumber = currPos%16;//求余数，得到当前数组要发送的位置
        headNumber = 16 - endNumber;//算出补偿数
        if(endNumber == 0){//如果发送数据的位置为1，数组索引增加
        	currIndex++;
        }
    }
}

//进入这个函数，currIndex=最后一个数组的长度
/*
    函数功能：把数组头和数组尾的数据 截取并显示到点阵上
    函数说明：当滚动到数组尾部，会从头部拿出部分数据显示到点阵的右边上，直到拿出的头部数据 滚动到点阵列的长度时退出
*/
void scroll_end_to_head()
{
	if(showState != 2) return;
	timer1Function = increase_end_to_head;
    endSize = matrixData.moduleSize - 1;//尾部剩余要显示的字，每过16列-1，因为num_head独自发送，所以-1
    next_array_index = 0;//把索引置0   指针每过16列，下一轮数据长度+1
    currPos = endNumber = 1;
    headNumber = 15;
    isLoop=1;
    while(isLoop && showState==2)
    {
        for(i=0;i<16;i++)
        {
            HC138Scan(i);
            //发新一轮数组的数据
            sendDataByHight(p[next_array_index][i],endNumber);//最先的不完整（不够16列）的数据在此发送
            for(j=next_array_index;j>0;j--) sendData(p[j-1][i]);//如果next_array_index>1，发送完整的16列
            //根据点阵模块数量-1，发中间剩余的数据
            for(j=0;j<endSize;j++) sendData(p[currIndex-j][i]);
            sendDataByLength(p[currIndex-endSize][i],headNumber);//发送头部的数据

            CS595();
        }
        vTaskDelay(5);
    }
}

/**
   函数功能：滚动开始 逐渐进入的效果
   函数说明：根据position计算前面要填充0的数量，如果position等于点阵列的数量，滚动退出
**/
void scroll_head()
{
	if(showState != SHOW_SCROLL) return;
	timer1Function = increase_head;
    currIndex = currPos = 0;
    increase_head();//第一次使用先对索引和相关数据初始化，之后由定时器调用
    isLoop=1;
    while(isLoop && showState==SHOW_SCROLL)
    {
        for(i=0;i<16;i++)
        {
            HC138Scan(i);
            //发送开始的数据
            sendDataByHight(p[currIndex][i],endNumber);
            //发送中间部分数据
            //如果position没有超过一个字（16长度），不会发送
            //如果超过16，则要发送超过的部分
            for(j=0;j<currIndex;j++) sendData(p[currIndex-j-1][i]);
            //根据num_head（dz_lie_count - position）的长度，对左边进行 清空不显示
            cleanData(headNumber);
            CS595();
        }
        vTaskDelay(5);
    }
}

/*
    函数功能：正常的滚动
    函数说明：该函数会从N个数组索引的长度开始继续滚动，直到数组结尾才退出
    一般是接着scroll_head()后面，即当上一个函数滚动了N个数组索引的长度后，才会使用
*/
void scroll_content()
{
	if(showState != SHOW_SCROLL) return;
	timer1Function = increase_content;
	endNumber = 0;
    headNumber = 16;//一定要赋值，否则会出现滚动不连续的问题
    currIndex++;
    isLoop=1;
    while(isLoop && showState==SHOW_SCROLL)
    {
        for(i=0;i<16;i++)
        {
            HC138Scan(i);
            sendDataByHight(p[currIndex][i],endNumber);//发送右边的数，从高位开始发送，发送num_end个长度
            //根据点阵模块的数量，发送中间的数据，去除左边剩余的数要单独发，故要从i=1开始，发送dz_modules_size-1遍
            for(j=1;j<matrixData.moduleSize;j++) sendData(p[currIndex-j][i]);
            //再发送左边剩余的数，因为数据到下一列时p_index加1，要拿到左边剩余的数的位置，要减去模块的数量（dz_modules_size）
            sendDataByLength(p[currIndex-matrixData.moduleSize][i],headNumber);
            CS595();
        }
        vTaskDelay(5);
    }
}

/**
   函数功能：滚动完成后逐渐退出的效果
   函数说明：根据点阵列的长度，对右边填充0进行滚动
**/
void scroll_end()
{
	if(showState != SHOW_SCROLL) return;
	endNumber = 0;
	headNumber = 16;//一定要赋值，否则会出现滚动不连续的问题
    endPos = arrayData.fontColumnSize + matrixData.columnSize;//实际position的长度 = 数组的列+点阵的列
    endSize = matrixData.moduleSize-1;//尾部剩余要显示的字，每过16列-1，因为num_head独自发送，所以-1
    timer1Function = increase_end;
    isLoop=1;
    while(isLoop && showState==SHOW_SCROLL)
    {
        for(i=0;i<16;i++)
        {
            HC138Scan(i);
            cleanData(endNumber);//先发 num_end 个长度的不显示的数据
            for(j=0;j<endSize;j++) sendData(p[currIndex-j][i]);
            sendDataByLength(p[currIndex-endSize][i],headNumber);//发送头部的数据
            CS595();
        }
        vTaskDelay(5);
    }
}

/**
 * 函数功能： 滚动一段距离
 * start:在数组开始的位置
 *
 */
void horizontalScroll(u8 start,u8 scroll_len)
{
    currIndex = start+1;
    currPos = 0;
    endPos = scroll_len * 16;
    while(1)
    {
        currPos++;
        if(currPos == endPos) break;//滚动到数组最后一个元素

        endNumber = currPos%16;//求余数，得到当前数组要发送的位置
        headNumber = 16 - endNumber;//算出补偿数
        if(endNumber == 0) currIndex++;//如果发送数据的位置为1，数组索引增加

        for(i=0;i<16;i++)
        {
            HC138Scan(i);

            sendDataByHight(p[currIndex][i],endNumber);//发送右边的数，从高位开始发送，发送num_end个长度

            //根据点阵模块的数量，发送中间的数据，去除左边剩余的数要单独发，故要从i=1开始，发送dz_modules_size-1遍
            for(j=1;j<matrixData.moduleSize;j++) sendData(p[currIndex-j][i]);

            //再发送左边剩余的数，因为数据到下一列时p_index加1，要拿到左边剩余的数的位置，要减去模块的数量（dz_modules_size）
            sendDataByLength(p[currIndex-matrixData.moduleSize][i],headNumber);

            CS595();
        }
        vTaskDelay(5);
    }
}


