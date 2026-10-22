#include <STC15F2K60S2.H>
#include "timer0.h"
#include "Delay.h"
#include "dianzhen.h"

/**
 * @file main.c
 * @brief STC15F2K60S2 点阵屏多图案动态切换扫描程序
 * @details 采用定时器0计时，每隔 1500ms (1.5秒) 切换显示图案；
 *          高频扫描每个点的坐标 (X, Y)，利用人眼视觉暂留实现图案显示。
 */

/* 计时与状态控制变量 */
unsigned int time = 1500;       // 图案切换计时器 (单位: ms)
unsigned char time1 = 1;        // 单点动态刷新微计时器 (单位: ms)
unsigned char state = 0;        // 图案状态索引: 0 - 图案一, 1 - 图案二
unsigned char num = 0;          // 当前图案内的点索引

/* 图案 0 坐标点表 (共 9 个点，保存在 Code ROM 中) */
unsigned char code X_Tab0[9] = {4, 3, 2, 1, 1, 1, 2, 3, 4};
unsigned char code Y_Tab0[9] = {1, 1, 1, 2, 3, 4, 5, 5, 5};

/* 图案 1 坐标点表 (共 10 个点，保存在 Code ROM 中) */
unsigned char code X_Tab1[10] = {2, 3, 4, 5, 4, 4, 4, 4, 3, 2};
unsigned char code Y_Tab1[10] = {1, 1, 1, 1, 2, 3, 4, 5, 5, 4};

void main(void)
{
    Timer0_Init(); // 初始化定时器0 (1ms中断一次)

    while (1)
    {
        /* 状态机图案切换与动态扫描 */
        switch (state)
        {
            case 0:
                if (time1 == 0)
                {
                    dianzhen(X_Tab0[num], Y_Tab0[num]); // 扫描点阵当前点
                    num++;
                    if (num >= 9)
                    {
                        num = 0;
                    }
                    time1 = 1; // 复位刷新周期
                }
                if (time == 0)
                {
                    state = 1;      // 切换至图案 1
                    num = 0;        // 点索引归零
                    time = 1500;    // 维持 1500ms
                }
                break;

            case 1:
                if (time1 == 0)
                {
                    dianzhen(X_Tab1[num], Y_Tab1[num]); // 扫描点阵当前点
                    num++;
                    if (num >= 10)
                    {
                        num = 0;
                    }
                    time1 = 1; // 复位刷新周期
                }
                if (time == 0)
                {
                    state = 0;      // 切换回图案 0
                    num = 0;        // 点索引归零
                    time = 1500;    // 维持 1500ms
                }
                break;

            default:
                state = 0;
                time = 1500;
                num = 0;
                break;
        }
    }
}

/**
 * @brief 定时器0中断服务函数 (每 1ms 触发一次)
 */
void Timer0_Routine(void) interrupt 1
{
    if (time > 0)
    {
        time--;
    }
    if (time1 > 0)
    {
        time1--;
    }
}
