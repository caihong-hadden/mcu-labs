/**
 * @file main.c
 * @brief STC15F2K60S2 LCD1602 字符液晶显示测试例程
 */

#include <STC15F2K60S2.H>
#include "LCD1602.h"
#include "Delay.h"

void main(void)
{
	LCD_Init();   // 初始化 LCD1602
	Delay(20);

	/* 在第 1 行显示字符串 */
	LCD_ShowString(1, 1, "LCD1602 TEST OK!");
	/* 在第 2 行显示格式化数值 */
	LCD_ShowNum(2, 1, 12345, 5);

	while (1)
	{
		/* 保持常显 */
	}
}
