#include <STC15F2K60S2.H>

// 如果中断内要控制硬件引脚或操作全局变量，在外部声明或引入头文件
// extern unsigned char pwm_cnt;
// extern unsigned char duty;
// sbit MOTOR = P2^4;

/* ---------------- 计时与节拍全局变量 ---------------- */
//unsigned int ms_cnt = 0;       // 毫秒基准计数（用于主循环非阻塞任务）
//unsigned char key_timer = 0;   // 按键扫描节拍计数
//unsigned char nixie_timer = 0; // 数码管刷新节拍计数

/**
 * @brief  定时器0初始化函数
 * @note   定时周期: 1毫秒 @ 12.000MHz (1T 模式)
 *         主要用途: 提供系统的基准毫秒时钟、按键消抖、数码管刷新节拍
 */
void Timer0_Init(void)
{
    AUXR |= 0x80;       // 1000 0000: 定时器0时钟为 1T 模式（不分频）
    TMOD &= 0xF0;       // 清除定时器0模式控制位
    TMOD |= 0x00;       // 配置为 16位自动重装载 模式 (GATE=0, C/T=0, M1=0, M0=0)
    
    // 初值计算: 65536 - (12,000,000Hz * 0.001s) = 65536 - 12000 = 53536 (0xD120)
    TL0 = 0x50;         // 设置低8位初始值
    TH0 = 0xFB;         // 设置高8位初始值
    
    TF0 = 0;            // 清除溢出标志位
    TR0 = 1;            // 启动定时器0计数
    ET0 = 1;            // 开启定时器0中断允许
    EA  = 1;            // 开启CPU总中断
}

/**
 * @brief  定时器1初始化函数
 * @note   定时周期: 100微秒 @ 12.000MHz (1T 模式)
 *         主要用途: 产生高频平滑的 PWM 调速波形 (100us * 100 = 10ms周期, 100Hz)
 */
void Timer1_Init(void)
{
    AUXR |= 0x40;       // 0100 0000: 定时器1时钟为 1T 模式（不分频）
    TMOD &= 0x0F;       // 清除定时器1模式控制位
    TMOD |= 0x00;       // 配置为 16位自动重装载 模式
    
    // 初值计算: 65536 - (12,000,000Hz * 0.0001s) = 65536 - 1200 = 64336 (0xFB50)
    TL1 = 0x50;         // 设置低8位初始值
    TH1 = 0xFB;         // 设置高8位初始值
    
    TF1 = 0;            // 清除溢出标志位
    TR1 = 1;            // 启动定时器1计数
    ET1 = 1;            // 开启定时器1中断允许
    EA  = 1;            // 开启CPU总中断
}

/**
 * @brief  定时器0中断服务函数（1毫秒中断一次）
 * @note   中断号: 1 (自动由硬件清零 TF0，无需手动清除)
 */
//void Timer0_Isr(void) interrupt 1
//{
//    ms_cnt++;
//    key_timer++;
//    nixie_timer++;

//    // 示例：每 10ms 可以在主程序中通过 key_timer 判定去执行按键扫描
//    // 示例：每 2ms 可以在主程序中通过 nixie_timer 判定去刷新一位数码管
//}

/**
 * @brief  定时器1中断服务函数（100微秒中断一次）
 * @note   中断号: 3 (自动由硬件清零 TF1，无需手动清除)
 */
//void Timer1_Isr(void) interrupt 3
//{
//    /* -------- 示例：PWM 波形发生逻辑 -------- */
//    /*
//    static unsigned char pwm_cnt = 0;
//    
//    pwm_cnt++;
//    if(pwm_cnt >= 100)
//    {
//        pwm_cnt = 0;
//    }
//    
//    if(pwm_cnt < duty)
//    {
//        MOTOR = 0; // 导通运行
//    }
//    else
//    {
//        MOTOR = 1; // 关断
//    }
//    */
//}