/**
 * @file main.c
 * @brief STC15F2K60S2 蓝桥杯模块化编程综合演示主程序
 * @details 硬件平台: STC15F2K60S2 / IAP15F2K61S2 (蓝桥杯 CT107D/CT107S)
 *          功能说明:
 *          演示模块化工程架构 (Delay + Nixie 驱动模块分离)。
 *          循环执行两阶段动作:
 *          1. 选通 Y4C LED 锁存器，执行 8 路流水灯扫描 (L1~L8 轮流点亮 500ms);
 *          2. 选通数码管，在第 1~8 位数码管上分别动态扫描显示数字 1~8，持续约 2 秒。
 */

#include <STC15F2K60S2.H>
#include "Delay.h"
#include "Nixie.h"

void main(void)
{
    unsigned char k;
    unsigned char f;
    unsigned int Time;

    while (1)
    {
        /* 1. 选通 Y4C 通道 (LED 锁存使能)，打开 8 路 LED 流水灯 */
        P42 = 0;
        P27 = 1; P26 = 0; P25 = 0;
        P0 = 0x01;

        for (k = 0; k < 8; k++)
        {
            P0 = ~(1 << k); // 逐位输出低电平，点亮 L1~L8
            Delay(500);     // 延时 500ms
        }
        P0 = 0xFF;          // 熄灭所有 LED

        /* 2. 数码管 8 位高频动态扫描 (维持约 2 秒) */
        for (Time = 0; Time <= 250; Time++)
        {
            for (f = 1; f <= 8; f++)
            {
                Nixie(f, f); // 第 f 位显示数字 f
                Delay(1);    // 维持 1ms 刷新节拍
            }
        }

        /* 3. 消隐第 1 位数码管 */
        Nixie(1, 10);
    }
}
