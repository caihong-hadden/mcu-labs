/**
 * @file main_15f.c
 * @brief STC15F 系列平台 (蓝桥杯 CT107D 架构) - 独立按键控制板载 LED 左右移位流动
 * @details 硬件平台: STC15F2K60S2 / IAP15F2K61S2 开发板 (蓝桥杯 CT107D/CT107S)
 *          时钟频率: 11.0592MHz (STC 1T 模式)
 *          硬件控制:
 *          1. 蓝桥杯 J5 跳线帽接在 BTN 端 (独立按键模式):
 *             - 按键 S7 连接 P3.0 引脚: 触发 LED 顺次左移递增 (L1->L2->...->L8->L1)
 *             - 按键 S6 连接 P3.1 引脚: 触发 LED 顺次右移递减 (L8->L7->...->L1->L8)
 *          2. 锁存器配置: J13 接 IO 模式，P4.2 输出低电平使能 WR 门禁。
 *          3. 选通 74HC138 Y4C 锁存器: P27=1, P26=0, P25=0 (LED 锁存使能)。
 *          4. P0 端口连接 L1~L8 指示灯 (低电平点亮)。
 *          5. 编写 1T 模式专用消抖延时，实现稳定的双向按键移位控制。
 */

#include <STC15F2K60S2.H>
#include <INTRINS.H>

/* 按键引脚定义 (CT107D S7 与 S6 按键) */
sbit Key_S7 = P3^0;
sbit Key_S6 = P3^1;

/**
 * @brief 专为 STC15 系列 (1T 模式) @11.0592MHz 适配的毫秒延时消抖函数
 * @param xms 延时时长 (ms)
 */
void Delay(unsigned int xms)	//@11.0592MHz
{
	unsigned char data i, j;
	while (xms--)
	{
		_nop_();
		i = 11;
		j = 190;
		do
		{
			while (--j);
		} while (--i);
	}
}

unsigned char LEDNum = 0; // 当前点亮的指示灯索引 (0 ~ 7)

/**
 * @brief 主函数
 */
void main(void)
{
    /* 1. 开启 74HC138 译码与 LED (Y4C) 锁存使能 */
    P42 = 0;    // WR 门禁使能 (J13 IO 模式)
    P27 = 1;
    P26 = 0;
    P25 = 0;    // Y4C 选通使能 (74HC573 LE 保持高电平透明传输)

    /* 2. 初始状态点亮第 1 颗 LED (L1) */
    P0 = ~(0x01 << LEDNum);

    while (1)
    {
        /* S7 按键 (P3.0): 循环向左移位 (LEDNum 自增) */
        if (Key_S7 == 0)
        {
            Delay(20);              // 前沿消抖
            while (Key_S7 == 0);    // 等待松开
            Delay(20);              // 后沿消抖

            LEDNum++;
            if (LEDNum >= 8)
            {
                LEDNum = 0;         // 循环回绕至 L1
            }
            P0 = ~(0x01 << LEDNum); // 更新板载指示灯
        }

        /* S6 按键 (P3.1): 循环向右移位 (LEDNum 自减) */
        if (Key_S6 == 0)
        {
            Delay(20);              // 前沿消抖
            while (Key_S6 == 0);    // 等待松开
            Delay(20);              // 后沿消抖

            if (LEDNum == 0)
            {
                LEDNum = 7;         // 循环回绕至 L8
            }
            else
            {
                LEDNum--;
            }
            P0 = ~(0x01 << LEDNum); // 更新板载指示灯
        }
    }
}
