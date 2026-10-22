/**
 * @file main.c
 * @brief STC15F2K60S2 电脑上位机串口控制板载 LED 灯
 * @details 硬件平台: STC15F2K60S2 (蓝桥杯 CT107D/CT107S)
 *          功能说明:
 *          1. 开启串口接收中断 (interrupt 4);
 *          2. 当上位机通过串口助手向单片机发送 1 字节指令时:
 *             - 单片机将该字节数据回传 (Echo) 给上位机;
 *             - 同时选通 Y4C LED 锁存器，将 ~SBUF 输出至 P0 端口，控制板载指示灯实时点亮对应位。
 */

#include <STC15F2K60S2.H>
#include "uart.h"
#include "timer0.h"

void main(void)
{
	/* 1. 选通 Y5C，初始化关闭蜂鸣器与继电器 */
	P27 = 1; P26 = 0; P25 = 1;
	P0 = 0x00;
	P27 = 0; P26 = 0; P25 = 0;

	/* 2. 选通 Y4C，初始关闭全部板载 LED (全高电平 0xFF) */
	P27 = 1; P26 = 0; P25 = 0;
	P0 = 0xFF;
	P27 = 0; P26 = 0; P25 = 0;

	/* 3. 初始化串口1与总中断 */
	UART_Init();

	while (1)
	{
		/* 主循环空闲等待串口中断接收指令 */
	}
}

/**
 * @brief 串口1中断服务函数
 */
void UART_Routine(void) interrupt 4
{
	if (RI == 1) // 接收中断标志置位 (接收到 1 字节)
	{
		UART_SendByte(SBUF); // 将接收到的数据回传给上位机

		/* 选通 Y4C (LED 锁存通道)，按位取反控制 L1~L8 指示灯点亮 */
		P27 = 1; P26 = 0; P25 = 0;
		P0 = ~SBUF; // 低电平点亮，例如发送 0x01 则点亮第 1 颗 LED
		P27 = 0; P26 = 0; P25 = 0;

		RI = 0; // 软件清零接收中断标志位
	}
}
