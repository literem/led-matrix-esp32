/*
 * display_static.c
 *
 *  Created on: 2022年11月23日
 *      Author: literem
 */
#include "scanning.h"
#include "display.h"
#include "display_static.h"
#include "display_scroll.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include <stdio.h>
#define ANIM_TIME 3

#define TaskDelay()	vTaskDelay(5)

u8 startIndex=0,nextStartIndex=0;//字模数组开始位置
u8 endIndex=0,nextEndIndex=0;//加了点阵模块长度后的位置
u8 notDisplayColumns=0,nextNotDisplayColumns=0;//不显示数据的模块的数量

//动画持续时间
u8 ms;

/*
    函数功能：刷新一行
    函数说明：根据的位置pos，刷新某行的数据
*/
void refresh(u8 pos)
{
    HC138Scan(pos);
    if(notDisplayColumns) cleanData(notDisplayColumns);
    for(n=endIndex;n>=startIndex;n--) //点阵的字模数据要倒过来发，即最后显示的字最先发送
    {
        sendData(p[n][pos]);
    }
    CS595();
}

/*
    函数功能：刷新当前的 指定的一行数据
    函数说明：根据给定变量scan 和 pos，刷新某行的数据
                pos：点阵模块长度的n个数组中，每个数组的第pos个元素
                scan：某行
*/
void refresh_by_row(u8 scan,u8 pos)
{
    HC138Scan(scan);
    if(notDisplayColumns) cleanData(notDisplayColumns);
    for(n=endIndex;n>=startIndex;n--)
    {
        sendData(p[n][pos]);
    }
    CS595();
}

/*
    函数功能：刷新下一行字
    函数说明：无
*/
void refresh_next(u8 scan,u8 pos)
{
    HC138Scan(scan);
    if(nextNotDisplayColumns) cleanData(nextNotDisplayColumns);
    for(n=nextEndIndex;n>=nextStartIndex;n--)
    {
        sendData(p[n][pos]);
    }
    CS595();
}

/*
    函数功能：处理数组中的位置
    函数说明：根据开始位置计算出发送了一个点阵模块长度后 数组的索引是否越界
    函数参数：start --> 在数组中开始位置
              len --> 要显示的长度
*/
void calculateDisplayPosition(u8 start,u8 len)
{
    startIndex = start;
    if(len >= matrixData.moduleSize)//如果显示的长度超过点阵模块数量，则在能显示的范围内全部显示，没有不显示的
    {
        notDisplayColumns=0;
        endIndex = start + matrixData.moduleSize - 1;//实际开始位置是从最后一个字，所以当全部显示的时候，就是当前位置+点阵模块数量
    }
    else
    {
        notDisplayColumns = (matrixData.moduleSize - len) * 16;//显示的长度不超过点阵模块长度，计算出不显示的数量
        endIndex = start + len - 1;//是从最后一个字，所以当全部显示的时候，就是当前位置+显示长度
    }
}

/*
    函数功能：处理下一行字要显示的位置
    函数说明：无
*/
void calculateDisplayNextPosition(u8 start,u8 len)
{
    nextStartIndex = start;
    if(len >= matrixData.moduleSize)//如果显示的长度超过点阵模块数量，则在能显示的范围内全部显示，没有不显示的
    {
        nextNotDisplayColumns=0;
        nextEndIndex = start + matrixData.moduleSize - 1;//实际开始位置是从最后一个字，所以当全部显示的时候，就是当前位置+点阵模块数量
    }
    else
    {
        nextNotDisplayColumns = (matrixData.moduleSize - len) * 16;//显示的长度不超过点阵模块长度，计算出不显示的数量
        nextEndIndex = start + len - 1;//是从最后一个字，所以当全部显示的时候，就是当前位置+显示长度
    }
}

/*
    函数功能：从下往上滚动的进场动画
    函数说明：给定一个索引，计算出相应的数值，从底部滚动显示
*/
void anim_bottom_top_in(u8 pos,u8 len)
{
    calculateDisplayPosition(pos,len);
    for(i=15;i>=0;i--)
    {
        ms = ANIM_TIME;
        while(ms--)
        {
            for(j=i,k=0;j<=15;j++,k++)
            {
                refresh_by_row(j,k);
            }
            TaskDelay();
        }
    }
}

