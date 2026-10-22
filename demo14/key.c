/**
 * @file key.c
 * @brief 独立按键驱动源文件
 * @details 硬件平台: 蓝桥杯 CT107D/CT107S (J5 跳线选择 BTN 独立按键模式)
 *          引脚连接:
 *          - S7: P3.0 (返回键码 1)
 *          - S6: P3.1 (返回键码 2)
 *          - S5: P3.2 (返回键码 3)
 *          - S4: P3.3 (返回键码 4)
 */

#include "key.h"
#include "Delay.h"
#include <STC15F2K60S2.H>

unsigned char Key(void)
{
	unsigned char KeyNumber = 0;

	if (P30 == 0) { Delay(20); while (P30 == 0); Delay(20); KeyNumber = 1; }
	if (P31 == 0) { Delay(20); while (P31 == 0); Delay(20); KeyNumber = 2; }
	if (P32 == 0) { Delay(20); while (P32 == 0); Delay(20); KeyNumber = 3; }
	if (P33 == 0) { Delay(20); while (P33 == 0); Delay(20); KeyNumber = 4; }

	return KeyNumber;
}
