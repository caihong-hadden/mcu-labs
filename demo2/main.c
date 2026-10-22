/**
 * @file main.c
 * @brief 传统 8051 (STC89C5xRC) 平台 - 点亮单颗 LED (第二颗 LED)
 * @details 硬件平台: STC89C52RC / 传统 8051 开发板 (12T 模式)
 *          引脚连接: P1 端口低 8 位直接驱动板载 8 颗 LED 的阴极 (低电平导通点亮)
 *          功能说明: 将 P1 端口输出相应低电平模式，使对应位置的 LED 稳定点亮。
 */

#include <STC89C5xRC.H>

/**
 * @brief 主函数
 */
void main(void)
{
    /* 
     * 0xFE = 1111 1110B: P1.0 输出低电平，点亮第 1 颗 LED
     * 若为点亮第 2 颗 LED 则为 0xFD (1111 1101B)
     */
    P1 = 0xFD; // 输出 0xFD，点亮第 2 颗 LED (D2/LED2)

    while (1)
    {
        /* 主循环死循环保持引脚电平稳定 */
    }
}
