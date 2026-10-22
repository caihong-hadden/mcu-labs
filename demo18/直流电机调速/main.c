/**
 * @file main.c
 * @brief STC15F2K60S2 直流电机 PWM 调速、按键换挡、数码管显示与串口上报综合系统
 * @details 硬件平台: STC15F2K60S2 (蓝桥杯 CT107D/CT107S)
 *          功能集成:
 *          1. 电机驱动: 定时器0产生 100Hz PWM 波形，驱动板载直流电机引脚 (P2.4 / P3.4);
 *          2. 矩阵按键调速: 按键4加速 (+10% 占空比)，按键5减速 (-10% 占空比);
 *          3. 蜂鸣器反馈: 按键按下伴随蜂鸣器按键音提示;
 *          4. 数码管显示: 2位数码管动态扫描显示当前电机占空比百分比数值;
 *          5. 串口通信: 实时上报按键触发事件与当前调速百分比信息至上位机串口助手。
 */

#include <STC15F2K60S2.H>
#include "keypro.h"
#include "nixie.h"
#include "buzzer.h"
#include "uart.h"
#include "timer.h"

#define PWM_PERIOD 100

sbit MOTOR = P2^4;  // 直流电机驱动引脚 1
sbit MOTOR2 = P3^4; // 直流电机驱动引脚 2

unsigned char pwm_cnt = 0;   // PWM 计数器 (0~99)
unsigned char duty = 0;      // 电机当前占空比 (0 ~ 100)
unsigned char Time = 0;      // 数码管扫描分频计时器
unsigned int  Time2 = 5000;  // 串口定时上报计数器
volatile bit  Time_flag = 0; // 串口上报就绪标志
unsigned char Key_Val = 0;   // 消费的按键键码
unsigned char duty_ten;      // 占空比十位数
unsigned char duty_unit;     // 占空比个位数

void main(void)
{		
    Timer0_Init(); // 启动定时器0 (100us 中断)
    UART_Init();   // 启动串口1

    while (1)
    {
        /* 1. 串口定时上报当前电机占空比信息 */
        if (Time_flag == 1)
        {
            Time_flag = 0;
            UART_SendString("当前占空比为：");
            if (duty == 100)
            {
                UART_SendByte('1');
                UART_SendByte('0');
                UART_SendByte('0');
            }
            else
            {
                UART_SendByte(duty / 10 + '0');
                UART_SendByte(duty % 10 + '0');
            }
            UART_SendString("%\r\n");
        }

        /* 2. 处理按键换挡调速逻辑 */
        if (Key_Val != 0)
        {
            unsigned char num = Key_Val;
            Key_Val = 0;

            UART_SendString("检测到按键：");
            UART_SendByte(num + '0');
            UART_SendString("\r\n");

            if (num == 4) // 按键 4: 加速
            {
                if (duty <= 90) duty += 10;
                else duty = 100;
            }
            if (num == 5) // 按键 5: 减速
            {
                if (duty >= 10) duty -= 10;
                else duty = 0;
            }

            duty_ten = duty / 10;
            duty_unit = duty % 10;
        }
    }
}

/**
 * @brief 定时器0中断服务函数 (100us 周期)
 * @details 负责电机 PWM 生成、矩阵按键底层扫描、数码管分时刷新以及串口上报定时
 */
void Timer0_Routine(void) interrupt 1
{		
    static unsigned char key_timer = 0;
    static unsigned char Key_Last = 0;
    unsigned char Key_Now = 0;

    Time++;
    pwm_cnt++;
    key_timer++;

    /* 1. 电机 PWM 输出生成 (100Hz 周期) */
    if (pwm_cnt >= PWM_PERIOD)
    {
        pwm_cnt = 0;
    }

    if (pwm_cnt < duty)
    {
        MOTOR = 0;   // 导通电机
        MOTOR2 = 0;
    }
    else
    {
        MOTOR = 1;   // 截止电机
        MOTOR2 = 1;
    }

    /* 2. 矩阵按键高频消抖与边沿检测 (每 10ms 扫描一次) */
    if (key_timer >= 100)
    {
        key_timer = 0;
        Key_Now = MatrixKey_ReadRaw();

        /* 按下瞬间触发蜂鸣器按键音 */
        if (Key_Last == 0 && Key_Now != 0)
        { 
            BUZZER(1);
        }
        /* 释放瞬间上报有效键码并关闭蜂鸣器 */
        if (Key_Last != 0 && Key_Now == 0)
        {		
            Key_Val = Key_Last; 
            BUZZER(0);
        }
        Key_Last = Key_Now;
    }

    /* 3. 串口上报计时器 (每 5 秒上报一次) */
    if (Time2 != 0)
    {
        Time2--; 
    }
    else
    {
        Time_flag = 1;
        Time2 = 5000;
    }

    /* 4. 数码管分时动态扫描 (每 2ms 切换一位) */
    if (Time >= 20)
    {
        static unsigned char digit_sel = 0;
        Time = 0;
        if (duty == 100)
        {
            if (digit_sel == 0)
            {
                Nixie(1, 9); // 第 1 位显示 9
                digit_sel = 1;
            }
            else
            {
                Nixie(2, 9); // 第 2 位显示 9
                digit_sel = 0;
            }
        }
        else
        {
            if (digit_sel == 0)
            {
                Nixie(1, duty_ten);  // 第 1 位显示十位
                digit_sel = 1;
            }
            else
            {
                Nixie(2, duty_unit); // 第 2 位显示个位
                digit_sel = 0;
            }
        }
    }
}