/*
    函数功能：从下往上滚动的退场动画
    函数说明：无
*/
void anim_bottom_top_out()
{
    for(i=0;i<16;i++)
    {
        ms = ANIM_TIME;
        while(ms--)
        {
            for(j=i,k=0;j<16;j++,k++) refresh_by_row(j,k);
            TaskDelay();
        }
    }
}

/*
    函数功能：从上往下滚动的进场动画
    函数说明：给定一个索引，计算出相应的数值，从顶部滚动显示
*/
void anim_top_bottom_in(u8 pos,u8 len)
{
    calculateDisplayPosition(pos,len);
    for(i=0,k=15;i<16;i++)
    {
        ms = ANIM_TIME;
        while(ms--)
        {
            for(j=0,k=15-i;j<=i;j++,k++) refresh_by_row(j,k);
            TaskDelay();
        }
    }
}

/*
    函数功能：从上往下滚动的退场动画
    函数说明：无
*/
void anim_top_bottom_out()
{
    s8 m=0;
    for(i=16,m=0;i>=0;i--,m++)
    {
        ms = ANIM_TIME;
        while(ms--)
        {
            for(j=0,k=m;j<i;j++,k++) refresh_by_row(j,k);
            TaskDelay();
        }
    }
}


/*
    函数功能：从中间展开的进场动画
    函数说明：
*/
void anim_middle_open_in(u8 pos,u8 len)
{
    calculateDisplayPosition(pos,len);
    for(i=8,k=9;i>0;i--,k++)
    {
        ms = ANIM_TIME;
        while(ms--)//每一帧动画的时间
        {
            for(j=i-1;j<k;j++) refresh(j);
            TaskDelay();
        }
    }
}

/*
    函数功能：从中间展开的退场动画
    函数说明：
*/
void anim_middle_open_out()
{
    for(i=8,j=8;i>0;i--,j++)
    {
        ms = ANIM_TIME;
        while(ms--)
        {
            for(k=0;k<i;k++)  refresh(k);
            for(k=j;k<16;k++) refresh(k);
            TaskDelay();
        }
    }
}

/*
    函数功能：从两边向中间闭合的进场动画
    函数说明：无
*/
void anim_both_close_in(u8 pos,u8 len)
{
    calculateDisplayPosition(pos,len);
    for(i=1,j=14;i<9;i++,j--)
    {
        ms = ANIM_TIME;
        while(ms--)
        {
            for(k=0;k<i;k++)  refresh(k);
            for(k=15;k>j;k--) refresh(k);
            TaskDelay();
        }
    }
}

/*
    函数功能：从两边向中间闭合的退场动画
    函数说明：无
*/
void anim_both_close_out()
{
    for(i=0,k=16;i<16;i++,k--)
    {
        ms = ANIM_TIME;
        while(ms--)
        {
            for(j=i;j<k;j++) refresh(j);
            TaskDelay();
        }
    }
}

/*
    函数功能：从左到右显示的动画
    函数说明：
*/
void anim_left_to_right_in(u8 pos,u8 len)
{
    u8 end_count;//,current_index;
    calculateDisplayPosition(pos,len);
    //current_index = startIndex;//当前数组索引
    end_count = matrixData.columnSize;//右边要发送不显示的列的数量，刚开始为点阵列的数量，即全部不显示，然后再递减
    for(i=startIndex;i<=endIndex;i++)//循环点阵模块数量次数，每循环一次索引+2
    {
        for(j=0;j<16;j+=2,end_count-=2)//每个模块有16列
        {
            for(k=0;k<16;k++)//逐个刷新，共16行
            {
                HC138Scan(k);
                cleanData(end_count);//再发送end_count位数据清空
                sendDataByHight(p[i][k],j);//把数组第current_index个元素的第j行发送，共发送j位数据
                if(i!=startIndex)//如果显示的不是第一个元素
                {
                    for(n=i-1;n>=startIndex;n--) sendData(p[n][k]);
                }
                CS595();
            }
        }
    }
}

