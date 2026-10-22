/**
 * @file Delay.c
 * @brief 软件阻塞延时模块源文件
 * @details 采用三重循环精准校准，1T 模式 @11.0592MHz 产生精准毫秒级延时
 */

#include "Delay.h"
#include <intrins.h>

void Delay(unsigned int xms)	//@11.0592MHz
{
	unsigned char data i, j;
	while (xms--)
	{
		_nop_();
		i = 11;
		j = 190;
		do
		{
			while (--j);
		} while (--i);
	}
}
