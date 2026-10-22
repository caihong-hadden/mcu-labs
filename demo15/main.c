/**
 * @file main.c
 * @brief STC15F2K60S2 串口数据定时发送例程
 * @details 硬件平台: STC15F2K60S2 (蓝桥杯 CT107D/CT107S)
 *          功能说明:
 *          1. 定时器0产生 1ms 中断基准，累计 1000ms (1秒);
 *          2. 每隔 1 秒通过 UART 自动发送递增计数值 num (0~255)，供上位机串口助手接收。
 */

#include <STC15F2K60S2.H>
#include "Delay.h"
#include "uart.h"
#include "timer0.h"

unsigned char num = 0;             // 待发送的数据变量
volatile unsigned int time = 1000; // 1秒计时器

void main(void)
{
	Timer0_Init(); // 初始化定时器0 (1ms中断)
	UART_Init();   // 初始化串口1

	while (1)
	{
		/* 每隔 1000ms 发送一次数据 */
		if (time == 0)
		{
			UART_SendByte(num); // 串口向上位机发送 1 字节
			num++;              // 数据递增 (0~255 循环)
			time = 1000;        // 重置 1 秒定时
		}
	}
}

/**
 * @brief 定时器0中断服务函数 (1ms 周期)
 */
void Timer0_Routine(void) interrupt 1
{
	if (time > 0)
	{
		time--;
	}
}
