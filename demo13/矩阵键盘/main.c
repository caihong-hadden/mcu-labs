/**
 * @file main.c
 * @brief STC15F2K60S2 矩阵键盘读取并在 LCD1602 上显示键码
 */

#include <STC15F2K60S2.H>
#include "Delay.h"
#include "LCD1602.h"
#include "Mitrixkey.h"

unsigned char KeyNumber;

void main(void)
{
	LCD_Init();
	Delay(20);
	LCD_ShowString(1, 1, "MatrixKey Test:");

	while (1)
	{
		KeyNumber = MatrixKey(); // 轮询扫描矩阵键盘
		if (KeyNumber)
		{
			LCD_ShowNum(2, 1, KeyNumber, 2); // 显示当前按下的键码
		}
	}
}
