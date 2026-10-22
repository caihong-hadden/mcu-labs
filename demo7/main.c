/**
 * @file main.c
 * @brief 传统 8051 (STC89C5xRC) 平台 - 独立按键状态翻转控制 LED (按键去抖与边沿释放)
 * @details 硬件平台: STC89C52RC / 传统 8051 开发板 (12T 模式, 晶振 12.000MHz)
 *          引脚连接:
 *          - LED1: P1.1 (低电平点亮)
 *          - key1: P2.1 (按下接地为 0)
 *          功能说明:
 *          机械按键在闭合与断开瞬间会产生 5~10ms 的机械抖动。
 *          程序通过先延时 20ms 滤除按下抖动，随后等待按键松开 (while(key1==0))，
 *          再延时 20ms 滤除松开抖动，完成一次稳定的按键有效触发，将 LED1 的亮灭状态翻转。
 */

#include <STC89C5xRC.H>

/* 引脚定义 */
sbit LED1 = P1^1;  // 控制指示灯 (P1.1)
sbit key1 = P2^1;  // 控制按键 (P2.1)

/**
 * @brief 软件毫秒延时函数 (用于按键消抖)
 * @param xms 延时毫秒数
 * @note  时钟频率: 12.000MHz (12T 模式)
 */
void Delay(unsigned int xms)	//@12.000MHz
{
	unsigned char data i, j;
	while (xms)
	{
		i = 2;
		j = 239;
		do
		{
			while (--j);
		} while (--i);
		xms--;
	}
}

/**
 * @brief 主函数
 */
void main(void)
{
	while (1)
	{
		/* 检查按键是否被按下 (初次检测到低电平) */
		if (key1 == 0)
		{
			Delay(20);          // 前沿消抖: 延时 20ms 避开闭合抖动期
			while (key1 == 0);  // 等待按键释放 (松手检测)
			Delay(20);          // 后沿消抖: 延时 20ms 避开断开抖动期
			LED1 = ~LED1;       // 翻转 LED1 电平状态 (亮->灭 或 灭->亮)
		}
	}
}
