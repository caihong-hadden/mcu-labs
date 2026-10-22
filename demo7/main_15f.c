/**
 * @file main_15f.c
 * @brief STC15F 系列平台 (蓝桥杯 CT107D 架构) - 独立按键状态翻转控制板载 LED
 * @details 硬件平台: STC15F2K60S2 / IAP15F2K61S2 开发板 (蓝桥杯 CT107D/CT107S)
 *          时钟频率: 11.0592MHz (STC 1T 模式)
 *          硬件控制:
 *          1. 蓝桥杯 J5 跳线帽接在 BTN 端 (独立按键模式):
 *             - 按键 S7 连接 P3.0 引脚 (按下为低电平 0)
 *          2. 锁存器配置: J13 接 IO 模式，P4.2 输出低电平使能 WR 门禁。
 *          3. 选通 74HC138 Y4C 锁存器: P27=1, P26=0, P25=0 (LED 锁存使能)。
 *          4. 实现 1T 模式专用 Delay 延时函数进行前后沿消抖，
 *             每次按下并松开 S7，板载指示灯 L1 (P0.0) 的亮灭状态翻转一次。
 */

#include <STC15F2K60S2.H>
#include <INTRINS.H>

/* 按键引脚定义 (CT107D S7 按键) */
sbit Key_S7 = P3^0;

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

/**
 * @brief 主函数
 */
void main(void)
{
    unsigned char led_state = 0; // L1 状态标志: 0-熄灭, 1-点亮

    /* 1. 开启 74HC138 译码与 LED (Y4C) 锁存使能 */
    P42 = 0;    // WR 门禁使能 (J13 IO 模式)
    P27 = 1;
    P26 = 0;
    P25 = 0;    // Y4C 选通使能 (74HC573 LE 保持高电平透明传输)

    /* 2. 初始关闭所有 LED */
    P0 = 0xFF;

    while (1)
    {
        /* 检测 S7 按键按下 (P3.0 为低电平) */
        if (Key_S7 == 0)
        {
            Delay(20);              // 前沿消抖
            while (Key_S7 == 0);    // 等待按键释放
            Delay(20);              // 后沿消抖

            led_state = !led_state; // 状态取反

            if (led_state)
            {
                P0 = 0xFE;          // 点亮 L1 (P0.0=0)
            }
            else
            {
                P0 = 0xFF;          // 熄灭 L1 (P0.0=1)
            }
        }
    }
}
