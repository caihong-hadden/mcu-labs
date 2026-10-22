/**
 * @file main.c
 * @brief 传统 8051 (STC89C5xRC) 平台 - 独立按键实时控制 LED 状态 (按压点亮/松开熄灭)
 * @details 硬件平台: STC89C52RC / 传统 8051 开发板
 *          引脚连接:
 *          - LED1: P1.0 (低电平点亮)
 *          - LED2: P1.1 (低电平点亮)
 *          - key1: P2.0 (按键按下接地，输入低电平 0)
 *          - key2: P2.1 (按键按下接地，输入低电平 0)
 *          功能说明: 轮询读取独立按键输入电平，按下键时对应 LED 实时导通点亮，松开后熄灭。
 */

#include <STC89C5xRC.H>

/* 引脚位定义 */
sbit LED1 = P1^0;  // 第 1 颗 LED 阴极
sbit LED2 = P1^1;  // 第 2 颗 LED 阴极
sbit key1 = P2^0;  // 独立按键 1
sbit key2 = P2^1;  // 独立按键 2

/**
 * @brief 主函数
 */
void main(void)
{
	while (1)
	{
		/* 检测按键 1 状态 */
		if (key1 == 0)
		{
			LED1 = 0;  // 按下按键 1，点亮 LED1
		}
		else
		{
			LED1 = 1;  // 松开按键 1，熄灭 LED1
		}

		/* 检测按键 2 状态 */
		if (key2 == 0)
		{
			LED2 = 0;  // 按下按键 2，点亮 LED2
		}
		else
		{
			LED2 = 1;  // 松开按键 2，熄灭 LED2
		}
	}
}
