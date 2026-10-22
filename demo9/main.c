/**
 * @file main.c
 * @brief 传统 8051 (STC89C5xRC) 平台 - 独立按键控制 LED 移位 (双向循环左移与右移)
 * @details 硬件平台: STC89C52RC / 传统 8051 开发板 (12T 模式, 晶振 12.000MHz)
 *          引脚连接:
 *          - LED1 ~ LED8: P1.0 ~ P1.7 (低电平点亮)
 *          - key1 (左移/加键): P2.0 (按下接地为 0)
 *          - key2 (右移/减键): P2.1 (按下接地为 0)
 *          功能说明:
 *          通过变量 LEDNum (0 ~ 7) 记录当前点亮的 LED 索引位置。
 *          按键 P2.0 触发索引递增 (0->1->...->7->0)，实现 LED 循环向左移动；
 *          按键 P2.1 触发索引递减 (7->6->...->0->7)，实现 LED 循环向右移动。
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

unsigned char LEDNum = 0; // 当前亮灯索引 (0 ~ 7)

/**
 * @brief 主函数
 */
void main(void)
{
	/* 初始状态: 点亮第 1 颗 LED (P1.0) */
	P1 = ~(0x01 << LEDNum);

	while (1)
	{
		/* 按键 1 (P2.0): 循环向后移位 (LEDNum 自增) */
		if (P20 == 0)
		{
			Delay(20);          // 前沿消抖
			while (P20 == 0);   // 松手检测
			Delay(20);          // 后沿消抖

			LEDNum++;
			if (LEDNum >= 8)
			{
				LEDNum = 0;     // 达到最左端后回绕至最右端第 1 颗
			}
			P1 = ~(0x01 << LEDNum); // 更新 P1 端口
		}

		/* 按键 2 (P2.1): 循环向前移位 (LEDNum 自减) */
		if (P21 == 0)
		{
			Delay(20);          // 前沿消抖
			while (P21 == 0);   // 松手检测
			Delay(20);          // 后沿消抖

			if (LEDNum == 0)
			{
				LEDNum = 7;     // 达到最右端后回绕至最左端第 8 颗
			}
			else
			{
				LEDNum--;
			}
			P1 = ~(0x01 << LEDNum); // 更新 P1 端口
		}
	}
}
