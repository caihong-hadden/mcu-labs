/**
 * @file uart.c
 * @brief 串口1 (UART1) 驱动模块源文件
 * @details 硬件平台: STC15F2K60S2 (蓝桥杯 CT107D/CT107S)
 *          时钟频率: 11.0592MHz (STC 1T 模式)
 *          波特率生成器: 定时器1 1T 模式产生标准波特率 (4800 / 9600 bps)
 *          通信协议: 8 位数据位, 无校验位, 1 位停止位 (8-N-1)
 */

#include "uart.h"
#include <STC15F2K60S2.H>

void UART_Init(void)
{
	SCON = 0x50;		// 8位数据位, 可变波特率, 允许接收 (REN=1)
	AUXR |= 0x40;		// 定时器1时钟设为 1T 模式 (速度提升 12 倍)
	AUXR &= 0xFE;		// 串口1选择定时器1作为波特率发生器
	TMOD &= 0x0F;		// 定时器1设为模式0 (16位自动重装载)
	TL1 = 0xE8;			// 定时器1初值低字节
	TH1 = 0xFF;			// 定时器1初值高字节 (波特率校准)
	ET1 = 0;			// 禁止定时器1中断
	TR1 = 1;			// 启动定时器1开始计数
	EA  = 1;			// 开启全局中断
	ES  = 1;			// 开启串口1中断
}

void UART_SendByte(unsigned char Byte)
{
	SBUF = Byte;		// 将数据写入发送缓冲寄存器
	while (TI == 0);	// 等待硬件发送完毕置位 TI
	TI = 0;				// 软件清零发送中断标志位
}
