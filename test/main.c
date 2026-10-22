/**
 * @file main.c
 * @brief STC15F2K60S2 系统基础运行与功能验证测试模板
 * @details 硬件平台: STC15F2K60S2 / IAP15F2K61S2
 *          功能说明: 提供单片机最简系统运行自检、心跳指示与空闲主循环骨架，
 *                   可直接作为新建 15F 工程时的模板代码。
 */

#include <STC15F2K60S2.H>
#include <INTRINS.H>

/**
 * @brief 毫秒级软件阻塞延时
 * @param xms 延时时长 (ms) @11.0592MHz (1T 模式)
 */
void Delay(unsigned int xms)
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
    /* 1. 锁存器初始化 (针对蓝桥杯 CT107D 平台，若使用核心板则直接操作 IO) */
    P42 = 0;    // WR 门禁使能
    P27 = 1;
    P26 = 0;
    P25 = 0;    // 选通 Y4C (LED 锁存器)

    /* 2. 初始关闭所有指示灯 */
    P0 = 0xFF;

    while (1)
    {
        /* 心跳指示: L1 指示灯以 500ms 周期交替闪烁 */
        P0 = 0xFE;  // 点亮 L1 (P0.0)
        Delay(500);
        P0 = 0xFF;  // 熄灭 L1
        Delay(500);
    }
}