/*
    函数功能：从左到右消失的动画
    函数说明：
*/
void anim_left_to_right_out()
{
    u8 start_count = 0;//每次加2，达到从左边到右边的消失
    //u8 end_count = (endIndex - startIndex) * 16;//计算右边是否有不显示的部分
    for(i=startIndex;i<=endIndex;i++)//点阵模块的数量，每刷完一个模块，加1
    {
        for(j=16;j>0;j-=2,start_count+=2)//每个模块有16列，每刷完一列，end_count加1
        {
            for(k=0;k<16;k++)//刷16行
            {
            	HC138Scan(k);
            	if(notDisplayColumns != 0) cleanData(notDisplayColumns);
            	for(n=endIndex;n>i;n--) sendData(p[n][k]);//从最后一个字发
                sendDataByLength(p[i][k],j);
                cleanData(start_count);
                CS595();
            }
        }
    }
}

/*
    函数功能：从右到左消失的动画
    函数说明：
*/
void anim_right_to_left_out()
{
    u8 end_count = notDisplayColumns;//计算右边是否有不显示的部分
    for(i=endIndex;i>=startIndex;i--)//点阵模块的数量，每刷完一个模块，current_index减1
    {
        for(j=16;j>0;j-=2,end_count+=2)//每个模块有16列，每刷完一列，end_count加2，加快显示速度
        {
            for(k=0;k<16;k++)//刷16行
            {
                HC138Scan(k);
                cleanData(end_count);
                sendDataByHight(p[i][k],j);
                for(n=i-1;n>=startIndex;n--) sendData(p[n][k]);
                CS595();
            }
        }
    }
}

/*
    函数功能：翻页动画 --> 从下往上滚动，滚动时保留上一行字的状态
    函数说明：下往上滚动
    注意事项：翻页没有退场动画
*/
void anim_bottom_to_top_scroll(u8 index,u8 len)
{
    calculateDisplayNextPosition(index,len);
    for(i=0;i<16;i++)//控制移动
    {
    	for(k=0,j=i;j<16;j++,k++) refresh_by_row(k,j);
		for(j=0;j<i;j++,k++)	  refresh_next(k,j);
		TaskDelay();
    }
    //移动完成后，把移动后所有的变量赋值给 当前要显示的变量上
    startIndex = nextStartIndex;
    endIndex = nextEndIndex;
    notDisplayColumns = nextNotDisplayColumns;
}

/*
    函数功能：翻页动画 --> 从上往下滚动的动画，滚动时保留上一行字的状态
    函数说明：上往下滚动
    注意事项：翻页没有退场动画
*/
void anim_top_to_bottom_scroll(u8 index,u8 len)
{
    calculateDisplayNextPosition(index,len);
    for(i=15;i>=0;i--)
    {
    	for(k=0,j=i;j<16;j++,k++) refresh_next(k,j);
		for(j=0;j<i;j++,k++) 	  refresh_by_row(k,j);
		TaskDelay();
    }

    //移动完成后，把移动后所有的变量赋值给 当前要显示的变量上
    startIndex = nextStartIndex;
    endIndex = nextEndIndex;
    notDisplayColumns = nextNotDisplayColumns;
}

/*
    函数功能：无进场动画显示
*/
void anim_none_in(u8 pos,u8 len)
{
    calculateDisplayPosition(pos,len);
}

/*
    函数功能：无退场动画显示
*/
void anim_none_out()
{
	vTaskDelay(500);
}

/*
    函数功能：设置垂直滚动
    函数参数：start --> 滚动多少个字
              len --> 显示的长度
*/
void setHorizontalScroll(u8 len)
{
	if((endIndex+len) >= (arrayData.fontIndex+arrayData.fontSize))
	{
		return;
	}
	horizontalScroll(endIndex,len);//开始从endIndex位置滚动len个长度
	startIndex+=len;
	endIndex+=len;
}
