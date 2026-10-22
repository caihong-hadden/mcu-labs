/**
 * @file timer0.c
 * @brief 定时器0模块源文件
 * @details 采用 STC15 专属 1T 高速定时器模式，产生 1ms 基准中断
 */

#include "timer0.h"
#include <STC15F2K60S2.H>

void Timer0_Init(void)		// 1毫秒@11.0592MHz (1T 模式)
{
	AUXR |= 0x80;			// 定时器0时钟设为 1T 模式 (速度提升 12 倍)
	TMOD &= 0xF0;			// 模式0 (16位自动重装载)
	TL0 = 0xCD;				// 定时初值低字节
	TH0 = 0xD4;				// 定时初值高字节 (65536 - 11059 = 54477 = 0xD4CD)
	TF0 = 0;				// 清除 TF0 溢出标志
	TR0 = 1;				// 启动定时器0
	ET0 = 1;				// 使能定时器0中断
	EA = 1;					// 使能总中断
	PT0 = 0;				// 默认中断优先级
}
