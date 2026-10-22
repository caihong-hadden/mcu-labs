/**
 * @file main.c
 * @brief 传统 8051 (STC89C5xRC) 平台 - 独立按键让 LED 显示二进制累加计数器
 * @details 硬件平台: STC89C52RC / 传统 8051 开发板 (12T 模式, 晶振 12.000MHz)
 *          引脚连接:
 *          - LED1 ~ LED8: P1.0 ~ P1.7 (低电平点亮，输出 0 亮，1 灭)
 *          - 按键: P2.1 (按下接地为 0)
 *          功能说明:
 *          利用 8 位无符号变量 LEDNum 记录当前计数值 (0 ~ 255)。
 *          每次按下按键时 LEDNum 自增 1，由于 LED 为低电平点亮特性，
 *          将计数值按位取反后写入端口 (P1 = ~LEDNum)，使 8 颗 LED 完美呈现标准二进制累加状态。
 */

#include <STC89C5xRC.H>

/**
 * @brief 毫秒阻塞延时函数 (按键消抖)
 * @param xms 延时时长 (ms)
 * @note  时钟频率: 12.000MHz (12T 模式)
 */
void Delay(unsigned int xms)	//@12.000MHz
{
	unsigned char data i, j;
	while (xms--)
	{
		i = 2;
		j = 239;
		do
		{
			while (--j);
		} while (--i);
	}
}

/**
 * @brief 主函数
 */
void main(void)
{
	unsigned char LEDNum = 0; // 二进制累加计数值 (0 ~ 255)

	while (1)
	{
		/* 检测按键按下 (P2.1) */
		if (P21 == 0)
		{
			Delay(20);          // 前沿消抖延时
			while (P21 == 0);   // 等待按键松手释放
			Delay(20);          // 后沿消抖延时

			LEDNum++;           // 计数值累加 1 (溢出 255 后自动回绕至 0)
			P1 = ~LEDNum;       // 低电平点亮，取反后映射显示在 8 颗 LED 上
		}
	}
}
