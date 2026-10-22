#include <STC15F2K60S2.H>
#include "buzzer.h"
// 传递给主循环的键值（1次有效按键产生1次非零值）

// 1. 无阻塞底层读取：快速扫描矩阵，立刻返回瞬时状态（无任何Delay和while）
unsigned char MatrixKey_ReadRaw(void)
{
    unsigned char key = 0;

    // 第一列扫描 (P3.1拉低)
    P3 = 0xFF;
    P31 = 0;
    if(P44 == 0) key = 6;
    if(P42 == 0) key = 10;
    if(P35 == 0) key = 14;
    if(P34 == 0) key = 18;

    // 第二列扫描 (P3.0拉低)
    P3 = 0xFF;
    P30 = 0;
    if(P44 == 0) key = 7;
    if(P42 == 0) key = 11;
    if(P35 == 0) key = 15;
    if(P34 == 0) key = 19;

    // 第三列扫描 (P3.2拉低)
    P3 = 0xFF;
    P32 = 0;
    if(P44 == 0) key = 5;
    if(P42 == 0) key = 9;
    if(P35 == 0) key = 13;
    if(P34 == 0) key = 17;

    // 第四列扫描 (P3.3拉低)
    P3 = 0xFF;
    P33 = 0;
    if(P44 == 0) key = 4;
    if(P42 == 0) key = 8;
    if(P35 == 0) key = 12;
    if(P34 == 0) key = 16;

    P3 = 0xFF; // 扫描完成后释放引脚
    return key;
}

// 2. 定时器0初始化：产生 1ms 中断用于时间基准（12MHz 1T模式）
//void Timer0_Init(void)
//{
//    AUXR |= 0x80;       // 定时器0时钟1T模式
//    TMOD &= 0xF0;       // 16位自动重装
//    TL0 = 0x20;         // 65536 - 12000 = 53536 (0xD120)
//    TH0 = 0xD1;
//    TF0 = 0;
//    TR0 = 1;
//    ET0 = 1;
//    EA = 1;
//}

// 3. 定时器0中断服务函数：每 10ms 扫描一次并完成消抖与松手检测
//void Timer0_Routine() interrupt 1
//{
//    static unsigned char key_timer = 0;
//    static unsigned char Key_Last = 0;
//    unsigned char Key_Now = 0;

//    key_timer++;
//    if(key_timer >= 10) // 每 10ms 执行一次按键逻辑（天然硬件级消抖）
//    {
//        key_timer = 0;
//        Key_Now = MatrixKey_ReadRaw();

//        // 判定条件：上一状态有按键，当前状态变为0 -> 说明产生了松手动作
//        if(Key_Last == 0 && Key_Now != 0) { 
//				 
//				 BUZZER(1);
//				 
//				 }
//				if(Key_Last != 0 && Key_Now == 0)
//        {		
//						
//            Key_Val = Key_Last; 
//						BUZZER(0);// 上报松手前的有效键值
//        }
//        //如果想改成“按下瞬间响应”（不必等松手），可以改成：
//         

//        Key_Last = Key_Now; // 记录当前状态供下一轮对比
//    }
//}

// 4. 主函数：处理业务逻辑
//void main()
//{
//    Timer0_Init();

//    while(1)
//    {
//        if(Key_Val != 0) // 有按键按下并释放
//        {
//            unsigned char num = Key_Val;
//            Key_Val = 0; // 手动清除标记

//            // 根据 num 进行具体动作处理（比如调整前面提到的 PWM 占空比）
//            if(num == 4) 
//            {
//                // TODO: 相应按键处理
//            }
//        }
//    }
//}