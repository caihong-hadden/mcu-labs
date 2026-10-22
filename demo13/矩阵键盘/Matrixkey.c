/**
 * @file Matrixkey.c
 * @brief 蓝桥杯 4x4 矩阵键盘底层扫描驱动源文件
 * @details 硬件平台: STC15F2K60S2 (蓝桥杯 CT107D/CT107S, J5 跳线选择 KBD 模式)
 *          引脚连接:
 *          - 列扫描输出: P3.0, P3.1, P3.2, P3.3
 *          - 行输入检测: P4.4, P4.2, P3.5, P3.4
 */

#include <STC15F2K60S2.H>
#include "Delay.h"

unsigned char MatrixKey(void)
{
	unsigned char KeyNum = 0;

	/* 扫描第 1 列 (P3.1 拉低) */
	P3 = 0xFF;
	P31 = 0;
	if (P44 == 0) { Delay(20); while (P44 == 0); Delay(20); KeyNum = 6; }
	if (P42 == 0) { Delay(20); while (P42 == 0); Delay(20); KeyNum = 10; }
	if (P35 == 0) { Delay(20); while (P35 == 0); Delay(20); KeyNum = 14; }
	if (P34 == 0) { Delay(20); while (P34 == 0); Delay(20); KeyNum = 18; }
	
	/* 扫描第 2 列 (P3.0 拉低) */
	P3 = 0xFF;
	P30 = 0;
	if (P44 == 0) { Delay(20); while (P44 == 0); Delay(20); KeyNum = 7; }
	if (P42 == 0) { Delay(20); while (P42 == 0); Delay(20); KeyNum = 11; }
	if (P35 == 0) { Delay(20); while (P35 == 0); Delay(20); KeyNum = 15; }
	if (P34 == 0) { Delay(20); while (P34 == 0); Delay(20); KeyNum = 19; }

	/* 扫描第 3 列 (P3.2 拉低) */
	P3 = 0xFF;
	P32 = 0;
	if (P44 == 0) { Delay(20); while (P44 == 0); Delay(20); KeyNum = 5; }
	if (P42 == 0) { Delay(20); while (P42 == 0); Delay(20); KeyNum = 9; }
	if (P35 == 0) { Delay(20); while (P35 == 0); Delay(20); KeyNum = 13; }
	if (P34 == 0) { Delay(20); while (P34 == 0); Delay(20); KeyNum = 17; }

	/* 扫描第 4 列 (P3.3 拉低) */
	P3 = 0xFF;
	P33 = 0;
	if (P44 == 0) { Delay(20); while (P44 == 0); Delay(20); KeyNum = 4; }
	if (P42 == 0) { Delay(20); while (P42 == 0); Delay(20); KeyNum = 8; }
	if (P35 == 0) { Delay(20); while (P35 == 0); Delay(20); KeyNum = 12; }
	if (P34 == 0) { Delay(20); while (P34 == 0); Delay(20); KeyNum = 16; }

	return KeyNum;
}
