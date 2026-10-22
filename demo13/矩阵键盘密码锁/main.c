/**
 * @file main.c
 * @brief STC15F2K60S2 蓝桥杯 4x4 矩阵键盘电子密码锁
 * @details 硬件平台: STC15F2K60S2 (蓝桥杯 CT107D/CT107S)
 *          功能特性:
 *          1. 密码输入: 支持 4 位数字密码键入，LCD1602 实时显示已输入密码;
 *          2. 密码确认: 按下确定键 (键码 14) 验证密码，预设密码为 2345:
 *             - 验证通过: 提示 "ok!"
 *             - 验证失败: 提示 "error!" 并记录错误次数
 *          3. 输错锁定: 连续输错 3 次锁定系统并倒计时 6 秒;
 *          4. 重置清空: 按下清空键 (键码 15) 清除已键入密码。
 */

#include <STC15F2K60S2.H>
#include "Delay.h"
#include "LCD1602.h"
#include "Mitrixkey.h"

unsigned char KeyNumber;
unsigned int PassWord, Count, Errorc, Time;

/**
 * @brief 清空当前密码输入界面与缓存
 */
void Clean(void)
{
	PassWord = 0;
	Count = 0;
	LCD_ShowString(1, 1, "Password:       ");
	LCD_ShowString(2, 5, "            ");
	LCD_ShowNum(2, 1, 0, 4);
}

void main(void)
{
	LCD_Init();
	Delay(20);
	Clean();

	while (1)
	{
		KeyNumber = MatrixKey();
		if (KeyNumber)
		{
			/* 0~9 数字按键输入处理 (限定 4 位数字) */
			if (KeyNumber < 14)
			{
				if (Count < 4)
				{
					PassWord *= 10;
					PassWord += (KeyNumber - 3) % 10;
					Count++;
				}
				LCD_ShowNum(2, 1, PassWord, 4);
			}

			/* 键码 15: 清除键 */
			if (KeyNumber == 15)
			{
				Clean();
			}

			/* 键码 14: 确认键 */
			if (KeyNumber == 14)
			{
				if (PassWord == 2345)
				{
					LCD_ShowString(1, 14, "ok!"); // 密码正确
				}
				else
				{
					LCD_ShowString(1, 11, "error!"); // 密码错误
					Delay(1000);
					Clean();
					Errorc++;

					/* 连续输错 3 次，锁定系统倒计时 6 秒 */
					if (Errorc == 3)
					{
						Time = 6;
						LCD_ShowString(1, 1, "Too many errors!");
						LCD_ShowString(2, 1, "Please Wait:   s");
						while (Time--)
						{
							Delay(1000);
							LCD_ShowNum(2, 13, Time, 2);
						}
						Errorc = 0;
						Clean();
					}
				}
			}
		}
	}
}
